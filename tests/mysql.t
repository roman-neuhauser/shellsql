setup ::

  $ . $TESTDIR/setup
  $ setup mysql

usage ::

  $ shmysql
  Usage: shmysql connecstring
  [1]

connect ::

  $ dbconn=$(shmysql socket=$PWD/mysql.sock dbname=sap user=root)

some queries ::

  $ shsql $dbconn "select 'hello world!'"
  "hello world!"

  $ shsql $dbconn "select 'hello', 'world!'"
  "hello" "world!"

  $ shsql $dbconn "select * from supplier;"
  "10" "S1" "0" "C1"
  "20" "S2" "1" "C2"
  "30" "S3" "2" "C3"

  $ shsql $dbconn "select * from part;"

disconnect ::

  $ shsqlend $dbconn
