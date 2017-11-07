[![Build Status](https://travis-ci.org/andy5995/htdig.svg?branch=master)](https://travis-ci.org/andy5995/htdig)

## Note from Andy Alt

2017-11-06

I may work on the project from time to time. The original developers have
abandoned it.

Any one interested in the project can join the
[chat room](https://join.slack.com/t/htdig/shared_invite/enQtMjY3NDU1MjMwODk3LTdmM2I2OWI5NWI4MzU4Y2JmMjk2MzAxNDYzM2IzZjJmMGE2MDZmMWMxNDY3MjAwOGFjMmE1YjM2MmM4MzVkNzk)

Email: andy400-dev@yahoo.com

To test/build htdig:

* ./configure
* make

Use `./configure --with-ssl` to build with ssl support (otherwise htdig
can't crawl sites starting with `https`)

## Below is the unaltered README from the original development team

```
ht://Dig 3.2.0b6 README
Copyright (c) 1995-2004 The ht://Dig Group
Contributions from many, see htdoc/THANKS.html

ht://Dig is distributed under the GNU Library General Public License (LGPL).
See the COPYING file for license information.

ht://Dig is a world-wide-web search system for an intranet or small internet.

See the htdoc/install.html file for compilation and installation instructions.

This distribution includes:

    * htdig     - A www information gathering and indexing system
    * htfuzzy   - A fuzzy search database creation program
    * htsearch  - A search engine.
    * httools   - An assortment of tools for operating on the databases

    * htlib     - A small general purpose C++ class library
    * htnet     - Net stuff classes
    * htword    - word occurences list handling, aka inverted index
    * htcommon  - ht://Dig specific classes
    * db        - The latest Berkeley DB package from Sleepycat Software

    * htdoc     - ht://Dig documentation in HTML

ht://Dig has been tested extensively under Linux using gcc/g++
If you use g++, make absolutely sure you have libstdc++ installed
as well. (libstdc++ has superceded the older libg++ package.)

To build, first read the htdoc/install.html document

Questions and discussion should be directed to the ht://Dig mailing list
  at htdig-general@lists.sourceforge.net
Find subscription instructions at http://www.htdig.org/mailing.html
Suggestions and bug reports can be made on our bug tracking database
  (see http://www.htdig.org/bugs.html)

For the most up-to-date documentation, use http://www.htdig.org/
```
