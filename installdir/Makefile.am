
include $(top_srcdir)/Makefile.config

man_MANS = htdig-pdfparser.1 htdig.1 htdump.1 htfuzzy.1 htload.1 htmerge.1 \
		htnotify.1 htpurge.1 htsearch.1 htstat.1 rundig.1 htdigconfig.8

IMAGES=		button1.gif button2.gif button3.gif button4.gif button5.gif \
		button6.gif button7.gif button8.gif button9.gif buttonl.gif \
		buttonr.gif button10.gif htdig.gif star.gif star_blank.gif \
		button1.png button2.png button3.png button4.png button5.png \
                button6.png button7.png button8.png button9.png buttonl.png \
                buttonr.png button10.png htdig.png star.png star_blank.png
COMMONHTML=	header.html footer.html wrapper.html nomatch.html syntax.html \
		long.html short.html
COMMONDICT=	bad_words english.0 english.aff synonyms

EXTRA_DIST = $(IMAGES) $(COMMONHTML) $(COMMONDICT) rundig htdig.conf mime.types \
		search.html HtFileType HtFileType-magic.mime cookies.txt

install-data-local:	all
	$(mkinstalldirs) $(DESTDIR)$(DATABASE_DIR)
	@echo "Installing default configuration files..."
	$(mkinstalldirs) $(DESTDIR)$(CONFIG_DIR)
	@if [ ! -f $(DESTDIR)$(CONFIG_DIR)/htdig.conf ]; then sed -e s%@\DATABASE_DIR@%$(DATABASE_DIR)% -e s%@\IMAGEDIR@%$(IMAGE_URL_PREFIX)% $(top_srcdir)/installdir/htdig.conf >$(DESTDIR)$(CONFIG_DIR)/htdig.conf; echo $(DESTDIR)$(CONFIG_DIR)/htdig.conf;fi
	@if [ ! -f $(DESTDIR)$(CONFIG_DIR)/mime.types ]; then cp $(top_srcdir)/installdir/mime.types $(DESTDIR)$(CONFIG_DIR)/mime.types; echo $(DESTDIR)$(CONFIG_DIR)/mime.types;fi
	@if [ ! -f $(DESTDIR)$(CONFIG_DIR)/HtFileType-magic.mime ]; then cp $(top_srcdir)/installdir/HtFileType-magic.mime $(DESTDIR)$(CONFIG_DIR)/HtFileType-magic.mime; echo $(DESTDIR)$(CONFIG_DIR)/HtFileType-magic.mime;fi
	$(mkinstalldirs) $(DESTDIR)$(SEARCH_DIR)
	@if [ ! -f $(DESTDIR)$(SEARCH_DIR)/$(SEARCH_FORM) ]; then sed -e s%@\IMAGEDIR@%$(IMAGE_URL_PREFIX)% $(top_srcdir)/installdir/search.html >$(DESTDIR)$(SEARCH_DIR)/$(SEARCH_FORM); echo $(DESTDIR)$(SEARCH_DIR)/$(SEARCH_FORM);fi
	$(mkinstalldirs) $(DESTDIR)$(COMMON_DIR)
	@for i in $(COMMONHTML); do \
		if [ ! -f $(DESTDIR)$(COMMON_DIR)/$$i ]; then sed -e s%@\IMAGEDIR@%$(IMAGE_URL_PREFIX)% $(top_srcdir)/installdir/$$i >$(DESTDIR)$(COMMON_DIR)/$$i; echo $(DESTDIR)$(COMMON_DIR)/$$i;fi; \
	done && test -z "$$fail" 
	@for i in $(COMMONDICT); do \
		if [ ! -f $(DESTDIR)$(COMMON_DIR)/$$i ]; then $(INSTALL_DATA) $(top_srcdir)/installdir/$$i $(DESTDIR)$(COMMON_DIR)/$$i; echo $(DESTDIR)$(COMMON_DIR)/$$i; fi; \
	done && test -z "$$fail"
	@if [ ! -f $(DESTDIR)$(CONFIG_DIR)/cookies.txt ]; then cp $(top_srcdir)/installdir/cookies.txt $(DESTDIR)$(CONFIG_DIR)/cookies.txt; echo $(DESTDIR)$(CONFIG_DIR)/cookies.txt;fi
	@echo "Installing images..."
	$(mkinstalldirs) $(DESTDIR)$(IMAGE_DIR)
	@for i in $(IMAGES); do \
		if [ ! -f $(DESTDIR)$(IMAGE_DIR)/$$i ]; then $(INSTALL_DATA) $(top_srcdir)/installdir/$$i $(DESTDIR)$(IMAGE_DIR)/$$i; echo $(DESTDIR)$(IMAGE_DIR)/$$i;fi; \
	done && test -z "$$fail"
	@echo "Creating rundig script..."
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@if [ ! -f $(DESTDIR)$(bindir)/rundig ]; then \
		sed -e s%@\BIN_DIR@%$(bindir)% -e s%@\COMMON_DIR@%$(COMMON_DIR)% -e s%@\DATABASE_DIR@%$(DATABASE_DIR)% -e s%@\DEFAULT_CONFIG_FILE@%$(DEFAULT_CONFIG_FILE)% $(top_srcdir)/installdir/rundig >$(DESTDIR)$(bindir)/rundig; \
		chmod 755 $(DESTDIR)$(bindir)/rundig; \
	fi
	@echo "Installing file-type identification script..."
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@if [ ! -f $(DESTDIR)$(bindir)/HtFileType ]; then \
		sed -es%@\CONFIG_DIR@%$(CONFIG_DIR)% $(top_srcdir)/installdir/HtFileType >$(DESTDIR)$(bindir)/HtFileType; \
		chmod 755 $(DESTDIR)$(bindir)/HtFileType; \
	fi
