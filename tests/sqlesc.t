setup ::

  $ . $TESTDIR/setup

tests ::

  $ SHSQL=mysql shsqlesc omg
  'omg' (no-eol)

  $ SHSQL=mysql shsqlesc '`wtf`'
  '`wtf`' (no-eol)

  $ SHSQL=mysql shsqlesc '\rofl'
  '\rofl' (no-eol)

  $ SHSQL=mysql shsqlesc omg '`wtf`' '\rofl'
  'omg `wtf` \rofl' (no-eol)


  $ SHSQL=sqlite3 shsqlesc omg
  'omg' (no-eol)

  $ SHSQL=sqlite3 shsqlesc '`wtf`'
  '`wtf`' (no-eol)

  $ SHSQL=sqlite3 shsqlesc '\rofl'
  '\rofl' (no-eol)

  $ SHSQL=sqlite3 shsqlesc omg '`wtf`' '\rofl'
  'omg `wtf` \rofl' (no-eol)


  $ SHSQL=postgres shsqlesc omg
  'omg' (no-eol)

  $ SHSQL=postgres shsqlesc '`wtf`'
  '`wtf`' (no-eol)

  $ SHSQL=postgres shsqlesc '\rofl'
  '\\rofl' (no-eol)

  $ SHSQL=postgres shsqlesc omg '`wtf`' '\rofl'
  'omg `wtf` \\rofl' (no-eol)
