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
 * The SHSQLINP routine
 *
 * Takes arguments as the SQL and standard input as the individual parameters
 *
 * This executes an "insert" for each line, so transactions are useful
 * here, Postgres users may want to do a "PREPARE .." etc here.  SQLITE3
 * users will CERTAINLY want to use transactions.
 */

#include <stdio.h>

#include "string.h"
#include "message.h"
#include "shellsql.h"
#include "strarr.h"
#include "traperr.h"


int getargpos(strarr *arr, char *s);
int getline(strarr *arr, char format, char fchr);
int out_put_c(strarr *arr, char c);


int mode = SHSQL_POSTGRES;



int main(int argc, char *argv[])
{

	string *str;
	char *ts;
	message *mes;
	long key;
	char format = SHSQL_SHELL;
	char fchr = 0;
	strarr *arr, *arrin;
	char *t;
	int numstr;
	int i, j;

	int c;

	if(argc < 3)
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
		else
			mode = SHSQL_POSTGRES;
	}
	else
		mode = SHSQL_POSTGRES;

	str = new_string();

	c = 2;
	ts = argv[2];
	if(!strcmp(ts, "--shell"))
	{
		format = SHSQL_SHELL;
		c++;
	}
	else if(!strcmp(ts, "--csv"))
	{
		format = SHSQL_CSV;
		c++;
	}
	else if(!strcmp(ts, "--colon"))
	{
		format = SHSQL_COLON;
		fchr = ':';
		c++;
	}
	else if(!strcmp(ts, "--pipe"))
	{
		format = SHSQL_PIPE;
		fchr = '|';
		c++;
	}
	else if(!strcmp(ts, "--tab"))
	{
		format = SHSQL_TAB;
		fchr = '\t';
		c++;
	}

	for(;c<argc;c++)
	{
		string_cat(str, argv[c]);
		if(c < argc - 1) string_cat_c(str, ' ');
	}


	/*
	 * Prepare the SQL.......
	 */

	arr = new_strarr();
	arrin = new_strarr();
	getargpos(arr, string_s(str));
	string_clear(str);

	numstr = strarr_num(arr);

	/*
	 * Set up error trapping
	 */

	trap_init();

	/*
	 * Now lets loop it
	 */


	while(getline(arrin, format, fchr) >= 0)
	{
		j = 0;
		i = 0;
		string_cat_c(str, SHSQL_NONE);
		while(i<numstr)
		{
			string_cat(str, strarr_out(arr, i));
			i++;

			if(i < numstr)
			{

				/*
				 * For now - Empty strings/numbers are NULL
				 */

				if(!(*(strarr_out(arrin, j))))
				{
					string_cat(str, "NULL");
				}
				else if((*(strarr_out(arr, i)) == '@'))
				{
					string_cat_c(str, '\'');
					string_cat(str, strarr_out(arrin, j));
					string_cat_c(str, '\'');
				}
				else
					string_cat(str, strarr_out(arrin, j));
				j++;
				i++;
			}
		}

		if(message_send(mes, str, -1, MES_CLIENT_TO_SERVER) < 0) 
		{
			message_delete(mes);
			fprintf(stderr, "%s: Could not send SQL query\n", argv[0]);
			string_delete(str);
			strarr_delete(arr);
			strarr_delete(arrin);
			exit(1);
		}

	/*
	 * get the query results back
	 */

		for(;;)
		{
			c = 0;
	
			string_empty(str);
			if (message_receive(mes, str, &c, MES_SERVER_TO_CLIENT) < 0)
			{
				message_delete(mes);
				fprintf(stderr, "%s: %s\n", argv[0], string_s(str));
				string_delete(str);
				strarr_delete(arr);
				strarr_delete(arrin);
				exit(1);
			}
	
			if(string_len(str) && (!trap_iserror))
				fputs(string_s(str), stdout);

			if(c) break;
		}

		string_empty(str);
		strarr_clear(arrin);

		if(trap_iserror) break;
	}

	string_delete(str);
	strarr_delete(arr);
	strarr_delete(arrin);
	message_delete(mes);

	if(trap_iserror)
		exit(1);
	else
		exit(0);
}




/*
 * The format for this, always 2p+1 strings (where p is para
 *
 * First string - first bit of SQL up to first para
 * Second string, # or @ depending on number or string
 * Third bit next bit of SQL (or end) and so on
 */
int getargpos(strarr *arr, char *s)
{
	int mode = 0;
	char delim;
	int delev = 0;
	for(;*s;s++)
	{
		if(mode == 0)
		{
			if(*s == '?')
			{
				strarr_end(arr);
				mode = 1;
			}
			else 
			{
				strarr_put_c(arr, *s);
				switch(*s) {
				case '\'':
				case '"':
				case '`':	/* For MySQL */
					delim = *s;
					mode = 2;
					break;
				case '[':	/* Some SQLs have this */
					delim = ']';
					mode = 3;
					delev++;
					break;
				}
			}
		}
		else if(mode == 1)	/* This is a parameter */
		{
			switch(*s) { 
			case '#':
			case '@':
				strarr_put_c(arr, *s);
				break;
			default:
				strarr_put_c(arr, '@');
				s--;
				break;
			}
			strarr_end(arr);
			mode = 0;
		}
		else if(mode == 2)	/* In normal quotes */
		{
			strarr_put_c(arr, *s);
			if(*s == delim) mode = 4;
		}
		else if(mode == 3)	/* In [ ] quotes */
		{
			strarr_put_c(arr, *s);
			switch(*s) {
			case '[':
				delev++;
				break;
			case ']':
				delev--;
				break;
			}
			if(!delev) mode = 0;
		}
		else if(mode == 4)	/* Possible double quotes */
		{
			if(*s == delim) 
				mode = 2;
			else
			{
				mode = 0;
				s--;
			}
		}
	}	
	strarr_end(arr);
	return 0;
}




