# Generated automatically from Makefile.in by configure.
include /home/turtle/src/www/htdig/Makefile.config
include .sniffdir/ofiles.incl

OBJS=		$(OFILES)
LIBOBJS=	Endings.o Exact.o Fuzzy.o Metaphone.o \
		Soundex.o Synonym.o EndingsDB.o SuffixEntry.o Substring.o

TARGET=		htfuzzy
LIBTARGET=	libfuzzy.a

all:	$(TARGET) $(LIBTARGET)

$(TARGET):		$(OBJS)
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)

$(LIBTARGET):		$(LIBOBJS)
	$(AR) rcv $(LIBTARGET) $?
	$(RANLIB) $(LIBTARGET)

install:	$(TARGET)
	$(INSTALL) $(TARGET) $(BIN_DIR)

clean:
	$(RM) $(TARGET) $(LIBTARGET) $(OBJS) *~ *.bak *% a.out *.orig core

distclean:	clean
	$(RM) Makefile *.proj

depend:
	$(MAKEDEPEND)  $(CXXFLAGS) -- $(OBJS)

htfuzzy.o:	/home/turtle/src/www/htdig/CONFIG
#DO NOT EDIT BELOW
Endings.o: Endings.cc Endings.h Fuzzy.h ../htlib/Object.h \
 ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h htfuzzy.h ../htlib/Configuration.h \
 ../htlib/Dictionary.h ../htlib/List.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h /opt/lib/g++-include/fstream.h \
 /opt/lib/g++-include/iostream.h /opt/lib/g++-include/streambuf.h \
 /opt/lib/g++-include/libio.h /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h
EndingsDB.o: EndingsDB.cc Endings.h Fuzzy.h ../htlib/Object.h \
 ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h htfuzzy.h ../htlib/Configuration.h \
 ../htlib/Dictionary.h ../htlib/List.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h /opt/lib/g++-include/fstream.h \
 /opt/lib/g++-include/iostream.h /opt/lib/g++-include/streambuf.h \
 /opt/lib/g++-include/libio.h /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h \
 SuffixEntry.h ../rx-1.5/rx/rxposix.h ../rx-1.5/rx/rxspencer.h \
 ../rx-1.5/rx/rxproto.h ../rx-1.5/rx/rxnode.h ../rx-1.5/rx/rxbitset.h \
 ../rx-1.5/rx/rxcset.h ../rx-1.5/rx/rxunfa.h ../rx-1.5/rx/_rx.h \
 ../rx-1.5/rx/rxhash.h ../rx-1.5/rx/rxanal.h ../rx-1.5/rx/rxsuper.h \
 ../rx-1.5/rx/rxnfa.h ../rx-1.5/rx/inst-rxposix.h \
 ../rx-1.5/rx/rxcontext.h
Exact.o: Exact.cc Exact.h Fuzzy.h ../htlib/Object.h ../htlib/lib.h \
 /opt/lib/g++-include/string.h /opt/lib/g++-include/cstring \
 /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h ../htlib/List.h
Fuzzy.o: Fuzzy.cc Fuzzy.h ../htlib/Object.h ../htlib/lib.h \
 /opt/lib/g++-include/string.h /opt/lib/g++-include/cstring \
 /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h htfuzzy.h ../htlib/Configuration.h \
 ../htlib/Dictionary.h ../htlib/List.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h /opt/lib/g++-include/fstream.h \
 /opt/lib/g++-include/iostream.h /opt/lib/g++-include/streambuf.h \
 /opt/lib/g++-include/libio.h /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h \
 ../htlib/StringList.h Endings.h Exact.h Metaphone.h Soundex.h \
 Synonym.h Substring.h
Metaphone.o: Metaphone.cc Metaphone.h Fuzzy.h ../htlib/Object.h \
 ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/ctype.h \
 ../htlib/Dictionary.h ../htlib/List.h
Soundex.o: Soundex.cc Soundex.h Fuzzy.h ../htlib/Object.h \
 ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h ../htlib/Dictionary.h ../htlib/List.h
SuffixEntry.o: SuffixEntry.cc SuffixEntry.h ../htlib/Object.h \
 ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 Endings.h Fuzzy.h ../htlib/Database.h
Synonym.o: Synonym.cc Synonym.h Fuzzy.h ../htlib/Object.h \
 ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h htfuzzy.h ../htlib/Configuration.h \
 ../htlib/Dictionary.h ../htlib/List.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h /opt/lib/g++-include/fstream.h \
 /opt/lib/g++-include/iostream.h /opt/lib/g++-include/streambuf.h \
 /opt/lib/g++-include/libio.h /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h \
 ../htlib/StringList.h
htfuzzy.o: htfuzzy.cc htfuzzy.h ../htlib/Configuration.h \
 ../htlib/Dictionary.h ../htlib/Object.h ../htlib/lib.h \
 /opt/lib/g++-include/string.h /opt/lib/g++-include/cstring \
 /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h ../htlib/List.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h /opt/lib/g++-include/fstream.h \
 /opt/lib/g++-include/iostream.h /opt/lib/g++-include/streambuf.h \
 /opt/lib/g++-include/libio.h /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h Fuzzy.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h Soundex.h Endings.h Metaphone.h Synonym.h \
 ../htcommon/defaults.h
Substring.o: Substring.cc Substring.h Fuzzy.h ../htlib/Object.h \
 ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Database.h ../htlib/List.h ../htlib/StringMatch.h \
 ../htlib/Configuration.h ../htlib/Dictionary.h
