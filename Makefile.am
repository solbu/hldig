# Main Makefile for ht://Dig
# Copyright (c) 1995-1999 The ht://Dig Group

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
   
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
   
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

include $(top_srcdir)/Makefile.config

SUBDIRS=        db htlib htcommon htfuzzy htdig \
		htsearch htmerge htnotify installdir include htdoc .

EXTRA_DIST = .version Makefile.config ChangeLog.0

install-data-hook:
	@echo ""
	@echo "Installation done."
	@echo ""
	@echo "Before you can start searching, you will need to create a"
	@echo "search database.  A sample script to do this has been"
	@echo "installed as " $(INSTALL_ROOT)$(bindir)/rundig
