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
 * shsql - the main execution client of the suite
 *
 * This should be the sane for all SQL engines
 *
 * Version 0.6 - introduced format character (first in string)
 * Version 0.7.2 - Bug fix - SQL from stdin fix
 * Version 0.7.3 - Incuded traperror - in order to try and clean up after errors
 */
#include <stdio.h>

#include "string.h"
#include "message.h"
#include "shellsql.h"
#include "traperr.h"



int main(int argc, char *argv[])
{

	string *str;
	char *ts;
	message *mes;
	long key;
	char format = SHSQL_SHELL;
	int use_stdin = 0;

	int c;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s HANDLE sql statement\n", argv[0]);
		exit(1);
	}

	key = strtol(argv[1], &ts, 10);

	if(*ts)
	{
		fprintf(stderr, "%s: HANDLE(first arg) not numeric\n", argv[0]);
		exit(1);
	}

	if((mes = new_message((int)key)) == NULL)
	{
		fprintf(stderr, "%s: Could not get message queue\n", argv[0]);
		exit(1);
	}

	/*
	 * Now we are cooking with gas
	 */

	str = new_string();

	if(argc == 2)
	{
		use_stdin = -1;
	}
	else
	{
		c = 2;
		ts = argv[2];
		if(!strcmp(ts, "--shell"))
		{
			format = SHSQL_SHELL;
			use_stdin = -1;
			c++;
		}
		else if(!strcmp(ts, "--csv"))
		{
			format = SHSQL_CSV;
			use_stdin = -1;
			c++;
		}
		else if(!strcmp(ts, "--colon"))
		{
			format = SHSQL_COLON;
			use_stdin = -1;
			c++;
		}
		else if(!strcmp(ts, "--pipe"))
		{
			format = SHSQL_PIPE;
			use_stdin = -1;
			c++;
		}
		else if(!strcmp(ts, "--tab"))
		{
			format = SHSQL_TAB;
			use_stdin = -1;
			c++;
		}
		if(use_stdin && c < argc) use_stdin = 0;
	}

	string_cat_c(str, format);

	if(use_stdin)
	{
		while((c = getchar()) != EOF)
			string_cat_c(str, c);
	}
	else
	{
		for(;c<argc;c++)
		{
			string_cat(str, argv[c]);
			if(c < argc - 1) string_cat_c(str, ' ');
		}
	}

	/*
	 * Set up error trapping
	 */
	
	trap_init();

	/*
	 * Send the SQL up.....
	 */

	if(message_send(mes, str, -1, MES_CLIENT_TO_SERVER) < 0) 
	{
		message_delete(mes);
		fprintf(stderr, "%s: Could not send SQL query\n", argv[0]);
		string_delete(str);
		exit(1);
	}

	/*
	 * get the query results back
	 */

	for(;;)
	{
		c = 0;

		string_clear(str);
		if (message_receive(mes, str, &c, MES_SERVER_TO_CLIENT) < 0)
		{
			message_delete(mes);
			fprintf(stderr, "%s: %s\n", argv[0], string_s(str));
			string_delete(str);
			exit(1);
		}

		if(!trap_iserror) 
		{
			fputs(string_s(str), stdout);
		}

		if(c) break;
	}

	string_delete(str);
	message_delete(mes);

	if(trap_iserror)
		exit(2);
	else
		exit(0);
}


