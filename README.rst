.. vim: ft=rst sts=2 sw=2 tw=77

.. :Author: Roman Neuhauser
.. :Contact: neuhauser+shellsql@sigpipe.cz
.. :Copyright: This document is in the public domain.

.. default-role:: emphasis

#############################################################################
                                  ShellSQL
#############################################################################
=============================================================================
             Commandline API for access to SQL database servers
=============================================================================

.. contents::

Introduction
############

ShellSQL is an SQL database API for the UNIX command line, with drivers for
several database servers:

* MariaDB, MySQL, Percona (libmysqlclient)
* PostgreSQL (libpq)
* SQL Server (FreeTDS)
* SQLite (libsqlite, libsqlite3)
* ODBC (unixODBC)

ShellSQL aims to provide shell scripts with an API analogous to that of
other, more "grown up" languages.

In contrast to common command line clients like *mysql(1)* or *psql(1)*, in
which the lifetime of a database connection is bound by the lifetime of the
database "prompt", ShellSQL allows a database connection to span multiple
shell commands.

A database connection is created with the `shsqlstart` utility.
`shsqlstart` writes a handle for the database connection to `stdout`,
forks, and the parent exits.  Other ShellSQL utilities like `shsql` then
use this handle to operate on the corresponding connection.

.. admonition:: Example

  ::

    export SHSQL=postgres
    local dbh tpl

    dbh=$(shsqlstart dbname=music)

    shsql $dbh 'SELECT name FROM artist ORDER BY name' | {
      while tpl=$(shsqlline); do
        eval set -- $tpl
      done
    }

    shsqlend $dbconn


License
#######

ShellSQL is published under the terms of the GPLv2.  See the `COPYING`_ file.

.. _COPYING: COPYING


Installation
############

Installation instructions are laid out in the `INSTALL.rst`_ file.

.. _`INSTALL.rst`: INSTALL.rst


Connection Commands
###################

These sets of commands establish a connection to the database.  They all
operate in a similar way in so far as much they initiate a background
SQL client and output a handle that is used in subsequent ShellSQL commands
to use it. For example::

  HANDLE=$(shpostgres dbname=test)

On success it returns 0.
On an error they output nothing and effect a return code
of 1.   An error message may appear on the standard error output.


`shsqlstart`
============

This is a generic connection routine.  This looks at the environment
variable `SHSQL`, and expects it to be "postgres", "sqlite3" or one of the
other engines, then starts "shpostgres" or 'shsqlite3" respectively with the
appropriate parameters.  If it is not recognizeable it starts using
"shpostgres".

An example of this working::

  SHPROG=postgres
  export SHPROG

  HANDLE=$(shsqlstart dbname=test)

and so on.

shsqlstart is a small wrapper program.  It expects the appropriate connection
program to be in the same directory as it ("shsqlstart").  This is the
reccomended way of starting "shsql" connection as the `SHSQL` environment
variables can be used in other utilities.


`shpostgres`
============

This initiates a PostgreSQL connection.  The syntax is::

  shpostgres connectarg [connectarg] ...

The connectarg(s) are concatonated together separated by spaces.  So for
example, the following is perfectlly legal::

  shpostgres dbname=test user=myuser

which does the same thing as::

  shpostgres "dbname=test user=myuser"

Either way, the arguments of the engne take the form of name=value where name
is one of the following. The are the same as the ones used in PQconnectdb()
in PostgreSQL's libpq library. Most have sensible defaults.

:host:
  The host name of the server
:hostaddr:
  The host IP address of the server
:port:
  The TCP port number to connect to on the server
:dbname:
  The database name
:user:
  The user name for the connection
:password:
  The password for the user
:connect_tomeout:
  The connection timeout in seconds, 0 is indefinate
:options:
  Command line options to be sent to the server.
  See Postgres documentation for more details.
:sslmode:
  the SSL mode for the connection, This is either
  `disable`, `allow`, `prefer` or `require`.
:service:
  Name of service that holds extra parameters.
  See Postgres documentation for more details.

If you are using the password assignment in the connection then you should
use the "password=mysecret" string as a separate parameter.  The reason for
this is that "shpostgres" will detect it and blank it out in the process
table so someone else doing a "ps" cannot see it.



`shmysql`
=========

This initiates a MySQL connection.  The syntax is::

  shmysql connectarg [connectarg] ...

The connectarg(s) are concatonated together separated by spaces.  So for
example, the following is perfectlly legal::

  shmysql dbname=test user=myuser

which does the same thing as::

  shmysql "dbname=test user=myuser"

Either way, the arguments of the engne take the form of name=value where name
is one of the following.  Most have sensible defaults.

:host:
  The host name or IP address of the server
:port:
  The TCP port number to connect to on the server
:dbname:
  The database name
:user:
  The user name for the connection
:password:
  The password for the user
:socket:
  The name of the UNIX socket if applicable
:flag:
  Usually not defined or zero, but can be used in special
  circumstances, see the documentation in MySQL for further
  information.

