#!/usr/local/bin/perl

#
# Sample external converter for htdig 3.1.4 or later.
# Usage: (in htdig.conf)
#
# external_parsers: application/msword->text/html /usr/local/bin/conv_doc.pl \
#               application/postscript->text/html /usr/local/bin/conv_doc.pl \
#               application/pdf->text/html /usr/local/bin/conv_doc.pl
#
# Written by Gilles Detillieux <grdetil@scrc.umanitoba.ca>.
# Based in part on the parse_word_doc.pl script, written by
# Jesse op den Brouw <MSQL_User@st.hhs.nl> but heavily revised.
#
# 1998/12/11
# Added:        catdoc test (is catdoc runnable?)    <carl@dpiwe.tas.gov.au>
# 1999/02/09
# Added:        uses ps2ascii to handle PS files     <grdetil@scrc.umanitoba.ca>
# 1999/02/15
# Added:        check for some file formats          <Frank.Richter@hrz.tu-chemnitz.de>
# 1999/02/25
# Added:        uses pdftotext to handle PDF files   <grdetil@scrc.umanitoba.ca>
# 1999/03/01
# Added:        extra checks for file "wrappers"     <grdetil@scrc.umanitoba.ca>
#               & check for MS Word signature (no longer defaults to catdoc)
# 1999/03/05
# Changed:      rejoin hyphenated words across lines <grdetil@scrc.umanitoba.ca>
#               (in PDFs)
# 1999/08/12
# Changed:      adapted for xpdf 0.90 release        <grdetil@scrc.umanitoba.ca>
# Added:        uses pdfinfo to handle PDF titles    <grdetil@scrc.umanitoba.ca>
# Changed:      change dashes to hyphens             <grdetil@scrc.umanitoba.ca>
# 1999/09/09
# Changed:      fix to handle empty PDF title right  <grdetil@scrc.umanitoba.ca>
# 1999/12/01
# Changed:      rewritten as external converter      <grdetil@scrc.umanitoba.ca>
#               stripped out all parser-related code
# Added:        test to silently ignore wrapped EPS files    < " >
# Added:        test for null device on Win32 env.   <PBISSET@emergency.qld.gov.au>
# 2000/01/12
# Changed:      "break" to "last" (no break in Perl) <wjones@tc.fluke.com>
# 2001/07/12
# Changed:      fix "last" handling in dehyphenation <grdetil@scrc.umanitoba.ca>
# Added:        handle %xx codes in title from URL   <grdetil@scrc.umanitoba.ca>
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

#########################################
#
# need some var's
$dehyphenate = 0;                       # set if we must dehyphenate text output
$ishtml = 0;                            # set if converter produces HTML
$null = "";
$magic = "";
$type = "";
$cvtr = "";
$cvtcmd = "";
$title = "";
@parts = ();

# make portable to win32 platform or unix
$null = "/dev/null";
if ($^O eq "MSWin32") {$null = "nul";}


#########################################
#
# Read first bytes of file to check for file type (like file(1) does)
open(FILE, "< $ARGV[0]") || die "Can't open file $ARGV[0]: $!\n";
read FILE,$magic,8;
close FILE;

if ($magic =~ /^\0\n/) {                # possible MacBinary header
    open(FILE, "< $ARGV[0]") || die "Can't open file $ARGV[0]: $!\n";
    read FILE,$magic,136;               # let's hope converters can handle them!
    close FILE;
}

