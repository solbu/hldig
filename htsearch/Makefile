# Generated automatically from Makefile.in by configure.
include /home/turtle/src/www/htdig/Makefile.config
include .sniffdir/ofiles.incl

OBJS=		$(OFILES)
FOBJS=		../htfuzzy/libfuzzy.a
TARGET=		htsearch
LOCAL_DEFINES=	-DCONFIG_DIR=\"$(CONFIG_DIR)\"

all:	$(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJS) $(FOBJS) $(LIBS) -lgdbm

install:	all
	$(INSTALL) $(TARGET) $(CGIBIN_DIR)/$(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET) *~ *.bak *% core *.orig a.out

distclean:	clean
	$(RM) Makefile *.proj

depend:
	$(MAKEDEPEND)  $(CXXFLAGS) -- $(OBJS)

htsearch.o:		/home/turtle/src/www/htdig/CONFIG
#DO NOT EDIT BELOW
Display.o: Display.cc htsearch.h ../htlib/List.h ../htlib/Object.h \
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
 ../htlib/StringList.h ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Dictionary.h ../htcommon/DocumentRef.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h \
 /opt/lib/g++-include/fstream.h /opt/lib/g++-include/iostream.h \
 /opt/lib/g++-include/streambuf.h /opt/lib/g++-include/libio.h \
 /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h ../htlib/Database.h ../htlib/good_strtok.h \
 ../htcommon/DocumentDB.h ../htlib/Configuration.h ResultMatch.h \
 ResultList.h DocMatch.h ../htcommon/WordReference.h \
 ../htlib/StringMatch.h ../htcommon/defaults.h Display.h \
 TemplateList.h Template.h ../htlib/cgi.h WeightWord.h \
 ../htcommon/WordRecord.h ../htlib/QuotedStringList.h ../htlib/URL.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/ctype.h
DocMatch.o: DocMatch.cc DocMatch.h ../htlib/Object.h ../htlib/lib.h \
 /opt/lib/g++-include/string.h /opt/lib/g++-include/cstring \
 /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h
ResultList.o: ResultList.cc ResultList.h ../htlib/Dictionary.h \
 ../htlib/Object.h ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h ../htlib/List.h \
 DocMatch.h ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h
ResultMatch.o: ResultMatch.cc ResultMatch.h ../htlib/Object.h \
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
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h
Template.o: Template.cc Template.h ../htlib/Object.h ../htlib/lib.h \
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
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h
TemplateList.o: TemplateList.cc TemplateList.h Template.h \
 ../htlib/Object.h ../htlib/lib.h /opt/lib/g++-include/string.h \
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
 ../htlib/List.h ../htlib/URL.h ../htlib/StringList.h
WeightWord.o: WeightWord.cc WeightWord.h ../htlib/htString.h \
 ../htlib/Object.h ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htcommon/WordRecord.h /opt/lib/g++-include/fstream.h \
 /opt/lib/g++-include/iostream.h /opt/lib/g++-include/streambuf.h \
 /opt/lib/g++-include/libio.h /opt/lib/g++-include/_G_config.h
htsearch.o: htsearch.cc htsearch.h ../htlib/List.h ../htlib/Object.h \
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
 ../htlib/StringList.h ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Dictionary.h ../htcommon/DocumentRef.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h \
 /opt/lib/g++-include/fstream.h /opt/lib/g++-include/iostream.h \
 /opt/lib/g++-include/streambuf.h /opt/lib/g++-include/libio.h \
 /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h ../htlib/Database.h ../htlib/good_strtok.h \
 ../htcommon/DocumentDB.h ../htlib/Configuration.h ResultMatch.h \
 ResultList.h DocMatch.h ../htcommon/WordReference.h \
 ../htlib/StringMatch.h ../htcommon/defaults.h WeightWord.h \
 ../htcommon/WordRecord.h parser.h ../htlib/Stack.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/ctype.h Display.h \
 TemplateList.h Template.h ../htlib/cgi.h ../htfuzzy/Fuzzy.h \
 /opt/lib/g++-include/String.h /opt/lib/g++-include/Regex.h \
 ../htcommon/WordList.h ../htlib/IntObject.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/signal.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/signal.h \
 /usr/include/sys/procset.h
parser.o: parser.cc parser.h htsearch.h ../htlib/List.h \
 ../htlib/Object.h ../htlib/lib.h /opt/lib/g++-include/string.h \
 /opt/lib/g++-include/cstring /opt/lib/g++-include/std/cstring.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/string.h \
 /usr/include/sys/feature_tests.h ../include/htconfig.h \
 /usr/include/sys/time.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/types.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/sys/isa_defs.h \
 /usr/include/sys/machtypes.h /usr/include/sys/select.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/time.h \
 /usr/include/sys/siginfo.h /usr/include/sys/machsig.h \
 ../htlib/StringList.h ../htlib/htString.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdarg.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/va-sparc.h \
 ../htlib/Dictionary.h ../htcommon/DocumentRef.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdio.h \
 /opt/lib/g++-include/fstream.h /opt/lib/g++-include/iostream.h \
 /opt/lib/g++-include/streambuf.h /opt/lib/g++-include/libio.h \
 /opt/lib/g++-include/_G_config.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/stdlib.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/unistd.h \
 /usr/include/sys/unistd.h ../htlib/Database.h ../htlib/good_strtok.h \
 ../htcommon/DocumentDB.h ../htlib/Configuration.h ResultMatch.h \
 ResultList.h DocMatch.h ../htcommon/WordReference.h \
 ../htlib/StringMatch.h ../htcommon/defaults.h WeightWord.h \
 ../htcommon/WordRecord.h ../htlib/Stack.h \
 /opt/lib/gcc-lib/sparc-sun-solaris2.5/2.7.2/include/ctype.h
