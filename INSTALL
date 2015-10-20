SHELLSQL - Installation Instructions
####################################

Prerequisites
=============

You need to have the development libraries of the SQL engines installed on the
computer prior to installing shsql.  These can be obtained the engines' web
sites, at time of writing these consist of.....

PostgreSQL
  http://www.postgresql.org
MySQL
  http://www.mysql.org
SQLite3
  http://www.sqlite.org
ODBC
  http://www.unixodbc.org
FreeTDS
  http://www.freetds.org (Read README.freetds before installing)


Installation
============

To install the database-agnostic tools plus the PostgreSQL driver::

  make tools shpostgres
  sudo make install-tools install-shpostgres

To install the database-agnostic tools plus the MySQL driver::

  make tools shmysql
  sudo make install-tools install-shmysql

And so on for the other drivers.  The tools are necessary for any driver.
Multiple drivers can be built and installed at once::

  make tools shmysql shpostgres
  sudo make install-tools install-shmysql install-shpostgres

Or you can build and/or install all of them at once::

  make all
  sudo make install

