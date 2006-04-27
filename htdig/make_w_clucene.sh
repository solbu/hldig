#!/bin/sh

set -x 

rm -f clhtdig
rm -f htdig.o

# needed for the buffio.h and tidy.h from the TidyParser
TIDY_INC='../HTMLTidy/include'


g++ -DHAVE_CONFIG_H -I. -I../libhtdig -I../libhtdig/CLuceneAPI -I../include -I../htlib -I../htnet -I../htcommon -I../db -I$TIDY_INC -g -O2 -Wall -fno-rtti -fno-exceptions -c -o htdig.o `test -f 'htdig.cc' || echo './'`htdig.cc


# link command
if test -e htdig.o
then
            g++ -g -O2 -Wall -fno-rtti -fno-exceptions /nfs/local/linux/gcc-3.2.2/lib/libstdc++.a \
            -o clhtdig htdig.o ../libhtdig/libhtdig.so.4.0  \

            # -I$CL_LIB -I$TIDY_LIB 
#../db/.libs/libhtdb.a    ###-L$CL_LIBDIR -L$TIDY_LIBDIR
fi

if test -e clhtdig
then
ldd clhtdig
fi

