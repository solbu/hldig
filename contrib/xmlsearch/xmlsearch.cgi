#!/bin/sh
#
# xmlsearch.cgi - wrapper for htsearch to get XML output
#	- needed for pre-3.1.6 versions of htsearch, which don't support
#	  the new search_results_contenttype attribute
#

`dirname ${SCRIPT_FILENAME}`/htsearch ${@+"$@"} |
    sed 's|^Content-type: text/html|Content-type: text/xml|'

