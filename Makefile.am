# Main Makefile for ht://Dig
# Copyright (c) 1995-1999 The ht://Dig Group

include $(top_srcdir)/Makefile.config

SUBDIRS=        @HTDIGDIRS@

EXTRA_DIST = .version Makefile.config ChangeLog.0

#
# If --enable-tests is not specified, should remove
# the test/Makefile anyway
#
distclean-local:
	if echo "@HTDIGDIRS@" | grep 'test ' > /dev/null ; \
	then : ; \
	else \
		rm -f test/Makefile ; \
	fi

install-data-hook:
	@echo ""
	@echo "Installation done."
	@echo ""
	@echo "Before you can start searching, you will need to create a"
	@echo "search database.  A sample script to do this has been"
	@echo "installed as " $(INSTALL_ROOT)$(bindir)/rundig