/*
 * This stuff needs to escape things....
 */
int out_put_c(strarr *arr, char c)
{
	switch(c) {
	case '\'':
		strarr_put_c(arr, '\\');
		break;
	case '\\':
		if(mode == SHSQL_POSTGRES) strarr_put_c(arr, '\\');
		break;
	}
	return strarr_put_c(arr, c);
}
		
	

int getline(strarr *arr, char format, char fchr)
{
	int ic;
	char c;
	char delim;
	char d;
	int numl = 0;

	int lmode = 0;

	strarr_clear(arr);

	for(;;)
	{
		ic = getchar();
		if(ic == EOF) return -1;
		c = (char) ic;

		switch(format) {

		case SHSQL_SHELL:	/* Space(s) separated with quote or double quote */
			if(lmode == 0)	/* Beginning of line of field */
			{
				if(c <= ' ')
					c = 0;
				else if(c == '"' || c == '\'')
				{
					c = 0;
					lmode = 1;
					delim = c;
				}
				else if(c == '\n')
				{
					return strarr_num(arr);
				}
				else
				{
					lmode = 2;
				}
			}
			else if(lmode == 1)	/* In quotes */
			{
				if(c == delim)
				{
					c = 0;
					lmode = 3;
				}
			}
			else if(lmode == 2)	/* No quotes TODO? Honour backslash? */ 
			{
				if(c == '\n')
				{
					strarr_end(arr);
					return strarr_num(arr);
				}
				else if(c <= ' ')
				{
					strarr_end(arr);
					lmode = 0;
					c = 0;
				}
				else if(c == '"' || c == '\'')
				{
					delim = c;
					c = 0;
					lmode = 1;
				}
			}
			else if(lmode == 3)	/* Possible end of quoted field? */
			{
				if(c == delim)
					lmode = 1;	/* Leave c to append delim char */
				else if(c == '\n')
				{
					strarr_end(arr);
					c = 0;
					return strarr_num(arr);
				}
				else if(c <=  ' ')
				{
					strarr_end(arr);
					c = 0;
					lmode = 0;
				}
				else
					lmode = 2;
			}
			break;
				
		case SHSQL_CSV:
			if(lmode == 10)	/* End of (last)field */
			{
				if(c > ' ')
					lmode = 0;
				else if(c == '\n')
				{
					strarr_end(arr);
					return strarr_num(arr);
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
					strarr_end(arr);
					lmode = 10;
					c = 0;
				}
				else if(c != '"')
				{
					out_put_c(arr, c);
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
					while(strarr_last(arr) <= ' ') strarr_minus(arr);
					strarr_end(arr);
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
					/* Quote put in by not making c = 0 */
					lmode = 2;
				}
				else if(c == ',' || c == '\n')
				{
					strarr_end(arr);
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
					strarr_end(arr);
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
					strarr_end(arr);
					c = 0;
				}
				else
				{
					lmode = 0;
				}
			}
			if(lmode == 3)
			{
				if(c < '0' || c > '7' || numl >= 3)
				{
					out_put_c(arr, d);
					lmode = 1;
					d = 0;
					numl = 0;
				}
			}

			if(lmode == 0)	/* Start of field */
			{
				if(c == fchr)
				{
					strarr_end(arr);
					c = 0;
					lmode = 10;
				}
				else if(c == '\n')
					c = 0;
				else
				{
					lmode = 1;
				}
			}
			else if(lmode == 1)	/* In field */
			{
				if (c == '\\')
				{
					lmode = 2;
					d = 0;
					c = 0;
					numl = 0;
				}
				else if(c == fchr || c == '\n')
				{
					strarr_end(arr);
					c = 0;
					lmode = 10;
				}
			}
			else if(lmode == 2)	/* In escape mode */
			{
				switch(c) {
				case 'n':	d = '\n';	break;
				case 'r':	d = '\r';	break;
				case 't':	d = '\t';	break;
				case 'N':	d = 0;		break;	/* This is NULL */

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					numl++;
					lmode = 3;
					d = (d << 3 ) & 0xff;
					d |= (c & 0x03);
					break;
					
				default:	d = c;		break;
				}
				c = d;
				lmode = 1;
			}
			else if(lmode == 3)	/* In escape mode - Numeric */
			{
				if(c >= '0' && c <= '7')
				{
					numl++;
					lmode = 3;
					d = (d << 3 ) & 0xff;
					d |= (c & 0x03);
				}
			}
		}
	
		if(c) out_put_c(arr, c);
	}
	return 0;
}
		