If you are using the password assignment in the connection then you should
use the "password=mysecret" string as a separate parameter.  The reason for
this is that "shpostgres" will detect it and blank it out in the process
table so someone else doing a "ps" cannot see it.



`shsqlite3`
===========

This initiates a SQLITE3 connection.  The syntax is::

  shsqlite3 databasefilename

Where databasefilename is the name of the SQLITE3 database to open.


`shsqlite`
==========

This initiates a SQLITE (version 2) connection.  The syntax is::

  shsqlite3 databasefilename

Where databasefilename is the name of the SQLITE database to open.


`shodbc`
========

This initiates an ODBC connection.  The syntax is::

  shodbc user password connectionstriing

where the user and password (which need to be there own parameters, NOT
incorporated in the connection string) are just that, and the connection
string is a (number of) parameters that constitute the connection string.

There is another parameter that can be passed using the environment
variable ODBC_TIMEOUT, which contains the login timeout in seconds.
This defaults to 20 if not defined.


`shfreetds`
===========

This initiates a FreeTDS connection, which can be used to connect to MS-SQL
and SyBase (amongst others).  The syntax is::

  shfreetds connectarg [connectarg] ...

The connectarg(s) are concatonated together separated by spaces.  So for
example, the following is perfectlly legal::

  shfreetds server=test dbname=test user=myuser

which does the same thing as::

  shfreetds "server=test dbname=test user=myuser"

Either way, the arguments of the engne take the form of name=value where name
is one of the following.  Most have sensible defaults.

:server:
  The server name (as in the freetds.conf file) - required.
:port:
  The TCP port number to connect to on the server
:dbname:
  The database name
:user:
  The user name for the connection
:password:
  The password for the user
:appname:
  The application name used for the connection
:host:
  The host name or IP address of the server


If you are using the password assignment in the connection then you should
use the "password=mysecret" string as a separate parameter.  The reason for
this is that "shfreetds" will detect it and blank it out in the process
table so someone else doing a "ps" cannot see it.

To use this it is important that you have read the FreeTDS implementation
documentation, especially in creating the "freetds.conf" file correctly.
Also, at time of writing, freetds's "ct" library interface has a bug in it and
should be corrected and recompiled.  This is documented in the "README.freetds"
file.


Execution commands
##################

`shsql`
=======

The command shsql performs the actual SQL queries.  As it's first
parameter it takes the handle obtained from the connection routine
above.  Subsequent parameters represent the query itself.  So an
example here is::

  shsql $HANDLE "insert into a (b, c) values ('x', 'y')"

Alternatively the SQL parameter can be split.  shsql concatonates
them together separating them with a space::

  shsql $HANDLE "insert into a (b, c)" \
          "values ('x', 'y')"

Should the query generate rows, then each row is represented by
a value inclosed by double quotes, each field on the row being
separated with a space, and each row separated by a new line character.

Therefore, the following query::

  shsql $HANDLE "select keyfield, datafield from mydata"

could print on the standard output something like::

  "FRED" "FRED BLOGS"
  "DAVID" "DAVID COPERFIELD"
  "JOAN" "JOAN BLOGS"

Should one of the fields contain a double quote character then that
is replaced by 2 double quote characters together.


However - a means to alter the output is to place an optional format parameter
after the handle, this is one of the following

  --csv     Comma Separated Variable output
  --colon   Colon(: character) delimeted output - not quoted
  --pipe    Pipe (| character)  delimited output - not quoted
  --tab     Tab delimited output - not quoted
  --shell   Shell (the default, desribed above) output

For example::

  shsql $HANDLE --csv "select keyfield, datafield from mydata"

would produce::

  "FRED","FRED BLOGS"
  "DAVID","DAVID COPERFIELD"
  "JOAN","JOAN BLOGS"

or::

  shsql $HANDLE --pipe "select keyfield, datafield from mydata"

would produce::

  FRED|FRED BLOGS
  DAVID|DAVID COPERFIELD
  JOAN|JOAN BLOGS


If no parameters (except for HANDLE) are supplied then "shsql" takes
the standard input as the SQL command::

  echo "insert into a (b, c) values ('x', 'y')" | shsql $HANDLE


.. admonition:: Caveats

  At time of writing no more than one "shsql" command can be actime on
  the same handle (or connection) at any one time.  Should transaction
  processing be required you should save the output of the query to a file::

    shsql $HANDLE "select keyfield, datafield from mydata" > tempfile

    # Now for the transaction processing....

    cat tempfile (
      while ROW=$(shsqlline)
      do
        eval set $ROW

        shsql $HANDLE "update stats" \
                "set totallen = totallen + length('$2')"\
                "where initialkey = substr('$1', 1, 1)"
      done
    )

  or something like that.


  Another caveat is that shsql does not really handle binary fields.  Should
  this be required then you should escape them in the SQL itself.


`shsqlinp`
==========

