setup ::

  $ . $TESTDIR/setup
  $ setup sqlite3

usage ::

  $ shsqlite3
  Usage: shsqlite3 dbfilename
  [1]

connect ::

  $ dbconn=$(shsqlite3 sap)

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
