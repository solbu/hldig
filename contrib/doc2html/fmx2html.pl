#!/opt/local/bin/perl
#!/usr/bin/perl -w
use strict;
#
# Version 1.0	29-Jan-2003
#
# Written by David Adams <d.j.adams@soton.ac.uk>
#
# Uses JGenerator utlity to extract URL's from Shockwave flash files
#  
# Can be called directly from htdig as an external converter,
#  or may be called by doc2html.pl converter script. 
#

####--- Configuration ---####

my $NULL = '/dev/null';
if ($^O eq "MSWin32") {$NULL = "nul";}

$ENV{'TMPDIR'} ||= '/tmp';
my $tfile = $ENV{'TMPDIR'} . "/$$.txt";

# JGenerator directory
my $IVGEN_HOME =  '/... .../jgenerator-2.2';

my $CLASSPATH="$IVGEN_HOME/classes:$IVGEN_HOME/lib/jgen.jar:$IVGEN_HOME/lib/log4j.jar:$IVGEN_HOME/lib/xalan.jar:$IVGEN_HOME/lib/xerces.jar:$IVGEN_HOME/lib/xml-apis.jar:$IVGEN_HOME/lib/ftp.jar:$IVGEN_HOME/lib/js.jar:$IVGEN_HOME/lib/fop-0.20.1.jar:$IVGEN_HOME/lib/commons-jexl-1.0-dev.jar:$IVGEN_HOME/lib/mm.mysql-2.0.14-bin.jar:$IVGEN_HOME/lib/pgjdbc2.jar";

# Could not figure out log4j configuration so using /dev/null 
my $OPTS = "-Dcom.iv.flash.installDir=$IVGEN_HOME -Dcom.iv.flash.log4j.configuration=$NULL";

# Full pathname of java executable
my $JAVA = "/... .../java";

####--- End of configuration ---####

if (! -x $JAVA) { die "Unable to execute Java" }

if (! -d $IVGEN_HOME) { die "Unable to access JGenerator" }

my $Input = $ARGV[0] || die "Usage: fmx2html.pl filename [mime-type] [URL]";
my $MIME_type = $ARGV[1] || '';
if ($MIME_type and ($MIME_type !~ m#^application/x-shockwave-flash#i)) {
  die "MIME/type $MIME_type wrong";
}

my $Name = $ARGV[2] || '';
$Name =~ s#^(.*/)##;
# decode if 2nd argument was a URL 
$Name =~ s/%([A-F0-9][A-F0-9])/pack("C", hex($1))/gie if $1;

####--- Run JGenerator ---####

# Loose STDERR output
open SAVERR, ">& STDERR";
open STDERR, ">> $NULL";

# Send STDOUT output to STDERR
open SAVEOUT, ">& STDOUT";
open STDOUT, ">& SAVERR";

# Run JGenerator
system $JAVA, '-classpath', $CLASSPATH, $OPTS, 'com.iv.flash.Generator', '-dump', $tfile, $Input;

# Reset STDOUT
close STDOUT;
open STDOUT, ">& SAVEOUT";

# Reset STDERR
close STDERR;
open STDERR, ">& SAVERR";

# If no output file then quit silently
if (! -e $tfile) { exit }
if (-z $tfile) { unlink $tfile; exit }

####--- Write HTML output ---####

# write head
print <<"HEAD";
<html>
<head>
<title>Flash MX $Name</title>
<meta name="robots" content="follow, noindex">
</head>
HEAD

# write body
print "<body>\n";
my $c = 0;

open(TFILE, "< $tfile");
while (<TFILE>) {
    if ($_ =~ m/\s+GetURL\s+/) {
      my $link = $_;
      chomp $link;
      $link =~ s/^.*\surl='(.*?)'\s.*$/$1/;
      if ($link =~ m/^FSCommand:/) { next }
      print '<a href="', $link, '"> </a>', "\n";
      $c++;
    } 
}

print "</body>\n</html>\n";
print STDERR "No links extracted\n" if ($c == 0);

unlink $tfile;

exit;
