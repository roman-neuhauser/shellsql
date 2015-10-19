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
 * Line items - code consolidated here
 */

#include "string.h"
#include "shellsql.h"

void outstring(string *str, char *s, char format);
char outesc(string *str, char c, char format);


void dolitem(string *str, char *s, int last, char format)
{
	switch(format) {
	case SHSQL_SHELL:
		string_cat_c(str, '"');
		if(s != NULL) outstring(str, s, format);
		string_cat_c(str, '"');
		if(!last)  string_cat_c(str, ' ');
		break;
	case SHSQL_CSV:
		string_cat_c(str, '"');
		if(s != NULL) outstring(str, s, format);
		string_cat_c(str, '"');
		if(!last)  string_cat_c(str, ',');
		break;
	case SHSQL_PIPE:
		if(s != NULL) outstring(str, s, format);
		if(!last)  string_cat_c(str, '|');
		break;
	case SHSQL_COLON:
		if(s != NULL) outstring(str, s, format);
		if(!last)  string_cat_c(str, ':');
		break;
	case SHSQL_TAB:
		if(s != NULL) outstring(str, s, format);
		if(!last)  string_cat_c(str, '\t');
		break;
	}
}


void outstring(string *str, char *s, char format)
{
	
	char c;

	for(;*s;s++)
	{
		c = *s;
		switch(format) {
		case SHSQL_SHELL:
		case SHSQL_CSV:
			if(c == '"') string_cat_c(str, '"');
			break;
		case SHSQL_COLON:
			if(c == ':') string_cat_c(str, '\\');
			c = outesc(str, c, format);
			break;
		case SHSQL_PIPE:
			if(c == '|') string_cat_c(str, '\\');
			c = outesc(str, c, format);
			break;
		case SHSQL_TAB:
			c = outesc(str, c, format);
			break;
		}
		if(c) string_cat_c(str, c);
	}
}

char outesc(string *str, char c, char format)
{
	switch(c) {
	case '\t':	string_cat(str, "\\t");		c = 0; break;
	case '\n':	string_cat(str, "\\n");		c = 0; break;
	case '\r':	string_cat(str, "\\r");		c = 0; break;
	case '\\':	string_cat(str, "\\\\");	c = 0; break;
	}
	return c;
}
			

