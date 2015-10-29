srcdir = .

RST2HTMLCMD = rst2html5
html = INSTALL.html README.html

.PHONY: html
html: $(html)

$(html): %.html: $(srcdir)/%.rst
	$(RST2HTMLCMD) $(RST2HTML) $< $@


.PHONY: clean
clean::
	$(RM) $(html)


CRAMCMD = cram

.PHONY: check
check: all
	env -i CRAM="$(CRAM)" PATH="$$PWD/$(srcdir)/tests:$$PWD/$(srcdir)/src:$(PATH)" $(CRAMCMD) -v $(srcdir)/tests


targets = all clean install \
          shfreetds install-shfreetds \
          shmysql install-shmysql \
          shodbc install-shodbc \
          shpostgres install-shpostgres \
          shsqlite install-shsqlite \
          shsqlite3 install-shsqlite3 \
          tools install-tools

.PHONY: $(targets)

$(targets)::
	$(MAKE) -C $(srcdir)/src $@
