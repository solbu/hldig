#!/bin/sh

./configure --with-ssl --with-zlib  --enable-bigfile \
--prefix=$PWD/testing \
&& make