if ($magic =~ /%!|^\033%-12345/) {      # it's PostScript (or HP print job)
    $cvtr = $CATPS;                     # gs 3.33 leaves _temp_.??? files in .
# keep quiet even if PS gives errors...
    $cvtcmd = "(cd /tmp; $cvtr; rm -f _temp_.???) < $ARGV[0] 2>$null";
# allow PS interpreter to give error messages...
#   $cvtcmd = "(cd /tmp; $cvtr; rm -f _temp_.???) < $ARGV[0]";
    $type = "PostScript";
    $dehyphenate = 0;                   # ps2ascii already does this
    if ($magic =~ /^\033%-12345/) {     # HP print job
        open(FILE, "< $ARGV[0]") || die "Can't open file $ARGV[0]: $!\n";
        read FILE,$magic,256;
        close FILE;
        exit unless $magic =~ /^\033%-12345X\@PJL.*\n*.*\n*.*ENTER\s*LANGUAGE\s*=\s*POSTSCRIPT.*\n*.*\n*.*\n%!/
    }
} elsif ($magic =~ /\305\320\323\306\036/) {    # it's a wrapped EPS - ignore
    exit
} elsif ($magic =~ /%PDF-/) {           # it's PDF (Acrobat)
    $cvtr = $CATPDF;
    $cvtcmd = "$cvtr -raw $ARGV[0] -";
# to handle single-column, strangely laid out PDFs, use coalescing feature...
#   $cvtcmd = "$cvtr $ARGV[0] -";
    $type = "PDF";
    $dehyphenate = 1;                   # PDFs often have hyphenated lines
    if (open(INFO, "$PDFINFO $ARGV[0] 2>$null |")) {
        while (<INFO>) {
            if (/^Title:/) {
                s/^Title:\s+//;
                s/\s+$//;
                s/\s+/ /g;
                s/&/\&amp\;/g;
                s/</\&lt\;/g;
                s/>/\&gt\;/g;
                $title = $_;
                last;
            }
        }
        close INFO;
    }
# to use coalescing feature conditionally...
#   if ($title =~ /...Title of Corel DRAW output.../) {
#       $cvtcmd = "$cvtr $ARGV[0] -";
#   }
} elsif ($magic =~ /WPC/) {             # it's WordPerfect
    $cvtr = $CATWP;
    $cvtcmd = "$cvtr $ARGV[0]";
    $type = "WordPerfect";
    $dehyphenate = 0;                   # WP documents not likely hyphenated
} elsif ($magic =~ /^{\\rtf/) {         # it's Richtext
    $cvtr = $CATRTF;
    $cvtcmd = "$cvtr $ARGV[0]";
    $type = "RTF";
    $dehyphenate = 0;                   # RTF documents not likely hyphenated
} elsif ($magic =~ /\320\317\021\340/) {    # it's MS Word
    $cvtr = $CATDOC;
    $cvtcmd = "$cvtr -a -w $ARGV[0]";
    $type = "Word";
    $dehyphenate = 0;                   # Word documents not likely hyphenated
} else {
    die "Can't determine type of file $ARGV[0]; content-type: $ARGV[1]; URL: $ARGV[2]\n";
}

die "$cvtr is absent or unwilling to execute.\n" unless -x $cvtr;

#############################################
#
# Start output.

# if running as a converter for "user-defined" output type...
#print "Content-Type: text/html\n\n";

if ($ishtml) {
    # converter will give its own HTML output
    system("$cvtcmd") || die "$cvtr doesn't want to be run from shell.\n";
    exit;
}

# Produce HTML output from converter's text output, so we can add title.
print "<HTML>\n<head>\n";

# print out the title, if it's set, and not just a file name, or make one up
if ($title eq "" || $title =~ /^[A-G]:[^\s]+\.[Pp][Dd][Ff]$/) {
    @parts = split(/\//, $ARGV[2]);         # get the file basename
    $parts[-1] =~ s/%([A-F0-9][A-F0-9])/pack("C", hex($1))/gie;
    $title = "$type Document $parts[-1]";   # use it in title
}
print "<title>$title</title>\n";

print "</head>\n<body>\n";

# Open file via selected converter, output its text.
open(CAT, "$cvtcmd |") || die "$cvtr doesn't want to be opened using pipe.\n";
while (<CAT>) {
    while (/[A-Za-z\300-\377]-\s*$/ && $dehyphenate) {
        $_ .= <CAT>;
        last if eof;
        s/([A-Za-z\300-\377])-\s*\n\s*([A-Za-z\300-\377])/$1$2/s
    }
    s/[\255]/-/g;                       # replace dashes with hyphens
    s/\f/\n/g;                          # replace form feed
    s/&/\&amp\;/g;                      # HTMLify text
    s/</\&lt\;/g;
    s/>/\&gt\;/g;
    print;
}

print "</body>\n</HTML>\n";

close CAT;

