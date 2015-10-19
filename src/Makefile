HFILES=message.h string.h shellsql.h strarr.h sarg.h

CC=gcc

CFLAGS=-g

default:
	echo -e "\n\n\n\n\nPlease edit and run install.sh script\n\n\n\n\n"

clean:
	rm -fr *.o shsql shsqlend shsqlline shsqlstart shsqlesc shpostgres shmysql shodbc shsqlinp shfreetds shsqlite3


tools: shsql shsqlend shsqlline shsqlstart shsqlesc shsqlinp


sarg.o: sarg.c $(HFILES)
	$(CC) $(CFLAGS) -c sarg.c -Wall

shsqlite3.o: shsqlite3.c $(HFILES)
	$(CC) $(CFLAGS) -c shsqlite3.c -Wall

shsqlite.o: shsqlite.c $(HFILES)
	$(CC) $(CFLAGS) -c shsqlite.c -Wall

shodbc.o: shodbc.c $(HFILES)
	$(CC) $(CFLAGS) -c shodbc.c -Wall

shpostgres.o: shpostgres.c $(HFILES)
	$(CC) $(CFLAGS) -c shpostgres.c -Wall

shmysql.o: shmysql.c $(HFILES)
	$(CC) $(CFLAGS) -c shmysql.c -I/usr/include/mysql -Wall

shsql.o: shsql.c $(HFILES)
	$(CC) $(CFLAGS) -c shsql.c -Wall

shsqlinp.o: shsqlinp.c $(HFILES)
	$(CC) $(CFLAGS) -c shsqlinp.c -Wall

shsqlend.o: shsqlend.c $(HFILES)
	$(CC) $(CFLAGS) -c shsqlend.c -Wall

shsqlstart.o: shsqlstart.c $(HFILES)
	$(CC) $(CFLAGS) -c shsqlstart.c -Wall

shsqlesc.o: shsqlesc.c $(HFILES)
	$(CC) $(CFLAGS) -c shsqlesc.c -Wall


# shsqlinit.o: shsqlinit.c $(HFILES)
	# $(CC) $(CFLAGS) -c shsqlinit.c -Wall

shsqlline.o: shsqlline.c $(HFILES)
	$(CC) $(CFLAGS) -c shsqlline.c -Wall

message.o: message.c $(HFILES)
	$(CC) $(CFLAGS) -c message.c -Wall

string.o: string.c $(HFILES)
	$(CC) $(CFLAGS) -c string.c -Wall

strarr.o: strarr.c $(HFILES)
	$(CC) $(CFLAGS) -c strarr.c -Wall

dolitem.o: dolitem.c $(HFILES)
	$(CC) $(CFLAGS) -c dolitem.c -Wall

traperr.o: traperr.c $(HFILES)
	$(CC) $(CFLAGS) -c traperr.c -Wall


shsqlite3: shsqlite3.o \
	 message.o \
	 string.o \
	 dolitem.o \

	$(CC) shsqlite3.o \
	 message.o \
	 string.o \
	 dolitem.o \
                 -lsqlite3 -Wall -o shsqlite3


shsqlite: shsqlite.o \
	 message.o \
	 string.o \
	 dolitem.o \

	$(CC) shsqlite.o \
	 message.o \
	 string.o \
	 dolitem.o \
                 -lsqlite -Wall -o shsqlite


shodbc: shodbc.o \
	 message.o \
	 string.o \
         dolitem.o \

	$(CC) shodbc.o \
	 message.o \
	 string.o \
         dolitem.o \
                 -lodbc -Wall -o shodbc


shpostgres: shpostgres.o \
	 message.o \
	 string.o \
         dolitem.o \

	$(CC) shpostgres.o \
	 message.o \
	 string.o \
         dolitem.o \
                 -lpq -Wall -o shpostgres

shmysql: shmysql.o \
	 message.o \
	 string.o \
         dolitem.o \
	sarg.o \

	$(CC) shmysql.o \
	 message.o \
	 string.o \
         dolitem.o \
	 sarg.o \
                 `mysql_config --libs` -Wall -o shmysql

shfreetds: shfreetds.o \
	 message.o \
	 string.o \
         dolitem.o \
	sarg.o \

	$(CC) shfreetds.o \
	 message.o \
	 string.o \
         dolitem.o \
	 sarg.o \
                 -lct -Wall -o shfreetds


shsql: shsql.o \
	 message.o \
	 string.o \
	 traperr.o \

	$(CC) shsql.o \
	 message.o \
	 string.o \
	 traperr.o \
                 -Wall -o shsql


shsqlinp: shsqlinp.o \
	 message.o \
	 string.o \
         strarr.o \
         traperr.o \

	$(CC) shsqlinp.o \
	 message.o \
	 string.o \
         strarr.o \
         traperr.o \
                 -Wall -o shsqlinp



shsqlend: shsqlend.o \
	 message.o \
	 string.o \

	$(CC) shsqlend.o \
	 message.o \
	 string.o \
                 -Wall -o shsqlend


# shsqlinit: shsqlinit.o \
#	 message.o \
#         string.o
#
#	$(CC) shsqlinit.o \
#	 message.o \
#        string.o \
#                 -Wall -o shsqlinit

shsqlline: shsqlline.o \
              string.o \

	$(CC) shsqlline.o \
              string.o \
                 -Wall -o shsqlline

shsqlstart: shsqlstart.o \

	$(CC) shsqlstart.o \
                 -Wall -o shsqlstart

shsqlesc: shsqlesc.o \
          string.o \

	$(CC) shsqlesc.o \
              string.o \
                 -Wall -o shsqlesc



