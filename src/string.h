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
 * A dinky C string object
 */

#ifndef __STRING_H
#define __STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_BLOCK 32


struct string_t {
	long len;
	long plen;
	char *s;
};

typedef struct string_t string;


string *new_string();
string *new_string_s(char *s);

string *string_cat(string *str, char *s);
string *string_cat_c(string *str, char c);
string *string_set(string *str, char *s);
string *string_clear(string *str);
string *string_empty(string *str);

char *string_s(string *str);
long string_len(string *str);

void string_delete(string *str);

char string_last(string *str);
void string_minus(string *str);

string *string_init(string *str);
void string_term(string *str);


#endif
