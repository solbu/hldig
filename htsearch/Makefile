# Generated automatically from Makefile.in by configure.
include /home/turtle/src/ht/Makefile.config
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

htsearch.o:		/home/turtle/src/ht/CONFIG
