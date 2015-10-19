#!/bin/sh

#    SHSQL suite - SQL utility for LINUX/UNIX shell scriptiing
#    Copyright (C) 2004  Edward Macnaghten
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    #the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#    Edward Macnaghten
#    EDL Systems
#    16 Brierley Walk
#    Cambridge
#    CB4 3NH
#    UK
#
#    eddy@edlsystems.com

#
# SHSQL - The SQL Shell script utility(ies) instal; script
#
# Usage:  ./install.sh postgres mysql sqlite sqlite3
#
# leave out any parameter for which you do not want the engine build/installed for
#
#
# To determine the bin directory...
#

BINDIR=/usr/bin

#
# Now for the processing.....
#

export BINDIR ERROR

#
# First - lets CD to the directory...
#

cd `dirname $0`/src

if [ $# -eq 0 ]
then
	while true
	do
		cat <<!



ShellSQL Install Menu
~~~~~~~~~~~~~~~~~~~~~

Please select what you wish to install, or press ENTER to quit

A - Tools + Postgres 
B - Tools + MySQL
C - Tools + SQLite3
D - Tools + SQLite (version 2)
E - Tools + ODBC
F - Tools + FreeTDS (MS-SQL, Sybase)


Z - Tools only 

ENTER - Quit

!
		echo -e "Please enter letter...\c"
		read LETTER

		case a$LETTER in
		aa|aA)
			../install.sh postgres
			;;
		ab|aB)
			../install.sh mysql
			;;
		ac|aC)
			../install.sh sqlite3
			;;
		ad|aD)
			../install.sh sqlite
			;;
		ae|aE)
			../install.sh odbc
			;;
		af|aF)
			../install.sh freetds
			;;
		az|aZ)
			../install.sh tools
			;;
		a)
			exit 0;
			;;
		*)
			echo -e "Invalid selection: \c"
		esac

		echo -e "Please press ENTER to continue..\c"
		read CONT

	done
fi


ERROR=y

for ENGINE in $*
do
	case $ENGINE in
	postgres|POSTGRES)
		ERROR=n
		make shpostgres
		cp shpostgres $BINDIR
		;;
	mysql|MYSQL)
		ERROR=n
		make shmysql
		cp shmysql $BINDIR
		;;
	sqlite3|SQLITE3)
		ERROR=n
		make shsqlite3
		cp shsqlite3 $BINDIR
		;;
	sqlite|SQLITE)
		ERROR=n
		make shsqlite
		cp shsqlite $BINDIR
		;;
	odbc|ODBC)
		ERROR=n
		make shodbc
		cp shodbc $BINDIR
		;;
	freetds|FREETDS)
		ERROR=n
		make shfreetds
		cp shfreetds $BINDIR
		;;
	tools|TOOLS)
		ERROR=n
		;;
	*)
		break;
		;;
	esac
done

if [ $ERROR = n ]
then
	make tools
	cp shsqlstart shsqlend shsql shsqlline shsqlesc $BINDIR
else
	cat <<!

An error has occured.


You also need to run this as "root", or change the BINDIR variable
at the top of this file to a location where you have permissions.

Failing that, further investigation is required.... :-(

!
	exit 1
fi

exit 0

