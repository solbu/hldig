#!/usr/local/bin/perl

#
# Sample external parser for htdig 3.1.3 or earlier.
# Usage: (in htdig.conf)
#
# external_parsers: application/msword /usr/local/bin/parse_doc.pl \
#               application/postscript /usr/local/bin/parse_doc.pl \
#               application/pdf /usr/local/bin/parse_doc.pl
#
# This is a general external parser script that handles MS Word documents
# among others (deprecated in favor of external converters like conv_doc.pl).
# For older versions of htdig which don't support external converters, if
# you're really stuck using one of them, this script will do an OK job of
# parsing out these documents.  For versions 3.1.4 or later, you're much
# better off using an external converter.  This sample script is still
# included with later versions, because it gives a working example of an
# external parser, so you can use this as a framework for when you REALLY
# NEED an external parser and a simple converter just won't do.
#
# Written by Jesse op den Brouw <MSQL_User@st.hhs.nl>, and enhanced by a
# cast of characters, none of which claim to be Perl experts.  It's pretty
# widely acknowledged that this script is somewhat of a hack.  Did we
# mention that you're better off running doc2html.pl or conv_doc.pl if
# you can?
#

# Use the environment-determined locale for word parsing, etc.
# Note that it's much easier to do all of this as an external converter
# and let htdig worry about the locale, etc.
use locale;

# 1998/12/10
# Added:        push @allwords, $fields[$x];         <carl@dpiwe.tas.gov.au>
# Replaced:     matching patterns. they match words starting or ending with ()[]'`;:?.,! now, not when in between!
# Gone:         the variable $line is gone (using $_ now)
#
# 1998/12/11
# Added:        catdoc test (is catdoc runnable?)    <carl@dpiwe.tas.gov.au>
# Changed:      push line semi-colomn wrong.         <carl@dpiwe.tas.gov.au>
# Changed:      matching works for end of lines now  <carl@dpiwe.tas.gov.au>
# Added:        option to rigorously delete all punctuation <carl@dpiwe.tas.gov.au>
#
# 1999/02/09
# Added:        option to delete all hyphens         <grdetil@scrc.umanitoba.ca>
# Added:        uses ps2ascii to handle PS files     <grdetil@scrc.umanitoba.ca>
# 1999/02/15
# Added:        check for some file formats          <Frank.Richter@hrz.tu-chemnitz.de>
# 1999/02/25
# Added:        uses pdftotext to handle PDF files   <grdetil@scrc.umanitoba.ca>
# Changed:      generates a head record with punct.  <grdetil@scrc.umanitoba.ca>
# 1999/03/01
# Added:        extra checks for file "wrappers"     <grdetil@scrc.umanitoba.ca>
#               & check for MS Word signature (no longer defaults to catdoc)
# 1999/03/05
# Changed:      rejoin hyphenated words across lines <grdetil@scrc.umanitoba.ca>
#               (in PDFs) & remove multiple punct. chars. between words (all)
# 1999/03/10
# Changed:      fix handling of minimum word length  <grdetil@scrc.umanitoba.ca>
# 1999/08/12
# Changed:      adapted for xpdf 0.90 release        <grdetil@scrc.umanitoba.ca>
# Added:        uses pdfinfo to handle PDF titles    <grdetil@scrc.umanitoba.ca>
# Changed:      keep hyphens by default, as htdig    <grdetil@scrc.umanitoba.ca>
#               does, but change dashes to hyphens
# 1999/09/09
# Changed:      fix to handle empty PDF title right  <grdetil@scrc.umanitoba.ca>
# 2000/01/12
# Changed:      "break" to "last" (no break in Perl) <wjones@tc.fluke.com>
# Changed:      code for parsing a line into a list of
#               words, to use "split", other streamlining.
# 2001/07/12
# Changed:      fix "last" handling in dehyphenation <grdetil@scrc.umanitoba.ca>
# Added:        handle %xx codes in title from URL   <grdetil@scrc.umanitoba.ca>
# 2002/01/28
# Added:        use "locale" for better word parsing <ghutchis@wso.williams.edu>
#########################################
#
# set this to your MS Word to text converter
# get it from: http://www.fe.msk.ru/~vitus/catdoc/
#
$CATDOC = "/usr/local/bin/catdoc";
#
# set this to your WordPerfect to text converter, or /bin/true if none available
# this nabs WP documents with .doc suffix, so catdoc doesn't see them
#
$CATWP = "/bin/true";
#
# set this to your RTF to text converter, or /bin/true if none available
# this nabs RTF documents with .doc suffix, so catdoc doesn't see them
#
$CATRTF = "/bin/true";
#
# set this to your PostScript to text converter
# get it from the ghostscript 3.33 (or later) package
#
$CATPS = "/usr/bin/ps2ascii";
#
# set this to your PDF to text converter, and pdfinfo tool
# get it from the xpdf 0.90 package at http://www.foolabs.com/xpdf/
#
$CATPDF = "/usr/bin/pdftotext";
$PDFINFO = "/usr/bin/pdfinfo";
#$CATPDF = "/usr/local/bin/pdftotext";
#$PDFINFO = "/usr/local/bin/pdfinfo";

