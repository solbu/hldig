.SUFFIXES: .in

FILES = httpd.conf srm.conf access.conf mime.types \
        htdig.conf htdig.conf2

all: $(FILES)

clean: 
	rm -fr $(FILES)

.in:
	sed -e "s/_USER_/$(user)/g" \
	    -e "s|_TESTDIR_|$(testdir)|g" \
	    -e "s|_SRCDIR_|$(srcdir)|g" \
	    -e "s|_PORT_|7400|g" \
	    < $< > $@
