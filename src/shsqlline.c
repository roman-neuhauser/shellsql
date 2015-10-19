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
 * This is a utility that processes outputs from shsql
 *
 * The syntax in bash shell script is something like....
 *
 * shsql $HANDLE "select * from mytable " | (
 * while RECORD=`shsqlline`
 * do
 *     set $RECORD
 *    echo Column1: $1, Column2: $2, Column3 $3
 * done
 * )
 *
 */

/*
 * Version 0.7 - include the --csv etc parameter
 */
#include <stdio.h>
#include <unistd.h>
#include "shellsql.h"

void outchar(char c);

char format = SHSQL_SHELL;
char fchr;
string *str = NULL;

int main(int argc, char *argv[])
{
	int mode = 0;
	char c;

	if(argc == 2)
	{
		if(!strcmp(argv[1], "--csv"))
			format = SHSQL_CSV;
		else if(!strcmp(argv[1], "--colon"))
		{
			format = SHSQL_COLON;
			fchr = ':';
		}
		else if(!strcmp(argv[1], "--pipe"))
		{
			format = SHSQL_PIPE;
			fchr = '|';
		}
		else if(!strcmp(argv[1], "--tab"))
		{
			format = SHSQL_TAB;
			fchr = '\t';
		}
		else if(!strcmp(argv[1], "--shell"))
			format = SHSQL_SHELL;
		else
			mode = -1;
	}
	else if(argc > 1)
		mode = -1;
	else
		format = SHSQL_SHELL;

	if(mode == -1)
	{
		fprintf(stderr, "Usage: %s [ --shell | --csv | --colon | --pipe | --tab ]\n", argv[0]);
		return 1;
		exit(1);
	}

	if(format != SHSQL_SHELL) str = new_string();

	for(;;)
	{
		/*
		 * Cannot use buffered reads as the buffered but not read
	         * characters would be lost on the pipe stream when the
	   	 * program closes
		 */
		if(read(0, &c, 1) < 1)
		{
			mode = 1;
			break;
		}

		outchar(c);
		if(c == '\n' && (!mode)) break;
	
		if(c == '"')
		{
			if(mode)
				mode = 0;
			else
				mode = -1;
		}
	}
	if(str != NULL)
	{
		fputs(string_s(str), stdout);
		string_delete(str);
	}
	fputc('\n', stdout);
	return mode;
}


/*
 * Outchar as opposed to putchar
 *
 * This checks format and does the appropriate thing
 *
 * I will use global variables here as it is easy and I
 * am not multi-threading or anything silly
 *
 */


int lmode = 0;

void outchar(char c)
{
	switch(format) {
	case SHSQL_SHELL:	/* An EASY one */
		fputc(c, stdout);
		c = 0;
		break;
	case SHSQL_CSV:
		if(lmode == 10)	/* End of (last)field */
		{
			if(c > ' ')
			{
				string_cat_c(str, ' ');
				lmode = 0;
			}
			else if(c == '\n')
			{
				string_cat_c(str, ' ');
				string_cat_c(str, '"');
				string_cat_c(str, '"');
				c = 0;
			}
			else
				c = 0;
			
		}
			
		if(lmode == 0)	/* Start of line/field */
		{
			if(c <= ' ')	/* No preceding spaces */
				c = 0;
			else if(c == ',')
			{
				string_cat_c(str, '"');
				string_cat_c(str, '"');
				lmode = 10;
				c = 0;
			}
			else if(c != '"')
			{
				string_cat_c(str, '"');
				lmode = 1;
			}
			else if(c == '\n')
				c = 0;
			else
				lmode = 2;

		}
		else if(lmode == 1)	/* In field, no quotes */
		{
			if(c == ',' || c == '\n')
			{
				char d;
				for(;;)
				{
					d = string_last(str);
					if( d == 0 || d > ' ') break;
					string_minus(str);
				}
				string_cat_c(str, '"');
				lmode = 10;
				c = 0;
			}
		}
		else if(lmode == 2)	/* In field - Quote */
		{
			if(c == '"') 
			{
				lmode = 3;
				c = 0;
			}
		}
		else if(lmode == 3)	/* In field - quote - come accross a quote */
		{
			if(c == '"')
			{
				/*
				 * Double double quotes
				 */
				string_cat_c(str, '"'); 
				/* Second one put in by not making c = 0 */
				lmode = 2;
			}
			else if(c == ',' || c == '\n')
			{
				string_cat_c(str, '"');
				lmode = 10;
				c = 0;
			}
			else
			{
				lmode = 4;
				c = 0;
			}
		}
		else if(lmode == 4)	/* Between/at end of quotes for quoted fields */
		{
			if(c == '"')
				lmode = 2;
			else if(c == ',' || c == '\n')
			{
				string_cat_c(str, '"');
				lmode = 10;
			}
			c = 0;
		}


		break;
	case SHSQL_PIPE:
	case SHSQL_COLON:
	case SHSQL_TAB:	
		if(lmode == 10)	/* End of field */
		{
			if(c == '\n')
			{
				string_cat_c(str, ' ');
				string_cat_c(str, '"');
				string_cat_c(str, '"');
				c = 0;
			}
			else
			{
				string_cat_c(str, ' ');
				lmode = 0;
			}
		}
		if(lmode == 0)	/* Start of field */
		{
			if(c == fchr)
			{
				string_cat_c(str, '"');
				string_cat_c(str, '"');
				c = 0;
				lmode = 10;
			}
			else if(c == '\n')
				c = 0;
			else
			{
				string_cat_c(str, '"');
				lmode = 1;
			}
		}
		else if(lmode == 1)	/* In field */
		{
			if(c == '"')
				string_cat_c(str, '"');
			else if (c == '\\')
			{
				lmode = 2;
				c = 0;
			}
			else if(c == fchr || c == '\n')
			{
				string_cat_c(str, '"');
				c = 0;
				lmode = 10;
			}
		}
		else if(lmode == 2)	/* In escape mode */
		{
			char d = c;
			switch(c) {
			case 'n':	d = '\n';	break;
			case 'r':	d = '\r';	break;
			case 't':	d = '\t';	break;
			default:	d = c;
			}
			c = d;
			lmode = 1;
		}
		break;
	}

	if(c) string_cat_c(str, c);
}
				

