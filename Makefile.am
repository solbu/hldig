# Main Makefile for ht://Dig
# Copyright (c) 1995-2004 The ht://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU Library General Public License version 2 or later
# <http://www.gnu.org/copyleft/lgpl.html>

include $(top_srcdir)/Makefile.config

if TESTS
TESTDIR = test
endif

SUBDIRS = db htlib htword htcommon htnet htfuzzy htdb htdig htsearch httools installdir include htdoc $(TESTDIR)

EXTRA_DIST = .version Makefile.config ChangeLog.0 contrib

dist-hook:
	find $(distdir) -depth -name CVS -print | xargs rm -fr 

#
# If --enable-tests is not specified, should remove
# the test/Makefile anyway
#
distclean-local:
	if test "X$(TESTDIR)" = "Xtest" ; \
	then \
		rm -f test/Makefile test/test_functions ; \
	fi

install-data-hook:
	@echo ""
	@echo "Installation done."
	@echo ""
	@echo "Before you can start searching, you will need to create a"
	@echo "search database.  A sample script to do this has been"
	@echo "installed as " $(DESTDIR)$(bindir)/rundig
