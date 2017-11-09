#!/bin/sh

echo 'HTTP/1.1 200 OK'
echo 'Connection: close'
echo 'Content-Type: text/html'
echo 'Location: /set3/nph-location.cgi'
echo
cat <<!
This is the content of the 
document
!