This command would primarily be used to import data into a table, though it
can be used for other updates as well.  Primarily what it does is execute
(the same) SQL statement for every line of standard input it receives
performing a rudimentary parameter substitution.  An example of thos could
be::


  shsqlinp $HANDLE "insert into mydata (keyfield, datafield)" \
                   "values (?, ?)" \
  << _EOF
  "JAMES" "JAMES BOND 007"
  "MARY" "MARY QUEEN OF SCOTTS"
  _EOF

or even::

  shsqlinp $HANDLE "insert into xxx (yyy, zzz) values(?, ?)" < file.txt

Also, perhaps an output from a shsql from another database::

  shsql $HANDLE_A "select a, b from cc" | \
  shsqlinp  $HANDLE_B "insert into cc(a, b) values(?, ?)"

However - please note that you cannot use this method to copy data from one
table to another, or to perform other updates,  in the same database connection.
The "client back end" can only execute one statement at a time, so in order to
use ShellSQL to update a table based on another you need to go through
a temporary file::

  shsql $HANDLE "select a, b from cc"  > temp.txt
  shsqlinp  $HANDLE "insert into cc(a, b) values(?, ?)" < temp.txt

The above will work for string, or varchar, type datatypes.  If a number is to
be included then either casting needs to be done in the SQL command itself, or
you can append a "#" to the "?"parameter.  To specify a string you can apend a
"@" character, however, this is not neccessary as this is the default::

  shsqlinp $HANDLE "insert into aa (numf, charf) values (?#, ?@)" << _EOF
  "123" "ORDER ONE-TWO-THREE"
  "-2" "MINUS TWO"
  _EOF

This need not be just used for importing, but can be used for updating, deleting
or other DML type operations::

  shsqlinp $HANDLE "update atable set bfield = ? where ckey = ?" << _EOF
  "NEW DATA FOR A" "KEYA"
  "NEW DATA FOR B" "KEYB"
  _EOF

The format of the input file can be controlled by an optional parameter after the
HANDLE.  These are the same for those in shsql:

  --csv     Comma Separated Variable output
  --colon   Colon(: character) delimeted output - not quoted
  --pipe    Pipe (| character)  delimited output - not quoted
  --tab     Tab delimited output - not quoted
  --shell   Shell (the default, desribed above) output


`shsqlend`
==========

This simply terminates the connection and background process::

  shsqlend $HANDLE

It is important that the handle/connection is not accessed after this
is called.

It is important to call this however, should you do not then processes and
message queues will remain running in the background. Should this happen then
these processes can be killed using the LINUX/UNIX kill, or the LINUX killall
command. Though if this is done then the -9 option should not be used.


Sundry Utilities
################

Some utilities accompany the suite to assist with the shell scripts, these
do not connect to the SQL client background process in the same way as the
above but they are designed to work with the suite.

`shsqlline`
===========

First the technological description - this is complicated so please feel free
to ignore it and go straight to the example below...

This takes a rowset as produced by a select query using "shsql" as a
standard input, and prints out the first row of this returning 0.  If
the end of file is reached then it fails by returning 1.

Example::

  shsql $HANDLE "select * from mytable" > tempfile
  cat tempfile | (
    while ROW=$(shsqlline)
    do
      eval set $ROW
      ....

    done
  )
  rm tempfile

This can be shoertened to::

  shsql $HANDLE "select * from mytable"  | (
    while ROW=$(shsqlline)
    do
      eval set $ROW
      ....

    done
  )

To go through it line by line::

  shsql $HANDLE "select * from mytable"  | (

This runs sgsql then pipes the rowset (standartd output) to a
sub-shell which::

    while ROW=$(shsqlline)
    do

goes into a loop that reads the first then next line of the rowset
and placing it into a variable called ROW, the loop then::

    eval set $ROW

places the first field in $1, second in $2 and so on.  The "eval" is needed to
properly process the double quotes and spaces in parameters.


If you have not fully uderstood this it is unimportant as long as you
know how to enumerate the row set.

Please not too, that if you wish to perform other transactions on the same
$HANDLE in the loop you need to use the "tempfile" method.


`shsqlesc`
==========

This takes as it's parameter(s) and prints on the standard output a string
that has the quotes etc escaped suitable for string parameters in SQL queries.
Please note it places the preceding and postceding 's on the string too.
Bear in mind that different SQL engines use different escape routines
so this uses the `SHSQL` parameter to control precisely how it
behaves (defaulting to postgres).

Example::

  SQLPAR=$(shsqlesc "Bobby's Girl")

would place the value::

  'Bobb''s Girl'

including all the quotes into SQLPAR.

The behaviour of this command is effected by the `SHSQL` environment variable.
Different engines have slightly different escape rules, and shsqlesc will
alter the way it behaves accordingly, so `SHSQL` should be set accordingly (as
in the shsqlstart program). If it is not it defaults to postgres.


Thank You
#########

The original author of ShellSQL up to 0.7.7 (released in February 2007)
is Edward Macnaghten.
