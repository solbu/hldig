
include $(top_srcdir)/Makefile.config

IMAGES=		button1.gif button2.gif button3.gif button4.gif button5.gif \
		button6.gif button7.gif button8.gif button9.gif buttonl.gif \
		buttonr.gif button10.gif htdig.gif star.gif star_blank.gif \
		button1.png button2.png button3.png button4.png button5.png \
                button6.png button7.png button8.png button9.png buttonl.png \
                buttonr.png button10.png htdig.png star.png star_blank.png
COMMONHTML=	header.html footer.html wrapper.html nomatch.html syntax.html \
		long.html short.html
COMMONDICT=	bad_words english.0 english.aff synonyms

EXTRA_DIST = $(IMAGES) $(COMMONHTML) $(COMMONDICT) rundig htdig.conf search.html

install-data-local:	all
	$(mkinstalldirs) $(DESTDIR)$(DATABASE_DIR)
	@echo "Installing default configuration files..."
	$(mkinstalldirs) $(DESTDIR)$(CONFIG_DIR)
	@if [ ! -f $(INSTALL_ROOT)$(CONFIG_DIR)/htdig.conf ]; then sed -e s%@DATABASE_DIR@%$(DATABASE_DIR)% -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% $(top_srcdir)/installdir/htdig.conf >$(INSTALL_ROOT)$(CONFIG_DIR)/htdig.conf; echo $(INSTALL_ROOT)$(CONFIG_DIR)/htdig.conf;fi
	$(mkinstalldirs) $(DESTDIR)$(SEARCH_DIR)
	@if [ ! -f $(INSTALL_ROOT)$(SEARCH_DIR)/$(SEARCH_FORM) ]; then sed -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% $(top_srcdir)/installdir/search.html >$(INSTALL_ROOT)$(SEARCH_DIR)/$(SEARCH_FORM); echo $(INSTALL_ROOT)$(SEARCH_DIR)/$(SEARCH_FORM);fi
	$(mkinstalldirs) $(DESTDIR)$(COMMON_DIR)
	@for i in $(COMMONHTML); do \
		if [ ! -f $(INSTALL_ROOT)$(COMMON_DIR)/$$i ]; then sed -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% $(top_srcdir)/installdir/$$i >$(INSTALL_ROOT)$(COMMON_DIR)/$$i; echo $(INSTALL_ROOT)$(COMMON_DIR)/$$i;fi; \
	done && test -z "$$fail"
	@for i in $(COMMONDICT); do \
		if [ ! -f $(INSTALL_ROOT)$(COMMON_DIR)/$$i ]; then $(INSTALL_DATA) $(top_srcdir)/installdir/$$i $(INSTALL_ROOT)$(COMMON_DIR)/$$i; echo $(INSTALL_ROOT)$(COMMON_DIR)/$$i; fi; \
	done && test -z "$$fail"
	@echo "Installing images..."
	$(mkinstalldirs) $(DESTDIR)$(IMAGE_DIR)
	@for i in $(IMAGES); do \
		if [ ! -f $(INSTALL_ROOT)$(IMAGE_DIR)/$$i ]; then $(INSTALL_DATA) $(top_srcdir)/installdir/$$i $(INSTALL_ROOT)$(IMAGE_DIR)/$$i; echo $(INSTALL_ROOT)$(IMAGE_DIR)/$$i;fi; \
	done && test -z "$$fail"
	@echo "Creating rundig script..."
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@if [ ! -f $(INSTALL_ROOT)$(bindir)/rundig ]; then \
		sed -e s%@BIN_DIR@%$(bindir)% -e s%@COMMON_DIR@%$(COMMON_DIR)% -e s%@DATABASE_DIR@%$(DATABASE_DIR)% $(top_srcdir)/installdir/rundig >$(INSTALL_ROOT)$(bindir)/rundig; \
		chmod 755 $(INSTALL_ROOT)$(bindir)/rundig; \
	fi
