setup ::

  $ . $TESTDIR/setup
  $ load_schema sqlite3

connect ::

  $ dbconn=$(shsqlite3 sap)

some queries ::

  $ shsql $dbconn "SELECT * FROM supplier;"

  $ shsqlinp $dbconn --shell "INSERT INTO supplier (sid, sname, status, city) VALUES (?#, ?, ?#, ?)" <<EOF
  > 10 "S1" 0 "C1"
  > 20 "S2" 1 "C2"
  > 30 "S3" 2 "C3"
  > EOF

disconnect ::

  $ shsqlend $dbconn

verify correct operation ::

  $ sqlite3 sap "SELECT * FROM supplier ORDER BY sid"
  10|S1|0|C1
  20|S2|1|C2
  30|S3|2|C3
