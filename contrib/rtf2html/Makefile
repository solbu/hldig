CC= gcc
CFLAGS= -O2 -Wall

rtf2html:	rtf2html.c
		$(CC) $(CFLAGS) -o rtf2html rtf2html.c

install:	rtf2html
	cp rtf2html /usr/local/bin

clean:
	rm -f rtf2html
