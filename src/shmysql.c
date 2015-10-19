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
 * MYSQL  - Coded with 4.1
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <mysql.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>

#include "string.h"
#include "message.h"
#include "shellsql.h"
#include "sarg.h"

#define SMYSQL_DUMMY 0
#define SMYSQL_HOST 1
#define SMYSQL_USER 2
#define SMYSQL_PASSWORD 3
#define SMYSQL_DBNAME 4
#define SMYSQL_PORT 5
#define SMYSQL_SOCKET 6
#define SMYSQL_FLAG 7

#define SMYSQL_PARAMETERS 8

typedef void (*sighandler_t)(int);
void mainloop();
void sighand();
void dolines(MYSQL_RES *res, char format);
void wipe(char *s);
void parse_strings(char **sqlarg, char *ins);

/*
 * Some global items.....
 */

MYSQL *sqldb;
message *mes;
int isloop = -1;

int main(int argc, char *argv[])
{
	int errn;
	string *str;
	int complete = -1;
	int i;

	
	char *sqlname[] = {
		"_dummy",
		"host",
		"user",
		"password",
		"dbname",
		"port",
		"socket",
		"flag"
	};

	sqlarg *sarg;


	/*
	 * Username and password may be in the connection string
	 */

	long flag = 0;
	unsigned int port = 0;

	MYSQL *tdb;

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
		if((!strncasecmp(argv[i], "password", 9))
		 || (!strncasecmp(argv[i], "user", 4))
		 || (!strncasecmp(argv[i], "username", 8)))
			wipe(argv[i]);
		if(i < argc - 1) string_cat_c(str, ' ');
	}

	sarg = new_sqlarg(sqlname, string_s(str), SMYSQL_PARAMETERS);

	string_delete(str);

	if(sarg == NULL)
	{
		message_status(mes, 127, "Error allocating string parameters\n", MES_SERVER_TO_CLIENT);
		message_delete(mes);
		exit(1);
	}

	if(sqlargi(sarg, SMYSQL_PORT) != NULL) port = atoi(sqlargi(sarg, SMYSQL_PORT));
	if(sqlargi(sarg, SMYSQL_FLAG) != NULL) flag = atol(sqlargi(sarg, SMYSQL_FLAG));


	sqldb = mysql_init(NULL);
	if(sqldb == NULL)
	{
		message_status(mes, 127, "Error allocating connection\n", MES_SERVER_TO_CLIENT);
		sqlarg_delete(sarg);
		message_delete(mes);
		exit(1);
	}

	tdb = mysql_real_connect(sqldb, sqlargi(sarg, SMYSQL_HOST), sqlargi(sarg, SMYSQL_USER), sqlargi(sarg, SMYSQL_PASSWORD),
					  sqlargi(sarg, SMYSQL_DBNAME), port, sqlargi(sarg, SMYSQL_SOCKET), flag);

	sqlarg_delete(sarg);

	

	if(tdb == NULL)
	{
		char ws[128];
		snprintf(ws, 127, "Error opening mysql: %s\n", mysql_error(sqldb));
		ws[127] = 0;
		message_status(mes, 127, ws, MES_SERVER_TO_CLIENT);
		mysql_close(sqldb);
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

	mysql_close(sqldb);
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

	string *str = NULL;

	MYSQL_RES *res;

	int complete = -1;
	int ans;
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
		ans = mysql_query(sqldb, string_s(str) + 1);
		string_clear(str);

		if(ans)
		{
			message_status(mes, 127, "cannot execute mysql query", MES_SERVER_TO_CLIENT);
		}
		else
		{
			if((res = mysql_store_result(sqldb)) == NULL)
			{
				if(mysql_field_count(sqldb))
					message_status(mes, 127, "Error retrieveing mysql data", MES_SERVER_TO_CLIENT);
				else
					message_send(mes, str, -1, MES_SERVER_TO_CLIENT); /* Is a DML query */
			}
			else
			{
				dolines(res, format);
				mysql_free_result(res);
				message_send(mes, str, -1, MES_SERVER_TO_CLIENT); /* Is a DML query */
			}
		}

	}
	string_delete(str);
}


void dolines(MYSQL_RES *res, char format)
{
	int j;
	unsigned int ncol = 0;

	string *str;
	char *s;

	MYSQL_ROW row;

	ncol = mysql_num_fields(res);

	str = new_string();

	while((row = mysql_fetch_row(res)) != NULL)
	{
		for(j=0;j<ncol;j++)
		{
			s = row[j];
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

