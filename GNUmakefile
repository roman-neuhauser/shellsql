srcdir = .

BINDIR = /usr/bin
CC = gcc
CFLAGS = -g -Wall
CRAMCMD = cram
RST2HTMLCMD = rst2html5

html = INSTALL.html README.html

.PHONY: html
html: $(html)

$(html): %.html: $(srcdir)/%.rst
	$(RST2HTMLCMD) $(RST2HTML) $< $@


.PHONY: check
check: all
	env -i CRAM="$(CRAM)" PATH="$$PWD/$(srcdir)/tests:$$PWD/$(srcdir):$(PATH)" $(CRAMCMD) -v $(srcdir)/tests


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
	rm -f $(html) $(objects) $(tools) $(drivers)


.PHONY: tools
tools: $(tools)


$(drivers): %: %.o message.o string.o dolitem.o
	$(CC) -o$@ $^ $(lib_$@)

$(tools): %: %.o message.o strarr.o string.o traperr.o
	$(CC) -o$@ $^

$(objects): $(addprefix $(srcdir)/src/,message.h string.h shellsql.h strarr.h sarg.h)

$(objects): %.o: $(srcdir)/src/%.c
	$(CC) $(CFLAGS) $(inc_$(basename $@)) -c $<


shsqlite3: -lsqlite3
shsqlite: -lsqlite
shodbc: -lodbc
shpostgres: -lpq
shmysql: sarg.o
shfreetds: sarg.o -lct
