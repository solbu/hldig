# TESTING

## Build
* ./configure
* make

## Testing
* ./configure --prefix=$PWD/testing --with-ssl
* make install

Files will be installed to $source_dir/testing; as a result, you won't have
any problems with write permissions to the database folder, and htdig will
know where to find the htdig.conf (-c won't be needed)

## https URLS
SSL isn't enabled by default, but it's needed for htdig to crawl `https`
urls. Use ./configure --with-ssl to build htdig with ssl support.

## To run

Change to the `testing` directory and you should see the following directory structure:
 ```
andy@oceanus:~/src/htdig/testing$ ls
bin  cgi-bin  conf  htdocs  include  lib  man  share  var
```
## htdig.conf
It's important to edit `testing/conf/htdig.conf` before testing.

Change to testing/bin and run `./htdig -i` to initialize the database.

Use `./htdig --help` to view the options for htdig.

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

## More details

The following document may have some extra information that you'll find useful,
but it may need updating:

https://andy5995.github.io/htdig/install.html
