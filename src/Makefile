BINDIR = /usr/bin

CC=gcc

CFLAGS = -g -Wall

install-files = install -m 0755 -Dt $(DESTDIR)$(1)/. $(2)

inc_shmysql = $(shell mysql_config --include)
lib_shmysql = $(shell mysql_config --libs)


programs = $(drivers) $(tools)

drivers = shfreetds shmysql shodbc shpostgres shsqlite shsqlite3
tools = shsql shsqlend shsqlline shsqlstart shsqlesc shsqlinp

install_drivers = $(patsubst %,install-%,$(drivers))

objects = $(addsuffix .o,$(drivers) $(tools) $(utils))
utils = dolitem message sarg strarr string traperr


.PHONY: all
all: $(drivers) $(tools)

.PHONY: install
install: all
	$(call install-files,$(BINDIR),$(programs))

.PHONY: install-tools
install-tools: $(tools)
	$(call install-files,$(BINDIR),$(tools))

.PHONY: $(install_drivers)
$(install_drivers): install-%: %
	$(call install-files,$(BINDIR),$^)

.PHONY: clean
clean:
	rm -f $(objects) $(tools) $(drivers)


.PHONY: tools
tools: $(tools)


$(drivers): %: %.o message.o string.o dolitem.o
	$(CC) -o$@ $^ $(lib_$@)

$(tools): %: %.o message.o strarr.o string.o traperr.o
	$(CC) -o$@ $^

srcdir = .

$(objects): $(addprefix $(srcdir)/,message.h string.h shellsql.h strarr.h sarg.h)

$(objects): %.o: $(srcdir)/%.c
	$(CC) $(CFLAGS) $(inc_$(basename $@)) -c $<


shsqlite3: -lsqlite3
shsqlite: -lsqlite
shodbc: -lodbc
shpostgres: -lpq
shmysql: sarg.o
shfreetds: sarg.o -lct
