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
 * SHODBC 
 */

/*
 * Changes
 *
 * 26-aug-2005 - Placed strdup'ed username and password 
 *             - submitted by Walter Haslbeck <haslbeck@t-online.de>
 */
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <strings.h>
#include <string.h>

#include "string.h"
#include "message.h"
#include "shellsql.h"

typedef void (*sighandler_t)(int);
void mainloop();
void sighand();
void dolines(SQLHSTMT *stmt, SQLSMALLINT ncol, char format);
void wipe(char *s);

/*
 * Some global items.....
 */

SQLHENV env;
SQLHDBC hdbc;
message *mes;
int isloop = -1;

int main(int argc, char *argv[])
{
	int errn;
	string *str;
	int complete = -1;
	int i;
	long ans;
	int logintimeout = 20;
	char *s;
	char *password;
	char *username;

	if(argc < 4)
	{
		fprintf(stderr, "Usage: %s user password connecstring\n", argv[0]);
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

	for(i=3;i<argc;i++)
	{
		string_cat(str, argv[i]);
		if(i < argc - 1) string_cat_c(str, ' ');
	}

	username = strdup(argv[1]);
	password = strdup(argv[2]);

	wipe(argv[2]);
	wipe(argv[1]);

	ans = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &(env));
	if((ans != SQL_SUCCESS) && (ans != SQL_SUCCESS_WITH_INFO))
	{
		message_status(mes, 127, "ODBC: Cannot allocate environment handle\n", MES_SERVER_TO_CLIENT);
		message_delete(mes);
		free(username);
		free(password);
		exit(1);
	}

	ans = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0); 

	if ((ans != SQL_SUCCESS) && (ans != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_ENV, env);
		message_status(mes, 127, "ODBC: Cannot set environment handle attributes\n", MES_SERVER_TO_CLIENT);
		message_delete(mes);
		exit(1);
	}

	/* 2. allocate connection handle, set timeout */

	ans = SQLAllocHandle(SQL_HANDLE_DBC, env, &(hdbc)); 

	if ((ans != SQL_SUCCESS) && (ans != SQL_SUCCESS_WITH_INFO))
	{
		SQLFreeHandle(SQL_HANDLE_ENV, env);
		message_status(mes, 127, "ODBC: Cannot allocate database handle\n", MES_SERVER_TO_CLIENT);
		message_delete(mes);
		free(username);
		free(password);
		exit(1);
	}

	/*
 	 * TODO - Parameterize ODBC_TIMEOUT as environment variable
	 */

	s = getenv("ODBC_TIMEOUT");
	if(s != NULL)
	{
		if(*s)
		{
			logintimeout = atol(s);
		}
	}

	SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *)logintimeout, 0);

	/* 3. Connect to the datasource  */

	ans = SQLConnect(hdbc, (SQLCHAR*) string_s(str), SQL_NTS,
                                     (SQLCHAR*) username, SQL_NTS,
                                     (SQLCHAR*) password, SQL_NTS);

	if ((ans != SQL_SUCCESS) && (ans != SQL_SUCCESS_WITH_INFO))
	{
		message_status(mes, 127, "ODBC: Cannot connect to database\n", MES_SERVER_TO_CLIENT);
		message_delete(mes);
/*
		SQLGetDiagRec(SQL_HANDLE_DBC, V_OD_hdbc,1, 
		              V_OD_stat, &V_OD_err,V_OD_msg,100,&V_OD_mlen);
		printf("%s (%d)\n",V_OD_msg,V_OD_err);
 */
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		SQLFreeHandle(SQL_HANDLE_ENV, env);
		free(username);
		free(password);
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

	SQLDisconnect(hdbc);
	SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, env);

	message_destroy(mes);
	message_delete(mes);

	free(username);
	free(password);

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

	int complete = -1;
	int ans;

	SQLSMALLINT ncol;

	SQLHSTMT stmt;
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
		ans = SQLAllocHandle(SQL_HANDLE_STMT, (SQLHANDLE) hdbc, (SQLHANDLE *)stmt);

		if((ans != SQL_SUCCESS) && ( ans != SQL_SUCCESS_WITH_INFO))
		{
			message_status(mes, 127, "ODBC: Cannot allocate statement handle\n", MES_SERVER_TO_CLIENT);
			return;
		}

		ans = SQLPrepare(stmt, string_s(str) + 1, SQL_NTS);

		if((ans != SQL_SUCCESS) && ( ans != SQL_SUCCESS_WITH_INFO))
		{
			message_status(mes, 127, "ODBC: Cannot execute statement\n", MES_SERVER_TO_CLIENT);
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			return;
		}

		ans = SQLNumResultCols(stmt, &(ncol));
		if(ans != SQL_SUCCESS && ans != SQL_SUCCESS_WITH_INFO)
		{
			message_status(mes, 127, "ODBC: - cannot determine SQL type\n", MES_SERVER_TO_CLIENT);
			SQLFreeHandle(SQL_HANDLE_STMT, stmt);
			return;
		}

		if(ncol) dolines(&stmt, ncol, format);
		SQLFreeHandle(SQL_HANDLE_STMT, stmt);

		string_clear(str);
		message_send(mes, str, -1, MES_SERVER_TO_CLIENT);


	}
	string_delete(str);
}


void dolines(SQLHSTMT *stmt, SQLSMALLINT ncol, char format)
{
	int j;
	long ans;

	string *str, *tstr;

	char istr[256];
	long sl = 0, sb = 0;

	SQLINTEGER si;

	str = new_string();
	tstr = new_string();

	for(;;)
	{
		ans = SQLFetch(*stmt);

		/*
		 * I assume error here is EOF...
		 */
		if((ans != SQL_SUCCESS) && ( ans != SQL_SUCCESS_WITH_INFO))
			break;

		for(j=0;j<ncol;j++)
		{

			si = 0;

			for(;;)
			{
				ans = SQLGetData(stmt, j+1, SQL_C_CHAR, istr, 256, &si);
				if(ans == SQL_NO_DATA) break;

				if(si == SQL_NO_TOTAL)
				{
					sb += 255;	
					sl = 255;
				}
				else
				{
					sb += si;
					sl = si;
				}

				string_cat(tstr, istr);
		
				if(si != SQL_NO_TOTAL) break;
			}
			dolitem(str, string_s(tstr), (j == ncol - 1), format);
			string_clear(tstr);
		}
		string_cat_c(str, '\n');
		if(message_send(mes, str, 0, MES_SERVER_TO_CLIENT) < 0) return;
		string_clear(str);
	}
	string_delete(str);
	string_delete(tstr);
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


