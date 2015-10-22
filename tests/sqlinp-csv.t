setup ::

  $ . $TESTDIR/setup
  $ load_schema sqlite3

  $ dbconn=$(shsqlite3 sap)

  $ sqlite3 sap "SELECT * FROM supplier"

trivial format ::

  $ shsqlinp $dbconn --csv "INSERT INTO supplier (sid, sname, status, city) VALUES (?#, ?, ?#, ?)" <<EOF
  > 10,S1,0,C1
  > 20,S2,1,C2
  > 30,S3,2,C3
  > EOF

  $ sqlite3 sap "SELECT * FROM supplier ORDER BY sid"
  10|S1|0|C1
  20|S2|1|C2
  30|S3|2|C3

  $ sqlite3 sap "DELETE FROM supplier"

quotes ::

  $ shsqlinp $dbconn --csv "INSERT INTO supplier (sid, sname, status, city) VALUES (?#, ?, ?#, ?)" <<EOF
  > 10,"S1",0,"C1"
  > 20,"S2",1,"C2"
  > 30,"S3",2,"C3"
  > EOF

  $ sqlite3 sap "SELECT * FROM supplier ORDER BY sid"
  10|S1|0|C1
  20|S2|1|C2
  30|S3|2|C3

  $ sqlite3 sap "DELETE FROM supplier"

quotes with spaces ::

  $ shsqlinp $dbconn --csv "INSERT INTO supplier (sid, sname, status, city) VALUES (?#, ?, ?#, ?)" <<EOF
  > 10,"S 1",0,"C 1"
  > 20,"S 2",1,"C 2"
  > 30,"S 3",2,"C 3"
  > EOF

  $ sqlite3 sap "SELECT * FROM supplier ORDER BY sid"
  10|S 1|0|C 1
  20|S 2|1|C 2
  30|S 3|2|C 3

  $ sqlite3 sap "DELETE FROM supplier"

quotes with quotes ::

  $ shsqlinp $dbconn --csv "INSERT INTO supplier (sid, sname, status, city) VALUES (?#, ?, ?#, ?)" <<EOF
  > 10,"S""1",0,"C""""1"
  > 20,"S""2",1,"C""""2"
  > 30,"S""3",2,"C""""3"
  > EOF

  $ sqlite3 sap "SELECT * FROM supplier ORDER BY sid"
  10|S"1|0|C""1
  20|S"2|1|C""2
  30|S"3|2|C""3

  $ sqlite3 sap "DELETE FROM supplier"

quotes with newlines ::

  $ shsqlinp $dbconn --csv "INSERT INTO supplier (sid, sname, status, city) VALUES (?#, ?, ?#, ?)" <<EOF
  > 10,S1,0,C1
  > 20,"S
  > 2",1,C2
  > 30,S3,2,C3
  > EOF

  $ sqlite3 sap "SELECT * FROM supplier ORDER BY sid"
  10|S1|0|C1
  20|S
  2|1|C2
  30|S3|2|C3

  $ sqlite3 sap "DELETE FROM supplier"

disconnect ::

  $ shsqlend $dbconn
