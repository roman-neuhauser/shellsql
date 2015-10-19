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
 * This is a wrapper for shpostgres, shmysql, shsqlite3, shodbc and any other
 * engine I may include
 *
 * It uses the SHSQL variable to decide what executeable to run (prepending "sh")
 * then execs it with same parameters
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

	char *bname;
	char *t;

	char ws[256];

	/*
	 * Assume all ececuteables in the same directory
 	 */

	bname = strdup( argv[0]);
	t = dirname(bname);

	if(strlen(t) >= 240)
	{
		fprintf(stderr, "%s: Command path too big\n", argv[0]);
		exit (1);
	}

	strncpy(ws, dirname(bname), 220);
	ws[240] = 0;
	free(bname);
	strcat(ws, "/sh");

	t = getenv("SHSQL");
	if(t == NULL)
		t = "postgres";
	else if (!(*t))
		t = "postgres";

	if (strlen(t) > 32)
	{
		fprintf(stderr, "%s: Invalid SHSQL value\n", argv[0]);
		exit (1);
	}

	strcat(ws, t);

	/*
	 * I do not know if I can use execvp and argv like this
 	 * but I think I can get away with it
	 */
	execvp(ws, argv);

	/*
	 * should never reach here.....
	 */

	fprintf(stderr, "%s: Failed to start %s\n", argv[0], ws);
	exit(1);
}

