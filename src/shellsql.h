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
 * This stores message format characters
 */

#ifndef __SHELLSQL_H
#define __SHELLSQL_H

#include "string.h"

#define SHSQL_SHELL 'A'
#define SHSQL_CSV 'B'
#define SHSQL_COLON 'C'
#define SHSQL_PIPE 'D'
#define SHSQL_TAB 'E'

#define SHSQL_NONE '0'

#define SHSQL_POSTGRES 1
#define SHSQL_MYSQL 2
#define SHSQL_SQLITE3 3
#define SHSQL_ODBC 4
#define SHSQL_FREETDS 5

void dolitem (string *str, char *s, int last, char format);

#endif
