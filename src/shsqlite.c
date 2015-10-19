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
 * SQLITE3 for those who like the simple life..
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sqlite.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>

#include "string.h"
#include "message.h"
#include "shellsql.h"

typedef void (*sighandler_t)(int);
void mainloop();
void sighand();
int callback(void *ptr, int ncol, char **col, char **name);

/*
 * Some global items.....
 */

sqlite *sqldb;
message *mes;
int isloop = -1;
char format;

int main(int argc, char *argv[])
{
	int errn;
	string *str;
	int complete = -1;

	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s dbfilename\n", argv[0]);
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

	if((sqldb = sqlite_open(argv[1], 0, NULL)) == NULL)
	{
		sqlite_close(sqldb);
		message_status(mes, 127, "Cannot open database", MES_SERVER_TO_CLIENT);
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

	sqlite_close(sqldb);
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
	string *nstr = NULL;

	int complete = -1;

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

		nstr = new_string();

		format = *(string_s(str));
		if(sqlite_exec(sqldb, string_s(str) + 1, callback, (void *)nstr, &errmes) != SQLITE_OK)
		{
			message_status(mes, 127, errmes, MES_SERVER_TO_CLIENT);
			free(errmes);
		}

		string_clear(str);
		message_send(mes, str, -1, MES_SERVER_TO_CLIENT);

		string_delete(nstr);

	}
	string_delete(str);
}


int callback(void *ptr, int ncol, char **col, char **name)
{
	int i;

	string *str;

	str = (string *) ptr;
	for(i=0;i<ncol;i++)
	{
		dolitem(str, col[i], (i == ncol - 1), format);
	}

	string_cat_c(str, '\n');

	message_send(mes, str, 0, MES_SERVER_TO_CLIENT);
	return 0;
}


void sighand()
{
	isloop = 0;
}

