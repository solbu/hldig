#!/bin/sh
#
# This is a php wrapper for use with htdig (http://www.htdig.org/)
# this source code can be obtained from:
#      http://freebsddiary.org/samples/htdig-php-wrapper.tar.gz

#
# HTBINDIR needs to be set to the cgi-bin directory in which htsearch resides
#
HTBINDIR=/home/freebsddiary/www/cgi-bin
QUERY_STRING="$@"
REQUEST_METHOD=GET
export QUERY_STRING REQUEST_METHOD
$HTBINDIR/htsearch
