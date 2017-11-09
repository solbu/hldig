# Last definitions below override, so change the order to redefine. You can't
# comment them out because %defines are parsed inside comments.
# For Red Hat [456].x...
%define contentdir /home/httpd
%define commondir /var/lib/htdig/common
%define databasedir /var/lib/htdig/db
%define searchdir %{contentdir}/html
%define configdir /etc/htdig
%define bindir /usr/sbin
%define mandir /usr/man
%define docdir /usr/doc
# For Red Hat [789].x, FCx...
%define contentdir /var/www
%define commondir %{_prefix}/share/htdig
%define databasedir /var/lib/htdig
%define searchdir %{contentdir}/html/htdig
%define configdir %{_sysconfdir}/htdig
%define bindir %{_bindir}
%define mandir %{_mandir}
%define docdir %{_docdir}
Summary: A web indexing and searching system for a small domain or intranet
Name: htdig
Version: 3.2.0b6
Release: 8
Copyright: GPL
Group: Networking/Utilities
BuildRoot: /var/tmp/htdig-root
Source0: http://www.htdig.org/files/htdig-%{PACKAGE_VERSION}.tar.gz
URL: http://www.htdig.org/
Packager: Gilles Detillieux <grdetil@scrc.umanitoba.ca>

%description
The ht://Dig system is a complete world wide web indexing and searching
system for a small domain or intranet. This system is not meant to replace
the need for powerful internet-wide search systems like Lycos, Infoseek,
Webcrawler and AltaVista. Instead it is meant to cover the search needs for
a single company, campus, or even a particular sub section of a web site.

As opposed to some WAIS-based or web-server based search engines, ht://Dig
can span several web servers at a site. The type of these different web
servers doesn't matter as long as they understand the HTTP 1.0 protocol.
%prep
%setup -q -n htdig-%{PACKAGE_VERSION}
#%patch0 -p0 -b .noparse

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr --mandir=%{mandir} \
	--bindir=%{bindir} --libexec=/usr/lib --libdir=/usr/lib \
	--with-image-dir=%{contentdir}/html/htdig \
	--with-cgi-bin-dir=%{contentdir}/cgi-bin \
	--with-search-dir=%{searchdir} \
	--with-config-dir=%{configdir} \
	--with-common-dir=%{commondir} \
	--with-database-dir=%{databasedir}
#rm -f htlib/langinfo.h		# conflicts with libc5 headers
#echo '#include "/usr/include/langinfo.h"' > htlib/langinfo.h # to keep htlib/Makefile happy
make

%install

rm -rf $RPM_BUILD_ROOT

make DESTDIR=$RPM_BUILD_ROOT install-strip
mkdir -p $RPM_BUILD_ROOT/etc/cron.daily
ln -s ../..%{bindir}/rundig $RPM_BUILD_ROOT/etc/cron.daily/htdig-dbgen
ln -s ../../../..%{docdir}/htdig-%{PACKAGE_VERSION} \
	$RPM_BUILD_ROOT%{contentdir}/html/htdig/htdoc

%clean
rm -rf $RPM_BUILD_ROOT

%post
# Only run this if installing for the first time
if [ "$1" = 1 ]; then
	SERVERNAME="`grep '^ServerName' /etc/httpd/conf/httpd.conf | awk 'NR == 1 {print $2}'`"
	[ -z "$SERVERNAME" ] && SERVERNAME="`hostname -f`"
	[ -z "$SERVERNAME" ] && SERVERNAME="localhost"
	TMPFILE=$(mktemp /tmp/ht.XXXXXX) || exit 1
	sed 's/^start_url:.*/#&\
# (See end of file for this parameter.)/' %{configdir}/htdig.conf > $TMPFILE
	cat $TMPFILE > %{configdir}/htdig.conf
	rm $TMPFILE
	cat >> %{configdir}/htdig.conf <<!

# Automatically set up by htdig RPM, from your current Apache httpd.conf...
# Verify and configure these, and set maintainer above, before running
# %{bindir}/rundig.
# See %{docdir}/htdig*/attrs.html for descriptions of attributes.

# The URL(s) where htdig will start.  See also limit_urls_to above.
start_url:	http://$SERVERNAME/

# These attributes allow indexing server via local filesystem rather than HTTP.
local_urls:	http://$SERVERNAME/=%{contentdir}/html/
local_user_urls:	http://$SERVERNAME/=/home/,/public_html/
!

fi

%files
%defattr(-,root,root)
%config %{configdir}/htdig.conf
%config %{configdir}/mime.types
%config %{configdir}/HtFileType-magic.mime
%config %{configdir}/cookies.txt
%config %{bindir}/rundig
%config %{searchdir}/search.html
%config %{commondir}/[a-rt-z]*.html
%config %{commondir}/s[a-df-z]*.html
%config %{commondir}/english*
%config %{commondir}/synonyms
%config %{commondir}/bad_words
%config(missingok) /etc/cron.daily/htdig-dbgen
%{bindir}/[Hh]t*
/usr/lib/*
/usr/include/*
%dir %{databasedir}
%{contentdir}/cgi-bin/htsearch
%{contentdir}/cgi-bin/qtest
%{contentdir}/html/htdig/*.gif
%{contentdir}/html/htdig/*.png
%{contentdir}/html/htdig/htdoc
%{mandir}/man*

%doc README htdoc/*

%changelog
* Thu Jun 10 2004 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - built with 3.2.0b6, adding man pages & include files
  - updated pathnames for current systems (/usr/share/htdig for common dir)
  - used variable for configdir, mandir & docdir
  - used mktemp to create safe temp file in post script

* Wed Jul  4 2001 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - used variables for many pathnames, to allow easy switchover to 7.x
    (using Powertools-like pathnames for Red Hat 7)

* Thu Jun  7 2001 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - updated to 3.2.0b4

* Fri Dec  1 2000 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - updated to 3.2.0b3

* Mon Feb 21 2000 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - fixed post script to add more descriptive entries in htdig.conf
  - made cron script a config file
  - updated to 3.2.0b2

* Thu Feb  3 2000 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - added mime.types as a config file

* Mon Jan 17 2000 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - updated to 3.2.0b1

* Fri Aug 13 1999 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - changed configure & install options and got rid of conf.patch file
    to work with latest 3.2 code

* Mon Jun  7 1999 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - fixed post script to use only first ServerName directive in httpd.conf

* Tue Mar 23 1999 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - updated to 3.2.0dev, for testing

* Thu Feb  4 1999 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - put web stuff back in /home/httpd/html & /home/httpd/cgi-bin, so it can
	go over a standard Apache installation on Red Hat
  - cleaned up install to make use of new features

* Thu Feb 4 1999 Ric Klaren <klaren@telin.nl>
  - changed buildroot stuff
  - minor spec file fixes
  - install web stuff in /home/httpd/htdig
  - made rundig config file

* Tue Sep 22 1998 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - Added local_urls stuff to generated htdig.conf file

* Fri Sep 18 1998 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - Built the rpm from latest htdig source (3.1.0b1), using earlier
    versions of rpms by Mihai Ibanescu <misa@dntis.ro> and Elliot Lee
    <sopwith@cuc.edu> as a model, incorporating ideas from both.  I've
    made the install locations as FSSTND compliant as I can think of.

