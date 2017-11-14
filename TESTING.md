# TESTING

## Build
* ./configure
* make [-j?]

You can often speed up build time by using `make -j<jobs>`.

Example: `make -j12`

(See https://www.gnu.org/software/make/manual/html_node/Options-Summary.html#Options-Summary for details)

## Testing
* ./configure --prefix=$PWD/testing --with-ssl
* make install

Files will be installed to $source_dir/testing; as a result, you won't have
any problems with write permissions to the database folder, and htdig will
know where to find the htdig.conf (-c won't be needed)

## https URLS
SSL isn't enabled by default, but it's needed for htdig to crawl `https`
urls. Use ./configure --with-ssl to build htdig with ssl support.

## htdig

Change to the `testing` directory and you should see the following directory structure:
 ```
andy@oceanus:~/src/htdig/testing$ ls
bin  cgi-bin  conf  htdocs  include  lib  man  share  var
```

Change to testing/bin and run `./htdig -i` to initialize the database.

Hint: It will search http://www.htdig.org so try using some keywords you find
on that web site.

Use `./htdig --help` to view the options for htdig.

I am going to have to set up a domain or URL that can be crawled for testing.
I plan to do that soon. For now, I suggest using http://www.htdig.org as the
start_url  .. For general testing?

Remove the contents of testing/var/htdig/ (the database files). Then from
testing/bin, run `./htdig -i` but you can use `-ivs` for extra output.
Using `-ivvs` will show even more output.  Then use the
`testing/cgi-bin/htsearch` program to search the database for keywords that
were indexed from the start_url in htdig.conf when `./htdig -i` was run.

'-i' is used to initialize the database for the first time. Normally it
wouldn't be used after that. But while testing new changes, often the files
in the database directory (testing/var/htdig) must be removed to ensure
an accurate test.

## htsearch

Change to `testing/cgi-bin`
Use `./htsearch` to search the database. You will get these two prompts:
```
Enter value for words:
Content-type: text/html

Enter value for format:
```
    At the first prompt, enter a keyword.
    At the second prompt, just hit return.

htdig was designed to be a search engine for a web site. When `htsearch` is
used from the command line, the html code it outputs is displayed. Its true
purpose is to be run through a form on a web site.

Here is an example of where it's used through a web site form. See the box in the
lower-left corner: http://www.htdig.org/

The is (old) html code used to display a form on a web site and request
input from a user. You can see how `htsearch` is called:

```
<form action="http://www.htdig.org/cgi-bin/htsearch" target=body>
  <b>Quick Search:</b><br>
  <font size="-1">
    <input type=text name=words size=15>
  </font>
</form>
```

## Cleaning up

To uninstall from the testing/ directory (or the directory specified by --prefix)

    make uninstall

remove build files

    make clean

remove files created by ./configure

    make distclean
