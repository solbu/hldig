# Last definitions below override, so change the order to redefine. You can't
# comment them out because %defines are parsed inside comments.
# For Red Hat 7.x...
%define contentdir /var/www
%define commondir /var/www/html/htdig
%define databasedir /var/lib/htdig
%define searchdir %{contentdir}/html/htdig
%define bindir %{_bindir}
# For Red Hat [456].x...
%define contentdir /home/httpd
%define commondir /var/lib/htdig/common
%define databasedir /var/lib/htdig/db
%define searchdir %{contentdir}/html
%define bindir /usr/sbin

Summary: A web indexing and searching system for a small domain or intranet
Name: htdig
Version: 3.1.6
Release: 0
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

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr --mandir=/usr/man \
	--bindir=%{bindir} --libexec=/usr/lib --libdir=/usr/lib \
	--with-image-dir=%{contentdir}/html/htdig \
	--with-cgi-bin-dir=%{contentdir}/cgi-bin \
	--with-search-dir=%{searchdir} \
	--with-config-dir=/etc/htdig \
	--with-common-dir=%{commondir} \
	--with-database-dir=%{databasedir}
make

%install

rm -rf $RPM_BUILD_ROOT

make INSTALL_ROOT=$RPM_BUILD_ROOT install-strip
ln -fs htdig $RPM_BUILD_ROOT%{bindir}/htdump
ln -fs htdig $RPM_BUILD_ROOT%{bindir}/htload
mkdir -p $RPM_BUILD_ROOT/etc/cron.daily
ln -s ../..%{bindir}/rundig $RPM_BUILD_ROOT/etc/cron.daily/htdig-dbgen
ln -s ../../../../usr/doc/htdig-%{PACKAGE_VERSION} \
	$RPM_BUILD_ROOT%{contentdir}/html/htdig/htdoc

%clean
rm -rf $RPM_BUILD_ROOT

%post
# Only run this if installing for the first time
if [ "$1" = 1 ]; then
	SERVERNAME="`grep '^ServerName' /etc/httpd/conf/httpd.conf | awk 'NR == 1 {print $2}'`"
	[ -z "$SERVERNAME" ] && SERVERNAME="`hostname -f`"
	[ -z "$SERVERNAME" ] && SERVERNAME="localhost"
	sed 's/^start_url:.*/#&\
# (See end of file for this parameter.)/' /etc/htdig/htdig.conf > /tmp/ht.$$
	cat /tmp/ht.$$ > /etc/htdig/htdig.conf
	rm /tmp/ht.$$
	cat >> /etc/htdig/htdig.conf <<!

# Automatically set up by htdig RPM, from your current Apache httpd.conf...
# Verify and configure these, and set maintainer above, before running
# %{bindir}/rundig.
# See /usr/doc/htdig*/attrs.html for descriptions of attributes.

# The URL(s) where htdig will start.  See also limit_urls_to above.
start_url:	http://$SERVERNAME/

# These attributes allow indexing server via local filesystem rather than HTTP.
local_urls:	http://$SERVERNAME/=%{contentdir}/html/
local_user_urls:	http://$SERVERNAME/=/home/,/public_html/
!

fi

%files
%defattr(-,root,root)
%config /etc/htdig/htdig.conf
%config %{bindir}/rundig
%config %{searchdir}/search.html
%config %{commondir}/[a-rt-z]*.html
%config %{commondir}/s[a-df-z]*.html
%config %{commondir}/english*
%config %{commondir}/synonyms
%config %{commondir}/bad_words
%config(missingok) /etc/cron.daily/htdig-dbgen
%{bindir}/htdig
%{bindir}/htdump
%{bindir}/htload
%{bindir}/htfuzzy
%{bindir}/htmerge
%{bindir}/htnotify
%dir %{databasedir}
%{contentdir}/cgi-bin/htsearch
%{contentdir}/html/htdig/*.gif
%{contentdir}/html/htdig/*.png
%{contentdir}/html/htdig/htdoc

%doc CONFIG README htdoc/*

%changelog
* Wed Jan  9 2002 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - make use of new ./configure options for pathnames, do away with patch file
  - used variables for many pathnames, to allow easy switchover to 7.x
    (using Powertools-like pathnames for Red Hat 7)

* Fri Sep 28 2001 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - make symlinks for htdump & htload, added to %files list

* Thu Jun  7 2001 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - updated to version 3.1.6

* Thu Feb 17 2000 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - fixed %post script to add more descriptive entries in htdig.conf
  - made cron script a config file

* Wed Feb 16 2000 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - updated to version 3.1.5

* Mon Jun  7 1999 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - fixed %post script to use only first ServerName directive in httpd.conf

* Thu Feb  4 1999 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - put web stuff back in /home/httpd/html & /home/httpd/cgi-bin, so it can
	go over a standard Apache installation on Red Hat
  - cleaned up %install to make use of new features

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

