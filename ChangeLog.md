# ChangeLog

This document contains information about changes after hl://Dig was
forked. To see information about the original project, visit [ht://Dig on SourceForge](https://sourceforge.net/projects/htdig/)

### Thu Jan 11 2018

[#24](https://github.com/andy5995/hldig/issues/24) Added support for a
linked css stylesheet and reformatted the html code of the .html files
in installdir. Those are the files that are used for hlsearch results.
Instructions for
[testing](https://github.com/andy5995/hldig/blob/master/TESTING.md)
have been updated accordingly.

If you've already installed lighttpd for testing (as outlined in
TESTING.md), you'll need to add ".css" to the mimetype.assign section
in ~/usr/etc/lighttpd.conf

```
mimetype.assign = (
  ".html" => "text/html",
  ".txt" => "text/plain",
  ".jpg" => "image/jpeg",
  ".png" => "image/png",
  ".css" => "text/css,
)
```

### Sun Jan 07 2018

ssl support is now enabled by default. Use `./configure --with-openssl=no`
to disable.

### Sat Jan 06 2018

Thanks to [Elliot Alexander](https://github.com/escottalexander) who,
with the help of [Bootstrap](http://getbootstrap.com/), overhauled the [web site](https://andy5995.github.io/hldig/)
and made it **mobile-friendly**.

### Sat Dec 30 2017

hl://Dig was tested and patched by [Martijn de Boer](https://github.com/sexybiggetje)
to build and run on **Mac OS X High Sierra 10.13**.

### Sun Dec 10 2017

This project fork was renamed to hl://Dig

### Sun Nov 19 2017

Updated the auto-files to support internationalization using
[gettext](https://www.gnu.org/software/gettext/manual/html_node/index.html#SEC_Contents),
and added an initial po template that primarily contains the "help" output
of htdig.

### Sat Nov 18 2017

Updated the "configure.ac" and "Makefile.am" files so aclocal, autoreconf,
and automake could be run (renamed the "configure.in" files to "configure.ac")

Documented how to use [lighttpd](http://www.lighttpd.net/)
for [testing](https://github.com/andy5995/htdig/blob/master/TESTING.md)

### Tue Nov 14 2017

Removed former ChangeLog; it would be confusing for newcomers

### Mon Nov 13 2017

Removed htdoc directory. New web site is needed.

Removed former README; it would be confusing for newcomers (a link to
the web site of the original maintainers will remain at the top of this
document)

### Sun Nov 12 2107

Merged a commit (patch apparently from Jim Cole) lost when I reverted to 3.2.0b6
[Fix bug in handling SSL connections.](https://github.com/andy5995/htdig/commit/2aa0e4ed52211003288491dedd3a1e72d1c4ddc1)

### Wed Nov 08 2017

[reverted to 3.2.0b6](https://github.com/andy5995/htdig/commit/af7c7041cf95e60be248a65ca0ee162024e06345)
and applied [patches from the GNU Debian maintainers](https://packages.debian.org/stretch/htdig)

### Mon Nov 06 2017

Mirrored the htdig repo from https://github.com/roklein/htdig

[andy5995 commented](https://github.com/roklein/htdig/issues/1)

>I see it's been some years since this has been worked on. Would you accept
any PRs if they came your way, or would it be best if I made a separate fork
if I decided to promote and continue this project? @roklein


roklein replied

>Thank you for your interest in this project. I think it's better you fork
and continue there. My repository is basically up to version 3.2b6 and some
patches from further down the tree and those used by some major distributions
(suse, fedora,...). So in a way, this is probably the most stable version of
htdig, currently. Unfortunately it seems, I only put the one tree into the
repository (I converted it from CVS), so the older 3.1.6 stuff isn't here.
Also the early 4.0 code is missing, but I think you are way better off, not
using it. Plese note, there is a fork from jrsupplee which has three additional
patches. I didn't review those, however, and never got a pull request.

>Best regards,
Robert
