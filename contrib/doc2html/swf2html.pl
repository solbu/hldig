#!/opt/local/bin/perl
#!/usr/bin/perl -w
use strict;
#
# Version 1.1	17-May-2002
#		19-Sep-2002
#		 6-Nov-2002
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
my $SWFPARSE = "/... .../swfdump";
####--- End of configuration ---###

if (! -x $SWFPARSE) { die "Unable to execute swfparse" }

my $Input = $ARGV[0] || die "Usage: swf2html.pl filename [mime-type] [URL]";
my $MIME_type = $ARGV[1] || '';
if ($MIME_type and ($MIME_type !~ m#^application/x-shockwave-flash#i)) {
  die "MIME/type $MIME_type wrong";
}

my $Name = $ARGV[2] || '';
$Name =~ s#^(.*/)##;
# decode if 2nd argument was a URL 
$Name =~ s/%([A-F0-9][A-F0-9])/pack("C", hex($1))/gie if $1;

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
my $c = 0;
while (<CAT>) {
###    if ($_ !~ m/\s+getUrl\s+(.*?)\s+.*$/) { next }
    if ($_ !~ m/\s+getUrl\s+(.*)$/) { next }
    my $link = $1 . ' ';
    if ($link =~ m/^FSCommand:/) { next }
    if ($link =~ m/\s+target\s+/) {
      $link =~ s/^(.*)\s+target\s+.*$/$1/;  
    } else {
      $link =~ s/^(.*?)\s+.*$/$1/; 
    }
    print '<A href="', $link, '"> </a>', "\n";
    $c++;
}
close CAT;

print "</BODY>\n</HTML>\n";
print STDERR "No links extracted\n" if ($c == 0);

exit;
