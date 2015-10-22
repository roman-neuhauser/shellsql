setup ::

  $ . $TESTDIR/setup
  $ PATH=$PWD:$PATH

  $ cat > shsnafu <<\EOF
  > echo this is $0 >&2
  > exit 42
  > EOF

  $ cat > shok <<\EOF
  > echo this is $0
  > exit 0
  > EOF
  $ chmod u+x shok

exec(2)s "sh${SHSQL}" ::

  $ SHSQL=fubar shsqlstart
  shsqlstart: Failed to start ./shfubar
  [1]

  $ SHSQL=snafu shsqlstart
  shsqlstart: Failed to start ./shsnafu
  [1]

relays errors and output ::

  $ chmod u+x shsnafu
  $ SHSQL=snafu shsqlstart
  this is ./shsnafu
  [42]

  $ SHSQL=ok shsqlstart
  this is ./shok

`SHSQL` defaults to "postgres" ::

  $ unset SHSQL
  $ cp shsnafu shpostgres

  $ shsqlstart
  this is ./shpostgres
  [42]
