# Generated automatically from Makefile.in by configure.
include /home/turtle/src/ht/Makefile.config
include .sniffdir/ofiles.incl

OBJS=	$(OFILES)

TARGET=		libht.a

all:		$(TARGET)

$(TARGET):	$(OBJS)
	$(AR) rcv $(TARGET) $?
	$(RANLIB) $(TARGET)

install:	$(TARGET)
	@echo "Nothing to install"

clean:
	$(RM) $(OBJS) $(TARGET) *~ *.bak *% core a.out *.orig

distclean:	clean
	$(RM) Makefile *.proj

depend:
	$(MAKEDEPEND)  $(CXXFLAGS) -- $(OBJS)

