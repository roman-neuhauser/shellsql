setup ::

  $ . $TESTDIR/setup
  $ load_schema sqlite3

  $ dbconn=$(shsqlite3 sap)

  $ shsql $dbconn "SELECT * FROM supplier;"

trivial format ::

  $ shsqlinp $dbconn --pipe "INSERT INTO supplier (sid, sname, status, city) VALUES (?#, ?, ?#, ?)" <<EOF
  > 10|S1|0|C1
  > 20|S2|1|C2
  > 30|S3|2|C3
  > EOF

  $ sqlite3 sap "SELECT * FROM supplier ORDER BY sid"
  10|S1|0|C1
  20|S2|1|C2
  30|S3|2|C3

  $ sqlite3 sap "DELETE FROM supplier"

disconnect ::

  $ shsqlend $dbconn
