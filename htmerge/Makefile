# Generated automatically from Makefile.in by configure.
include /home/turtle/src/ht/Makefile.config
include .sniffdir/ofiles.incl

OBJS=		$(OFILES)
TARGET=		htmerge
LOCAL_DEFINES=	-DSORT_PROG=\"/opt/bin/sort\"

all:		$(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)

install:	$(TARGET)
	$(INSTALL) $(TARGET) $(BIN_DIR)

clean:
	$(RM) $(OBJS) $(TARGET) *~ *% *.bak core a.out *.orig

distclean:	clean
	$(RM) Makefile *.proj

depend:
	$(MAKEDEPEND) $(CXXFLAGS) -- $(OBJS)

htmerge.o:		/home/turtle/src/ht/CONFIG
