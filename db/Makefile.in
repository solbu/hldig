all clean distclean:
	cd dist ; if [ -f Makefile ] ; then $(MAKE) $@ ; fi

install:

distdir: distclean
	find * \( ! -name '*~' -a ! -name '.#*' -a ! -name '#*#' -a ! -name core \) -print | egrep -v 'CVS|RCS' | cpio -pdm $(distdir)
