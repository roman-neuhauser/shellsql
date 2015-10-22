CRAMCMD = cram

.PHONY: check
check: all
	env -i CRAM="$(CRAM)" PATH="$$PWD/tests:$$PWD/src:$(PATH)" $(CRAMCMD) -v tests


targets = all clean install \
          shfreetds install-shfreetds \
          shmysql install-shmysql \
          shodbc install-shodbc \
          shpostgres install-shpostgres \
          shsqlite install-shsqlite \
          shsqlite3 install-shsqlite3 \
          tools install-tools

.PHONY: $(targets)

$(targets):
	$(MAKE) -C src $@
