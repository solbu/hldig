#!/bin/sh

if [ ! -z "$QUERY_STRING" ]
then
	echo 'HTTP/1.1 200 OK'
	echo 'Connection: close'
	echo 'Content-Type: text/html'
	echo
	cat <<!
root::0:root
other::1:
bin::2:root,bin,daemon
sys::3:root,bin,sys,adm
adm::4:root,adm,daemon
uucp::5:root,uucp
mail::6:root
tty::7:root,tty,adm
lp::8:root,lp,adm
nuucp::9:root,nuucp
staff::10:
daemon::12:root,daemon
nobody::60001:
noaccess::60002:
users::100:
basis::200:

!
fi
	
sleep 200

