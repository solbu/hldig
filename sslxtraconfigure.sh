#!/bin/sh

./configure --with-ssl --with-zlib  --enable-bigfile --with-apache \
--prefix=$PWD/testing \
&& make

