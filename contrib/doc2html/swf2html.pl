#!/usr/bin/perl -w
use strict;
#
# Version 1.0	25-May-2001
# Written by David Adams <d.j.adams@soton.ac.uk>
#
# Uses swfparse utlity to extract URL's from  Shockwave flash files
#  
# Can be called directly from htdig as an external converter,
#  or may be called by doc2html.pl converter script. 
#

####--- Configuration ---####
# Full path of swfparse
# (get it from http:/www.htdig.org/files/contrib/contrib/parsers/)

##### YOU MUST SET THIS  ####

my $SWFPARSE = "/.. .../swfdump";
####--- End of configuration ---###

if (! -x $SWFPARSE) { die "Unable to execute swfparse" }

my $Input = $ARGV[0] || die "Usage: swf2html.pl filename [mime-type] [URL]";
my $MIME_type = $ARGV[1] || '';
if ($MIME_type and ($MIME_type !~ m#^application/x-shockwave-flash#i)) {
  die "MIME/type $MIME_type wrong";
}

my $Name = $ARGV[2] || '';
$Name =~ s#^.*/##;
$Name =~ s/%([A-F0-9][A-F0-9])/pack("C", hex($1))/gie;

print <<"HEAD";
<HTML>
<HEAD>
<TITLE>SWF $Name</TITLE>
<META NAME="robots" CONTENT="follow, noindex">
</HEAD>
HEAD

open(CAT, "$SWFPARSE -t '$Input'|") || 
	  die "$SWFPARSE doesn't want to be opened using pipe\n";

print "<BODY>\n";
while (<CAT>) {
    if ($_ !~ m/\s+getUrl\s+(.*?)\s+.*$/) { next }
    my $link = $1; 
    print '<A href="', $link, '"> </a>', "\n";
}
close CAT;

print "</BODY>\n</HTML>\n";

exit;
