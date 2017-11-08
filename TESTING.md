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

## htdig.conf
It's important to edit this file before testing.
