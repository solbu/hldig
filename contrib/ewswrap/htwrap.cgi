#!/usr/bin/perl -w

# htwrap.cgi
#
# by John Grohol (grohol@cmhc.com)
# Freeware
# v1.00 - 5 Oct 1998
#
# Simple wrapper script for htsearch to 
# do some basic sanity checking on the query
# and tries to re-form it into a valid htsearch query.
#
# This script must be called using the GET method!
#
#_______________________________________________________
# Set some defaults here
# These can be overridden in the calling form

$config         = "htdig";              # htDig config file
$exclude        = "";                   # exclude this url
$restrict       = "";                   # restrict to this url
$format         = "builtin-long";       # results format
$method         = "and";                # default method
$dir            = "/usr/httpd/cgi-bin"; # Set cgi-bin dir

#_______________________________________________________
# Rest of program

 $| = 1;

# Get the form variables

&ParseTags($ENV{'PATH_INFO'});
&ParseTags($ENV{'QUERY_STRING'}); 

$squery         = $tags{'words'};
$restrict       = $tags{'restrict'};
$method         = $tags{'method'};
$format         = $tags{'format'};
$page           = $tags{'page'};

if (not($page)) { $page=1; }

 $squery =~ s/\+//g;
 $squery =~ s/\-//g;
 $squery =~ s/the//g;
 $squery =~ s/not//g;
 $squery =~ s/what//g;

# If someone puts "and" or "or" in the query,
# then it should be a boolean query

 if (($squery =~ " and ") || ($squery =~ " or ")) {
        $method = "boolean"; }

# How many words are there in the query?
 @words = split(/ /,$squery);
 foreach $word (@words) { $xwd++; }

# If there are quotes in the query, we have to
# turn them into parantheses and make it boolean

if (($squery =~ "\"")) {
        $oo = (index($squery,"\""))+1;
        $od = (index($squery,"\"",$oo))-1;
        $op = $od - $oo +1;
        $yty = substr($squery,$oo,$op);
                @wrds = split(/ /,$yty);
                foreach $wrd (@wrds) { $xww++; }


        if ($xww eq 2) {  # Right now, can only handle 2-word phrases
         $oi = (index($yty," "));
         if ($oi > -1) {
                $ytt = substr($yty,0,$oi);
                $john = $od - $oi +1;
                $yte = substr($yty,$oi+1,$john);
                $james = substr($squery,$od+2);
                $james =~ s/ and//g;
                $james =~ s/ / and /g;
                $squery = "($ytt and $yte) $james"; # We turn it into a
                $method = "boolean";                # boolean query
           }

# More than 2 words in quotes (phrase), just
# turn it into one big string of words and set method to "and"

        } else {
         $squery =~ s/\"//g;   # Dump quotes
         $squery =~ s/ and//g; # Dump and's
         $squery =~ s/ or//g;  # Dump or's
         $method = "and";
         $yty = "";
        }
}

# Set the environmental variables

$ENV{'REQUEST_METHOD'} = 'GET';
$ENV{'QUERY_STRING'} = "config=$config&restrict=$restrict&exclude=$exclude&words=$squery&method=$method&format=$format&page=$page"
;

# Run htsearch

system("$dir/htsearch");

exit;

sub ParseTags {
        local($_) = @_;
        local(@terms, $tag, $val);
        s|^/||;
                @terms = split('&');
                foreach $term (@terms) {
                        ($tag,$val) = split('=',$term,2);
      $val =~ tr/+/ /;
      $val =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
      $val =~ s/<!--(.|\n)*-->//g;
      $val =~ s/<([^>]|\n)*>//g;
                        # may override previous value
                        $tags{$tag} = $val;
                }
}

1;
