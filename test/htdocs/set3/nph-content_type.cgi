#!/bin/sh

echo 'HTTP/1.1 200 OK'
echo 'Connection: close'
echo 'Content-Type: text/html ; ISO-8859-1'
echo
cat <<!
This is the content of the 
document
!
