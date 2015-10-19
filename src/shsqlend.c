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
 * shsqlend - Close the SQL stuff
 *
 * This should be the sane for all SQL engines
 */
#include <stdio.h>

#include "string.h"
#include "message.h"



int main(int argc, char *argv[])
{

	char *ts;
	message *mes;
	long key;

	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s HANDLE\n", argv[0]);
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

	if(message_end(mes, MES_CLIENT_TO_SERVER) < 0)
	{
		/*
		 * Error - delete it anyway
		 */
		message_destroy(mes);
		message_delete(mes);
		exit(1);
	}

	/*
	 * The server should be dead now
	 */

	message_delete(mes);
	exit(0);
}