# need some var's
$minimum_word_length = 3;
$head = "";
@allwords = ();
@temp = ();
$x = 0;
#@fields = ();
$calc = 0;
$dehyphenate = 0;
$title = "";
#
# okay. my programming style isn't that nice, but it works...

#for ($x=0; $x<@ARGV; $x++) {           # print out the args
#       print STDERR "$ARGV[$x]\n";
#}

# Read first bytes of file to check for file type (like file(1) does)
open(FILE, "< $ARGV[0]") || die "Oops. Can't open file $ARGV[0]: $!\n";
read FILE,$magic,8;
close FILE;

if ($magic =~ /^\0\n/) {                # possible MacBinary header
        open(FILE, "< $ARGV[0]") || die "Oops. Can't open file $ARGV[0]: $!\n";
        read FILE,$magic,136;           # let's hope parsers can handle them!
        close FILE;
}

if ($magic =~ /%!|^\033%-12345/) {      # it's PostScript (or HP print job)
        $parser = $CATPS;               # gs 3.33 leaves _temp_.??? files in .
        $parsecmd = "(cd /tmp; $parser; rm -f _temp_.???) < $ARGV[0] |";
# keep quiet even if PS gives errors...
#       $parsecmd = "(cd /tmp; $parser; rm -f _temp_.???) < $ARGV[0] 2>/dev/null |";
        $type = "PostScript";
        $dehyphenate = 0;               # ps2ascii already does this
        if ($magic =~ /^\033%-12345/) { # HP print job
                open(FILE, "< $ARGV[0]") || die "Oops. Can't open file $ARGV[0]: $!\n";
                read FILE,$magic,256;
                close FILE;
                exit unless $magic =~ /^\033%-12345X\@PJL.*\n*.*\n*.*ENTER\s*LANGUAGE\s*=\s*POSTSCRIPT.*\n*.*\n*.*\n%!/
        }
} elsif ($magic =~ /%PDF-/) {           # it's PDF (Acrobat)
        $parser = $CATPDF;
        $parsecmd = "$parser -raw $ARGV[0] - |";
# to handle single-column, strangely laid out PDFs, use coalescing feature...
#       $parsecmd = "$parser $ARGV[0] - |";
        $type = "PDF";
        $dehyphenate = 1;               # PDFs often have hyphenated lines
        if (open(INFO, "$PDFINFO $ARGV[0] 2>/dev/null |")) {
                while (<INFO>) {
                        if (/^Title:/) {
                                $title = $_;
                                $title =~ s/^Title:\s+//;
                                $title =~ s/\s+$//;
                                $title =~ s/\s+/ /g;
                                $title =~ s/&/\&amp\;/g;
                                $title =~ s/</\&lt\;/g;
                                $title =~ s/>/\&gt\;/g;
                                last;
                        }
                }
                close INFO;
        }
} elsif ($magic =~ /WPC/) {             # it's WordPerfect
        $parser = $CATWP;
        $parsecmd = "$parser $ARGV[0] |";
        $type = "WordPerfect";
        $dehyphenate = 0;               # WP documents not likely hyphenated
} elsif ($magic =~ /^{\\rtf/) {         # it's Richtext
        $parser = $CATRTF;
        $parsecmd = "$parser $ARGV[0] |";
        $type = "RTF";
        $dehyphenate = 0;               # RTF documents not likely hyphenated
} elsif ($magic =~ /\320\317\021\340/) {    # it's MS Word
        $parser = $CATDOC;
        $parsecmd = "$parser -a -w $ARGV[0] |";
        $type = "Word";
        $dehyphenate = 0;               # Word documents not likely hyphenated
} else {
        die "Can't determine type of file $ARGV[0]; content-type: $ARGV[1]; URL: $ARGV[2]\n";
}
# print STDERR "$ARGV[0]: $type $parsecmd\n";
die "Hmm. $parser is absent or unwilling to execute.\n" unless -x $parser;


# open it
open(CAT, "$parsecmd") || die "Hmmm. $parser doesn't want to be opened using pipe.\n";
while (<CAT>) {
        while (/[A-Za-z\300-\377]-\s*$/ && $dehyphenate) {
                $_ .= <CAT>;
                last if eof;
                s/([A-Za-z\300-\377])-\s*\n\s*([A-Za-z\300-\377])/$1$2/s
        }
        $head .= " " . $_;
#       s/\s+[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]+|[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]+\s+|^[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]+|[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]+$/ /g;    # replace reading-chars with space (only at end or begin of word, but allow multiple characters)
##       s/\s[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]|[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]\s|^[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]|[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]$/ /g;    # replace reading-chars with space (only at end or begin of word)
##       s/[\(\)\[\]\\\/\^\;\:\"\'\`\.\,\?!\*]/ /g;      # rigorously replace all by <carl@dpiwe.tas.gov.au>
##       s/[\-\255]/ /g;                                 # replace hyphens with space
#       s/[\255]/-/g;                                   # replace dashes with hyphens
#       @fields = split;                                # split up line
#       next if (@fields == 0);                         # skip if no fields (does it speed up?)
#       for ($x=0; $x<@fields; $x++) {                  # check each field if string length >= 3
#               if (length($fields[$x]) >= $minimum_word_length) {
#                       push @allwords, $fields[$x];    # add to list
#               }
#       }

        # Delete valid punctuation.  These are the default values
        # for valid_punctuation, and should be changed other values
        # are specified in the config file.
        tr{-\255._/!#$%^&'}{}d;
        push @allwords, grep { length >= $minimum_word_length } split /\W+/;
}

close CAT;

exit unless @allwords > 0;              # nothing to output

#############################################
# print out the title, if it's set, and not just a file name
if ($title !~ /^$/ && $title !~ /^[A-G]:[^\s]+\.[Pp][Dd][Ff]$/) {
        print "t\t$title\n";
} else {                                        # otherwise generate a title
        @temp = split(/\//, $ARGV[2]);          # get the filename, get rid of basename
        $temp[-1] =~ s/%([A-F0-9][A-F0-9])/pack("C", hex($1))/gie;
        print "t\t$type Document $temp[-1]\n";  # print it
}


#############################################
# print out the head
$head =~ s/^\s+//;                      # remove leading and trailing space
$head =~ s/\s+$//;
$head =~ s/\s+/ /g;
$head =~ s/&/\&amp\;/g;
$head =~ s/</\&lt\;/g;
$head =~ s/>/\&gt\;/g;
print "h\t$head\n";
#$calc = @allwords;
#print "h\t";
##if ($calc >100) {                      # but not more than 100 words
##       $calc = 100;
##}
#for ($x=0; $x<$calc; $x++) {            # print out the words for the exerpt
#        print "$allwords[$x] ";
#}
#print "\n";


#############################################
# now the words
#for ($x=0; $x<@allwords; $x++) {
#       $calc=int(1000*$x/@allwords);           # calculate rel. position (0-1000)
#       print "w\t$allwords[$x]\t$calc\t0\n";   # print out word, rel. pos. and text type (0)
#}
$x = 0;
for ( @allwords ) {
    # print out word, rel. pos. and text type (0)
    printf "w\t%s\t%d\t0\n", $_, 1000*$x++/@allwords;
}

$calc=@allwords;
# print STDERR "# of words indexed: $calc\n";
