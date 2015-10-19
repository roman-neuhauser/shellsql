/*
    SHSQL suite - SQL utility for LINUX/UNIX shell scriptiing
    Copyright (C) 2004  Edward Macnaghten

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Edward Macnaghten
    EDL Systems
    16 Brierley Walk
    Cambridge
    CB4 3NH
    UK

    eddy@edlsystems.com
 */

/*
 * POSTGRRES - becoming the ipso factor standard enterprise db for linux
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <strings.h>

#include "string.h"
#include "message.h"
#include "shellsql.h"

typedef void (*sighandler_t)(int);
void mainloop();
void sighand();
void dolines(PGresult *pgr, char format);
void wipe(char *s);

/*
 * Some global items.....
 */

PGconn *sqldb;
message *mes;
int isloop = -1;

int main(int argc, char *argv[])
{
	int errn;
	string *str;
	int complete = -1;
	int i;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s connecstring\n", argv[0]);
		exit(1);
	}

	if((mes = new_message(0)) == NULL)
	{
		fprintf(stderr, "%s: Cannot open message queue\n", argv[0]);
		exit(1);
	}


	/*
	 * I believe a fork duplicates malloced stuff, I am in the poo if it does not :-)
	 */
	errn = fork();

	if(errn == -1)
	{
		fprintf(stderr, "%s: Cannot fork child process\n", argv[0]);
		exit(1);
	}
	else if(errn)
	{
/*
 * This is the parent - which behaves like clien
 */

		/*
		 * Initially just a status message is snt back, zero string length 
		 *
		 * Still needs a string to put it into
		 */


		str = new_string();
		
		if(message_receive(mes, str, &complete, MES_SERVER_TO_CLIENT) < 0)
		{
			fprintf(stderr, "%s: %s\n", argv[0], string_s(str));
			message_destroy(mes);
			message_delete(mes);
			string_delete(str);
			exit(1);
		}
		else
		{
			printf("%d\n",message_id(mes)) ;
			message_delete(mes);
			string_delete(str);
			exit(0);
		}
	}
/*
 * All parents have exited now
 */

/*
 * This is all a child
 */

	/*
	 * I think I need to do this here...
	 * I do not fully understand why, it just works!
	 * otherwise shell parent hangs!
	 */

	setsid();
	if(fork()) exit(0);
	close(0);
	close(1);
	close(2);

	/*
	 * We are in daemon mode now
	 */

	/*
	 * open the database
	 */

	str = new_string();

	for(i=1;i<argc;i++)
	{
		string_cat(str, argv[i]);
		if((!strncasecmp(argv[i], "password", 8))
		 || (!strncasecmp(argv[i], "user", 4))
		 || (!strncasecmp(argv[i], "useriname", 8)))
			wipe(argv[i]);
		if(i < argc - 1) string_cat_c(str, ' ');
	}

	sqldb = PQconnectdb(string_s(str));
	string_delete(str);

	if(sqldb == NULL)
	{
		message_status(mes, 127, "Error allocating connection\n", MES_SERVER_TO_CLIENT);
		message_delete(mes);
		exit(1);
	}
	if (PQstatus(sqldb) != CONNECTION_OK)
	{
		char ws[64];
		snprintf(ws, 63, "Error opening postgres %s/%s\n", PQhost(sqldb), PQdb(sqldb));
		ws[63] = 0;
		message_status(mes, 127, ws, MES_SERVER_TO_CLIENT);
		PQfinish(sqldb);
		message_delete(mes);
		exit(1);
	}

	/*
	 * Transmit to parent that we are hunky dory
	 */

	message_status(mes, 0, "", MES_SERVER_TO_CLIENT);

	/*
	 * We are open for business - Lets go
 	 */

	mainloop();

	/*
	 * At the end, tidy up
	 */

	PQfinish(sqldb);
	message_destroy(mes);
	message_delete(mes);
	exit(0);
}


/*
 * This is the main server bit
 *
 * It receives SQL statements and executes them, breaking at EOF
 */
void mainloop()
{

	int sz;
	char *errmes;

	string *str = NULL;

	int complete = -1;

	PGresult *pgr;

	char format;

	str = new_string();

	while(isloop)
	{
		
		signal(SIGINT, sighand);
		signal(SIGTERM, sighand);

		complete = -1;
		sz = message_receive(mes, str, &complete, MES_CLIENT_TO_SERVER);
		
		if(sz < 0)
		{
			/*
			 * End of file 
			 */
			isloop = 0;
			break;
		}
		else if(sz <= 1) continue;

		format = *(string_s(str));
		pgr = PQexec(sqldb, string_s(str) + 1);
		if(pgr == NULL)
		{
			message_status(mes, 127, "Postges error - cannot allocate result\n", MES_SERVER_TO_CLIENT);
			string_clear(str);
		}
		else
		{
			errmes = PQresultErrorMessage(pgr);
			if(*errmes)
			{
				char ws[128];
				snprintf(ws, 127, "postgres error: %s\n", errmes);
				message_status(mes, 127, ws, MES_SERVER_TO_CLIENT);
				string_clear(str);
			}
			else
			{

				if(PQresultStatus(pgr) == PGRES_TUPLES_OK)
					dolines(pgr, format);
				string_clear(str);
				message_send(mes, str, -1, MES_SERVER_TO_CLIENT);
			}
			PQclear(pgr);
		}
	}
	string_delete(str);
}


void dolines(PGresult *pgr, char format)
{
	int i, j, ncol = 0, nrow = 0;

	string *str;
	char *s;

	nrow = PQntuples(pgr);
	ncol = PQnfields(pgr);

	str = new_string();

	for(i=0;i<nrow;i++)
	{
		for(j=0;j<ncol;j++)
		{
			s = PQgetvalue(pgr, i, j);

			dolitem(str, s, (j == ncol - 1), format);
		}
		string_cat_c(str, '\n');
		if(message_send(mes, str, 0, MES_SERVER_TO_CLIENT) < 0) return;
		string_clear(str);
	}
	string_delete(str);
	return;
}

void sighand()
{
	isloop = 0;
}


/*
 * For blanking out passwords....
 */
void wipe(char *s)
{
	while(*s)
	{
		*s = '*';
		s++;
	}
}


