#!/bin/sh

echo 'HTTP/1.1 200 OK'
echo 'Connection: close'
echo 'Content-Type: text/html'
echo 'Location: /nph-location.cgi'
echo
cat <<!
This is the content of the 
document
!
