# Generated automatically from Makefile.in by configure.
include /home/turtle/src/htdig/Makefile.config
include .sniffdir/ofiles.incl

OBJS=		$(OFILES)

TARGET=		htdig

all:	$(TARGET)

$(TARGET):		$(OBJS)
	$(CXX) -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIBS)

install:	$(TARGET)
	$(INSTALL) $(TARGET) $(BIN_DIR)

clean:
	$(RM) $(TARGET) $(OBJS) *~ *.bak *% a.out *.orig core

distclean:	clean
	$(RM) Makefile *.proj

depend:
	$(MAKEDEPEND)  $(CXXFLAGS) -- $(OBJS)

main.o:	/home/turtle/src/htdig/CONFIG
