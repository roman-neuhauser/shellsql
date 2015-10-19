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
 * shsqlesc
 *
 * Used to escape characters for SQL parameters
 *
 * Uses SHSQL variable to control things, for details
 * on the differences between engines see the catstr() function
 * here
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string.h"
#include "shellsql.h"

void catstr(string *str, char *s);

int mode = SHSQL_POSTGRES;

int main(int argc, char *argv[])
{
	char *t;
	string *str;
	int i;

	t = getenv("SHSQL");

	if(t != NULL)
	{
		if(!strcmp(t, "postgres"))
			mode = SHSQL_POSTGRES;
		else if(!strcmp(t, "mysql"))
			mode = SHSQL_MYSQL;
		else if(!strcmp(t, "sqlite3"))
			mode = SHSQL_SQLITE3;
		else if(!strcmp(t, "odbc"))
			mode = SHSQL_ODBC;
		else if(!strcmp(t, "freetds"))
			mode = SHSQL_FREETDS;
		else
			mode = SHSQL_POSTGRES;
	}
	else
		mode = SHSQL_POSTGRES;


	str = new_string();

	for(i=1;i<argc;i++)
	{
		catstr(str, argv[i]);
		if(string_len(str) && i < argc - 1)
			string_cat_c(str, ' ');
	}

	if(string_len(str))
	{
		fputc('\'', stdout);
		fputs(string_s(str), stdout);
		fputc('\'', stdout);
	}
	else
		fputs("NULL", stdout);

	string_delete(str);
	return 0;
}

/*
 * The only current difference is that POSTGRES needs to escape the backquotes
 * while everything else does not
 */
void catstr(string *str, char *s)
{
	while(*s)
	{
		if(!(*s)) return;

		switch(*s) {
		case '\'':
			string_cat(str, "''");
			break;
		/*
		case '"':
			string_cat(str, "\"\"");
			break;
		 */
		case '\\':
			if(mode == SHSQL_POSTGRES)
				string_cat(str, "\\\\");
			else
				string_cat_c(str, *s);
			break;
		default:
			string_cat_c(str, *s);
			break;
		}
		s++;
	}
}

		


