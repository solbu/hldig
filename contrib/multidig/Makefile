#
# Makefile for the multidig system
#
# Copyright (c) 1998-2000 The ht://Dig Group
# Distributed under the terms of the GNU General Public License (GPL)
#   version 2 or later.
# for the ht://Dig search system http://www.htdig.org/
# and the multidig script system http://www.htdig.org/contrib/scripts/
#


#
# You probably want to change some or all of these.
#  BASH = location of bash or other Bourne-like shell with 'source' builtin
#  BASEDIR = directory of ht://Dig installation
# These should probably be OK.
#  BINDIR = directory of ht://Dig binaries. Also destination for these scripts.
#  CONFIG_DIR = directory of ht://Dig config files.
#  DB_BASE = base directory for ht://Dig / multidig databases
BASH=		/bin/bash
BASEDIR=	/opt/htdig
BINDIR=		$(BASEDIR)/bin
CONFIG_DIR=	$(BASEDIR)/conf
DB_BASE=	$(BASEDIR)/db


#
# You shouldn't need to change any of this...
#
SCRIPTS=	add-collect add-urls multidig \
		new-collect new-db gen-collect
CONF=		db.conf multidig.conf

all:

clean:
	rm -f *~

install:
	@echo "Installing scripts..."
	@for i in $(SCRIPTS); do \
		sed -e s%@BASH@%$(BASH)% \
		-e s%@CONFIG_DIR@%$(CONFIG_DIR)% $$i >$(BINDIR)/$$i; \
		chmod a+x $(BINDIR)/$$i; \
	echo $(BINDIR)/$$i; \
	done && test -z "$$fail"
	@echo
	@echo "Installing config files..."
	@echo
	@for i in $(CONF); do \
           sed -e s%@BASH@%$(BASH)% -e s%@BASEDIR@%$(BASEDIR)% \
		-e s%@BINDIR@%$(BINDIR)% -e s%@CONFIG_DIR@%$(CONFIG_DIR)% \
		-e s%@DB_BASE@%$(DB_BASE)% $$i >$(CONFIG_DIR)/$$i; \
           echo $(CONFIG_DIR)/$$i; \
        done && test -z "$$fail"
	@echo
	@echo "Done with installation."
	@echo
