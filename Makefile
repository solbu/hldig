# Generated automatically from Makefile.in by configure.
include /home/turtle/src/ht/Makefile.config

DIRS=		gdbm-1.7.3 rx-1.5 \
		htlib htcommon htfuzzy htdig \
		htsearch htmerge htnotify
INSTALLDIRS=	htfuzzy htdig \
		htsearch htmerge htnotify
CREATEDIRS=	$(BIN_DIR) $(CONFIG_DIR) $(COMMON_DIR) $(DATABASE_DIR) \
		$(IMAGE_DIR) $(CGIBIN_DIR) $(SEARCH_DIR)
NO_DIST=	BETA.DIST htdig/*.conf htmerge/*.conf htsearch/*.conf \
		htfuzzy/*.conf *.gz */*.gz *~ */*~ mailarchive
IMAGES=		button1.gif button2.gif button3.gif button4.gif button5.gif \
		button6.gif button7.gif button8.gif button9.gif buttonl.gif \
		buttonr.gif button10.gif htdig.gif star.gif star_blank.gif

all:
	@for i in $(DIRS); \
	do \
		(cd $$i; $(MAKE) $(MFLAGS) all); \
	done;

clean:
	@for i in $(DIRS); \
	do \
		(cd $$i; $(MAKE) $(MFLAGS) clean); \
	done;

distclean:
	@for i in $(DIRS); \
	do \
		(cd $$i; $(MAKE) $(MFLAGS) distclean); \
	done;
	$(RM) -rf config.cache config.status config.log config.h \
		Makefile Makefile.config makedp *.gz test *~ \
		include/htconfig.h
	$(RM) -f rx-1.5/config.cache rx-1.5/config.status

install:	all
	@echo "Installing ht://Dig"
	@echo ""
	@echo "Creating directories (if needed)..."
	-@for i in $(CREATEDIRS); \
	do \
		echo "  $$i"; \
		$(INSTALL) -d $$i; \
	done;
	@echo ""
	@echo "Installing individual programs..."
	@for i in $(INSTALLDIRS); \
	do \
		(cd $$i; $(MAKE) $(MFLAGS) install); \
	done;
	@echo ""
	@echo "Installing default configuration files..."
	@if [ ! -f $(CONFIG_DIR)/htdig.conf ]; then $(SED) -e s%@DATABASE_DIR@%$(DATABASE_DIR)% -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% installdir/htdig.conf >$(CONFIG_DIR)/htdig.conf; echo $(CONFIG_DIR)/htdig.conf;fi
	@if [ ! -f $(COMMON_DIR)/bad_words ]; then $(INSTALL) installdir/bad_words $(COMMON_DIR); echo $(COMMON_DIR)/bad_words; fi
	@if [ ! -f $(SEARCH_DIR)/$(SEARCH_FORM) ]; then $(SED) -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% installdir/search.html >$(SEARCH_DIR)/$(SEARCH_FORM); echo $(SEARCH_DIR)/$(SEARCH_FORM);fi
	@if [ ! -f $(COMMON_DIR)/footer.html ]; then $(SED) -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% installdir/footer.html >$(COMMON_DIR)/footer.html; echo $(COMMON_DIR)/footer.html;fi
	@if [ ! -f $(COMMON_DIR)/header.html ]; then $(SED) -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% installdir/header.html >$(COMMON_DIR)/header.html; echo $(COMMON_DIR)/header.html;fi
	@if [ ! -f $(COMMON_DIR)/nomatch.html ]; then $(SED) -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% installdir/nomatch.html >$(COMMON_DIR)/nomatch.html; echo $(COMMON_DIR)/nomatch.html;fi
	@if [ ! -f $(COMMON_DIR)/syntax.html ]; then $(SED) -e s%@IMAGEDIR@%$(IMAGE_URL_PREFIX)% installdir/syntax.html >$(COMMON_DIR)/syntax.html; echo $(COMMON_DIR)/syntax.html;fi
	@if [ ! -f $(COMMON_DIR)/english.0 ]; then $(INSTALL) installdir/english.0 $(COMMON_DIR); echo $(COMMON_DIR)/english.0;fi
	@if [ ! -f $(COMMON_DIR)/english.aff ]; then $(INSTALL) installdir/english.aff $(COMMON_DIR); echo $(COMMON_DIR)/english.aff;fi
	@if [ ! -f $(COMMON_DIR)/synonyms ]; then $(INSTALL) installdir/synonyms $(COMMON_DIR); echo $(COMMON_DIR)/synonyms;fi
	@echo "Installing images..."
	@for i in $(IMAGES); \
	do \
		if [ ! -f $(IMAGE_DIR)/$$i ]; then $(INSTALL) -m 0664 installdir/$$i $(IMAGE_DIR)/$$i; echo $(IMAGE_DIR)/$$i;fi \
	done;
	@echo "Creating rundig script..."
	@if [ ! -f $(BIN_DIR)/rundig ]; then \
		$(SED) -e s%@BIN_DIR@%$(BIN_DIR)% -e s%@COMMON_DIR@%$(COMMON_DIR)% installdir/rundig >$(BIN_DIR)/rundig; \
		chmod 755 $(BIN_DIR)/rundig; \
	fi
	@echo "Installation done."
	@echo ""
	@echo "Before you can start searching, you will need to create a"
	@echo "search database.  A sample script to do this has been"
	@echo "installed as " $(BIN_DIR)/rundig

depend:
	@for i in $(DIRS); \
	do \
		(cd $$i; $(MAKE) $(MFLAGS) depend); \
	done;

dist:
	$(RM) -r $(DISTDIR)
	mkdir $(DISTDIR)
	$(TAR) cf - . | (cd $(DISTDIR); $(TAR) xf -)
	(cd $(DISTDIR); $(MAKE) distclean)
	-for i in $(DIRS); \
	do \
		(cd $(DISTDIR)/$$i; $(RM) -fr RCS docu); \
	done;
	(cd $(DISTDIR); $(RM) -rf $(NO_DIST))
	(cd $(DISTDIR)/..; $(TAR) czf $(DIST).tar.gz $(DIST))

tar:
	tar czf ht.tar.gz --exclude ht.tar.gz .

