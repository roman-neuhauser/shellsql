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
 * A dinky string object
 *
 * string *new_string()	- Creates the object with zero length string
 * string *new_string(char *) - Creates the object with contents of *s
 * string_set(string *str, char *s)	- Sets the string to contents of *s
 * string_cat(string *str, char *s)	- Appends string with *s
 * string_cat_c(string *str, char c)	- Appends string with character c
 * string_clear(string *str)		- Clears the string (to empty string)
 * string_empty(string *str)		- Empties the string (to empty string)
 * char *string_s(string *str)		- Returns the string as char *
 * int  string_len(string *str)		- Returns the length
 * string_delete(string *str)		- Deletes it (frees resources)
 *
 * string_init(string *str)		- Initializes an existing string structure
 * string_term(string *str)		- Terminates it
 *
 * NB At no time is the string NULL, it always creates and clears with 
 * an empty string
 */

#include "string.h"

string *new_string()
{
	string *str;
	if((str = (string *)malloc(sizeof(string))) == NULL) return NULL;
	if((str->s = malloc(STRING_BLOCK)) == NULL)
	{
		free(str);
		return NULL;
	}
	str->len = 0;
	str->plen = STRING_BLOCK;
	*(str->s) = 0;
	return str;
}

string *new_string_s(char *s)
{

	string *str;
	int plen;

	str->len = strlen(s);
	plen = (((str->len + 1)/ STRING_BLOCK) + 1) * STRING_BLOCK;

	
	if((str = (string *)malloc(sizeof(string))) == NULL) return NULL;
	if((str->s = malloc(plen)) == NULL)
	{
		free(str);
		return NULL;
	}
	str->plen = plen;
	strcpy(str->s, s);
	return str;
}
	

/*
 * Initialize an existing string 
 */
string *string_init(string *str)
{
	if((str->s = malloc(STRING_BLOCK)) == NULL)
	{
		return NULL;
	}
	str->len = 0;
	str->plen = STRING_BLOCK;
	return str;
}
/*
 * Terminate it
 */
void string_term(string *str)
{
	free(str->s);
}


string *string_cat(string *str, char *s)
{
	int plen;
	str->len += strlen(s);

	plen = (((1 + str->len)/ STRING_BLOCK) + 1) * STRING_BLOCK;

	
	if(plen > str->plen)
	{
		if((str->s = realloc(str->s, plen)) == NULL)
		{
			free(str);
			return NULL;
		}
		str->plen = plen;
	}
		
	strcat(str->s, s);
	return str;
}

string *string_cat_c(string *str, char c)
{
	int plen;

	plen = (((2 + str->len)/ STRING_BLOCK) + 1) * STRING_BLOCK;

	
	if(plen > str->plen)
	{
		if((str->s = realloc(str->s, plen)) == NULL)
		{
			free(str);
			return NULL;
		}
		str->plen = plen;
	}
		
	str->s[str->len] = c;
	str->len++;
	str->s[str->len] = 0;
	return str;
}

string *string_set(string *str, char *s)
{
	int plen;
	str->len = strlen(s);

	plen = (((1 + str->len)/ STRING_BLOCK) + 1) * STRING_BLOCK;

	
	if(plen != str->plen)
	{
		if((str->s = realloc(str->s, plen)) == NULL)
		{
			free(str);
			return NULL;
		}
		str->plen = plen;
	}
		
	strcpy(str->s, s);
	return str;
}

string *string_clear(string *str)
{
	str->len = 0;

	if(STRING_BLOCK != str->plen)
	{
		if((str->s = realloc(str->s, STRING_BLOCK)) == NULL)
		{
			free(str);
			return NULL;
		}
		str->plen = STRING_BLOCK;
	}
		
	*(str->s) = 0;
	return str;
}
string *string_empty(string *str)
{
	str->len = 0;
	*(str->s) = 0;
	return str;
}

char *string_s(string *str)
{
	return str->s;
}
long string_len(string *str)
{
	return str->len;
}

void string_delete(string *str)
{
	free(str->s);
	free(str);
}

char string_last(string *str)
{
	if(str->len)
		return str->s[str->len - 1];
	return 0;
}

void string_minus(string *str)
{
	if(str->len)
	{
		(str->len)--;
		str->s[str->len] = 0;
	}
}


