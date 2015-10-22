::

  $ . $TESTDIR/setup

  $ function dump
  > {
  >   od -t c
  > }

nothing::

  $ cat /dev/null | shsqlline | dump
  0000000  \n
  0000001

  $ cat /dev/null | shsqlline --shell | dump
  0000000  \n
  0000001

  $ cat /dev/null | shsqlline --colon | dump
  0000000  \n
  0000001

  $ cat /dev/null | shsqlline --csv | dump
  0000000  \n
  0000001

  $ cat /dev/null | shsqlline --tab | dump
  0000000  \n
  0000001

empty tuple/line::

  $ echo | shsqlline | dump
  0000000  \n  \n
  0000002

  $ echo | shsqlline --shell | dump
  0000000  \n  \n
  0000002

  $ echo | shsqlline --colon | dump
  0000000  \n
  0000001

  $ echo | shsqlline --csv | dump
  0000000  \n
  0000001

  $ echo | shsqlline --tab | dump
  0000000  \n
  0000001

N-tuple::

  $ echo '"snafu fubar" omg "rofl lmao"' | shsqlline | dump
  0000000   "   s   n   a   f   u       f   u   b   a   r   "       o   m
  0000020   g       "   r   o   f   l       l   m   a   o   "  \n  \n
  0000037

  $ echo '"snafu fubar" omg "rofl lmao"' | shsqlline --shell | dump
  0000000   "   s   n   a   f   u       f   u   b   a   r   "       o   m
  0000020   g       "   r   o   f   l       l   m   a   o   "  \n  \n
  0000037

  $ echo 'snafu fubar:omg:rofl lmao' | shsqlline --colon | dump
  0000000   "   s   n   a   f   u       f   u   b   a   r   "       "   o
  0000020   m   g   "       "   r   o   f   l       l   m   a   o   "  \n
  0000040

  $ echo 'snafu fubar, omg, rofl lmao' | shsqlline --csv | dump
  0000000   "   s   n   a   f   u       f   u   b   a   r   "       "   o
  0000020   m   g   "       "   r   o   f   l       l   m   a   o   "  \n
  0000040

  $ printf 'snafu fubar\tomg\trofl lmao\n' | shsqlline --tab | dump
  0000000   "   s   n   a   f   u       f   u   b   a   r   "       "   o
  0000020   m   g   "       "   r   o   f   l       l   m   a   o   "  \n
  0000040

