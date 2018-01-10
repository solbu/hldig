#!/bin/sh
#
# You can get "tidy" from your OS distribution or by going to
# its web site @ http://www.htacg.org/

tidy --indent auto --indent-spaces 2 --char-encoding utf8 -m --quiet yes $1
