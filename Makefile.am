# Main Makefile for hl://Dig
# Copyright (c) 2017 The hl://Dig Group <https://solbu.github.io/hldig/>
# For copyright details, see the file COPYING in your distribution
# or the GNU Library General Public License version 2 or later
# <http://www.gnu.org/copyleft/lgpl.html>

include $(top_srcdir)/Makefile.config

if TESTS
TESTDIR = test
endif

# process the Makefile.am files in these directories. the po directory
# is a little # different because of a rule near the end of configure.ac

SUBDIRS = db hllib hlword htcommon htnet hlfuzzy hldb hldig hlsearch	\
hltools installdir include $(TESTDIR) po

# files to be installed to destination docdir when 'make install' is run
# packextrasdir=@docdir@

EXTRA_DIST =		\
VERSION			\
COPYING			\
INSTALL			\
README.md		\
AUTHORS.md		\
TESTING.md		\
CONTRIBUTING.md		\
TRANSLATING.md 		\
ChangeLog 		\
STATUS			\
CODE_OF_CONDUCT.md	\
ABOUT-NLS

# packextras_DATA=$(EXTRA_DIST)

# This only install the html and gif files, but not the assets (css and images)
# in the docs directory
# packextraswebsitedir=@docdir@/docs

# WEBSITE = \
#	docs/*.html \
#	docs/*.gif
# packextraswebsite_DATA=$(WEBSITE)

ACLOCAL_AMFLAGS = -I m4

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
	@echo ""
	@echo "To uninstall hldig, use 'make uninstall' (must be run before 'make distclean')"
	@echo "To remove build files, use 'make distclean'"


distdir = $(PACKAGE)-$(VERSION)
