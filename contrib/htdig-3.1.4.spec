Summary: A web indexing and searching system for a small domain or intranet
Name: htdig
Version: 3.1.4
Release: 0
Copyright: GPL
Group: Networking/Utilities
BuildRoot: /var/tmp/htdig-root
Source0: http://www.htdig.org/files/htdig-%{PACKAGE_VERSION}.tar.gz
Patch0: htdig-%{PACKAGE_VERSION}-conf.patch
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
%patch0 -p1

%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=/usr \
	--bindir=/usr/sbin --libexec=/usr/lib --libdir=/usr/lib \
	--mandir=/usr/man --sysconfdir=/etc/htdig \
	--localstatedir=/var/lib/htdig \
	--with-image-dir=/home/httpd/html/htdig \
	--with-cgi-bin-dir=/home/httpd/cgi-bin \
	--with-search-dir=/home/httpd/html
make

%install

rm -rf $RPM_BUILD_ROOT

make INSTALL_ROOT=$RPM_BUILD_ROOT install-strip
mkdir -p $RPM_BUILD_ROOT/etc/cron.daily
ln -s ../../usr/sbin/rundig $RPM_BUILD_ROOT/etc/cron.daily/htdig-dbgen
ln -s ../../../../usr/doc/htdig-%{PACKAGE_VERSION} \
	$RPM_BUILD_ROOT/home/httpd/html/htdig/htdoc

%clean
rm -rf $RPM_BUILD_ROOT

%post
# Only run this if installing for the first time
if [ "$1" = 1 ]; then
	SERVERNAME="`grep '^ServerName' /etc/httpd/conf/httpd.conf | awk 'NR == 1 {print $2}'`"
	[ -z "$SERVERNAME" ] && SERVERNAME="`hostname -f`"
	[ -z "$SERVERNAME" ] && SERVERNAME="localhost"
	echo "start_url:	http://$SERVERNAME/
local_urls:	http://$SERVERNAME/=/home/httpd/html/
local_user_urls:	http://$SERVERNAME/=/home/,/public_html/" >> /etc/htdig/htdig.conf

fi

%files
%defattr(-,root,root)
%config /etc/htdig/htdig.conf
%config /usr/sbin/rundig
%config /home/httpd/html/search.html
/etc/cron.daily/htdig-dbgen
/usr/sbin/htdig
/usr/sbin/htfuzzy
/usr/sbin/htmerge
/usr/sbin/htnotify
/var/lib/htdig
/home/httpd/cgi-bin/htsearch
/home/httpd/html/htdig

%doc CONFIG README htdoc/*

%changelog
* Tue Nov 30 1999 Gilles Detillieux <grdetil@scrc.umanitoba.ca>
  - updated to version 3.1.4

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

