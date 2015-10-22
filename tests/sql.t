setup ::

  $ . $TESTDIR/setup
  $ load_schema sqlite3

connect ::

  $ dbconn=$(shsqlite3 sap)

SELECT an empty resultset ::

  $ shsql $dbconn "select * from supplier;"

INSERT some data ::

  $ shsql $dbconn "insert into supplier (sid, sname, status, city) values (10, 'S1', 0, 'C1')"
  $ shsql $dbconn "insert into supplier (sid, sname, status, city) values (20, 'S2', 1, 'C2')"
  $ shsql $dbconn "insert into supplier (sid, sname, status, city) values (30, 'S3', 2, 'C3')"

SELECT some data ::

  $ shsql $dbconn "select * from supplier;"
  "10" "S1" "0" "C1"
  "20" "S2" "1" "C2"
  "30" "S3" "2" "C3"

disconnect ::

  $ shsqlend $dbconn
