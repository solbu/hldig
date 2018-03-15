# TESTING

## System Requirements

### Linux
Install the following packages:

    build-essential
    libstdc++
    libcrypto
    libssl-dev
    zlib1g-dev

### Mac
Install the appropriate xcode cli tools.

    xcode-select --install

Using homebrew install latest openssl and zlib.

    brew install openssl zlib


### Windows
(Edit me)

## Testing
* ./configure --prefix=$PWD/testing
* make [-j?]

You can often speed up build time by using `make -j<jobs>`.

Example: `make -j5`

Suggested: Use a number that is 1 greater than the number of CPUs on
your computer.

(See https://www.gnu.org/software/make/manual/html_node/Options-Summary.html#Options-Summary for details)

On some systems, a build will fail if using the `j` option. If that happens,
try building using only `make` with no options.

    make install

Files will be installed to $source_dir/testing; as a result, you won't have
any problems with write permissions to the database folder, and hldig will
know where to find the hldig.conf (-c won't be needed)

## https URLS

SSL, which is now enabled by default, is needed for hldig to crawl
`https` urls. To disable support for SSL, use ./configure
--with-openssl=no when building

## install lighttpd

One method we use for testing is having hldig crawl a lightweight web server
that can run locally on your computer. It's more polite than crawling someone's
public web site.

Download the latest source package of [lighttpd](http://www.lighttpd.net/)
and extract it to a directory outside of your `hldig` source directory.

You may have to install some dependencies for lighttpd to build.
Notes about that are on the [lighttpd wiki](http://redmine.lighttpd.net/projects/lighttpd/wiki/InstallFromSource).

    ./configure --prefix=$HOME/usr
    make
    make install

Make an `etc` and a `log` directory

    mkdir -p $HOME/usr/var/log
    mkdir ~/usr/etc

Copy `lighttpd.conf.sample` to ~/usr/etc.
Rename it to `lighttpd.conf` and edit the paths for these two variables:

    server.document-root
    server.breakagelog

cd to $srcdir/samplesite and make a symbolic link to your testing/cgi-bin directory

    ln -s ../testing/share/hldig/cgi-bin

Copy `hlsearch.sh` from your $srcdir/scripts directory to your server root
(not the cgi-bin directory).

_Note: hlsearch.sh is a wrapper script that calls hlsearch._

So lighttpd can find the web graphics and css stylesheet, in your
server root, make a symbolic link to testing/htdocs/hldig

    ln -s ../testing/share/hldig/www

Change back to the top level of your hldig source directory.
Start the lighttpd server

    scripts/run-lighttpd

In your browser, navigate to `http://localhost:3002` and you should see
the hldig local test site.

Later, to kill the server, just use ctrl-c.

Now switch to another terminal window.

## hldig

Change to the `testing` directory and you should see the following directory structure:
```
andy@oceanus:~/src/hldig/testing$ ls
bin  etc  include  lib  share  var
```

Verify that `start_url` in testing/etc/hldig/hldig.conf is set to `http://localhost:3002`

Change to testing/bin and run `./hldig -is` to initialize the database. The
`-i` initializes and adding the `s' shows a summary.

Hint: It will crawl http://localhost:3002 so try using some keywords you find
on the hldig local test site.

Use `./hldig --help` to view the options for hldig.

Use the form on the local test site to search the new database you created.

'-i' is used to initialize the database for the first time. Normally it
wouldn't be used after that. But while testing new changes, often the files
in the database directory (testing/var/htdig) must be removed to ensure
an accurate test.

If you make changes to the source code, you will have to run `make install`
to update the files in testing/.

## hlsearch

### To test `hlsearch` from the console

Change to `testing/share/hldig/cgi-bin`
Use `./hlsearch` to search the database. You will get these two prompts:
```
Enter value for words:
Content-type: text/html

Enter value for format:
```
    At the first prompt, enter a keyword.
    At the second prompt, just hit return.

hldig was designed to be a search engine for a web site. When `hlsearch` is
used from the command line, the html code it outputs is displayed. Its true
purpose is to be run through a form on a web site.

Here is an example of where it's used through a web site form:

[hldig Testing Ground](http://htdig.dreamhosters.com/)

The is (old) html code used to display a form on a web site and request
input from a user. You can see how `hlsearch` is called:

```
<form action="http://localhost:3002/hlsearch.sh" target=body>
  <b>Quick Search:</b><br>
  <font size="-1">
    <input type=text name=words size=15>
  </font>
</form>
```

## Cleaning up

To uninstall from the testing/ directory (or the directory specified by --prefix),
change to the top level of the `hldig` source directory.

    make uninstall

remove build files

    make clean

remove files created by ./configure

    make distclean

## CSS and HTML changes

The css file is located in docs/assets/css. The stylesheet used for the
main web site is the same one used for the samplesite and the hlsearch
results pages. It's copied from that directory to testing/htdocs/hldig/
when `make install` is run.

If you make changes to that css file, or the html files in installdir/,
you'll need to remove the files from **testing/share/hldig** and
**testing/htdocs/hldig** and then then run `make install` from
installdir.

## Web site

Please see [CONTRIBUTING -> Web site](https://github.com/andy5995/hldig/blob/master/CONTRIBUTING.md#web-site) for information
about making changes to the web site.
