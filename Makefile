-include Makefile.config

TARGETS = src

DIR := $(shell basename `pwd`)

MAKECMDGOALS ?= linux

linux pi:
	for TRG in $(TARGETS) ; do $(MAKE) -C $$TRG $(MAKECMDGOALS) ; done

clean:
	for TRG in $(TARGETS) ; do $(MAKE) -C $$TRG $(MAKECMDGOALS) ; done

force:

doc: force
	cd doc && $(MAKE) doc

tar:
	$(MAKE) clean
	cd .. ; tar cfvz $(PROGRAM)-$(VERSION).tar.gz $(DIR)
