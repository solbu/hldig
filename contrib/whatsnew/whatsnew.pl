#!/usr/local/bin/perl
#
# whatsnew.pl v1.1  (C) 1996 Iain Lea
# modified 26 Oct 1998 (c) 1998 Jacques Reynes
#
# ChangeLog
# 960321 IL  Reversed sorting to show newest documents first
# 981026 JR  Modified to work with Berkeley DB2.
# 980204 GRH Modified to work with changes in ht://Dig db format
#
# Produces a HTML 'Whats New' page with custom header and footer.
#
#   Title
#   Descriptions
#   URL
#   Last modification date (in ctime format)
#
# The date is specified as yyyymmdd
#
# Usage: whatsnew.pl [options]
#        -h       help
#        -d date  base date [default: $DefDate]
#        -n days  list documents newer than days old [default: $DefDays]
#        -f file  database index [default: $DefIndex]
#        -F file  HTML footer 
#        -H file  HTML header
#        -o file  HTML generated file
#        -v       verbose

use BerkeleyDB;
require 'timelocal.pl';
require 'getopts.pl';

$DefIndex = '    your data base  .docdb';
$DefOutputFile = ' your result file URL created in your web server  whatsnew.html';
$TmpFile = "/tmp/whatsnew.$$";
$DefFooter = '';
$DefHeader = '';
$Verbose = 0;
$NewNum = 0;
$DefDays = 3;
chop (($DefDate = '19'.`date +%y%m%d`));

&ParseCmdLine;

$DefDate =~ /([0-9]{4})([0-9]{2})([0-9]{2})/;
$When = timelocal (0, 0, 0, $3, $2 - 1, $1 - 1900)- ($DefDays * 86400);
$NewDate = localtime ($When);
$dbfile = $DefIndex;

print "Generating 'Whats New' for documents newer than '$NewDate'...\n" if $Verbose;

&ReadDatabase ($DefIndex, $TmpFile);
&WriteWhatsNew ($TmpFile, $DefOutputFile, $DefHeader, $DefFooter);

exit 1;

#############################################################################
# Subroutines
#

sub ParseCmdLine
{
	&Getopts ('d:f:F:hH:n:o:v');

	if ($opt_h ne "") {
		print <<EndOfHelp
Produce an HTML 'Whats New' page with custom header & footer for database.

Usage: $0 [options]
  -h       help
  -d date  base date [default: $DefDate]
  -n days  list documents newer than days old [default: $DefDays]
  -f file  database index [default: $DefIndex]
  -F file  HTML footer 
  -H file  HTML header
  -o file  HTML generated file
  -v       verbose

EndOfHelp
;
		exit 0;
	}       
	$DefDate = $opt_d if ($opt_d ne "");
	$DefDays = $opt_n if ($opt_n ne "");
	$DefIndex = $opt_f if ($opt_f ne "");
	$DefFooter = $opt_F if ($opt_H ne "");
	$DefHeader = $opt_H if ($opt_H ne "");
	$DefOutputFile = $opt_o if ($opt_o ne "");
	$Verbose = 1 if ($opt_v ne "");
}

sub ReadDatabase
{
	my ($Index, $TmpFile) = @_;

	tie %docdb, 'BerkeleyDB::Btree', -Filename => $Index, -Flags => DB_RDONLY || die "Error: $Index - $!";

	open (TMP, ">$TmpFile") || die "Error: $TmpFile - $!\n";

	while (($key, $value) = each %docdb)
	{
	    next if $key =~ /^nextDocID/;
		%rec = parse_ref_record ($value);
		if ($rec{'TIME'} >= $When)
		{
			$Line = "$rec{'TIME'}|$rec{'URL'}|$rec{'TITLE'}|$rec{'DESCRIPTIONS'}\n";
			print $Line if $Verbose;
			print TMP $Line;
			$NewNum++;
		}
	}

	close (TMP);
}

sub WriteWhatsNew
{
	my ($InFile, $OutFile, $Header, $Footer) = @_;

	open (URLS, "sort -r $InFile |") || die "Error: $InFile - $!\n";
	open (HTML, ">$OutFile") || die "Error: $OutFile - $!\n";

	&PrintBoilerPlate ($Header, 1);

	while (<URLS>) {
		chop;
		($Time, $URL, $Title, $Description) = split ('\|');
		$Ctime = localtime ($Time);
		if ($Verbose) {
		print <<EOT
Title:       $Title
Description: $Description
URL:         $URL
Modified:    $Ctime

EOT
;
		}
		print HTML <<EOT
<strong>Title:</strong>       <a href="$URL">$Title</a>
<strong>Description:</strong> $Description
<strong>URL:</strong>         $URL
<strong>Modified:</strong>    $Ctime

EOT
;
	}

	&PrintBoilerPlate ($Footer, 0);

	close (HTML);
	close (URLS);

	unlink ($InFile);
}

sub PrintBoilerPlate
{
	my ($File, $IsHeader) = @_;

	if ($File ne "" && -e $File) { 
		open (FILE, $File) || die "Error: $File - $!\n";
		while (<FILE>) {
			print HTML;
		}
		close (FILE);
	} else {
		if ($IsHeader) {
			print HTML <<EOT
<html>
<head>
<title>Whats New!</title>
</head>
<body>
<h2>Whats New!</h2>
<center>
<a href="/whatsnew.html"><img src="/new.gif"></a>
<a href="/"><img src="/home.gif"></a>
<a href="/intranet.html"><img src="/search.gif"></a>
<a href="mailto:Iain.Lea\@sbs.de"><img src="/contact.gif"></a>
</center>
<hr>
<strong>Found $NewNum documents newer than '$NewDate'</strong>
<pre>
EOT
;
		} else {
			print HTML <<EOT
</pre>
<hr>
<center>
<a href="/whatsnew.html"><img src="/new.gif"></a>
<a href="/"><img src="/home.gif"></a>
<a href="/intranet.html"><img src="/search.gif"></a>
<a href="mailto:Iain.Lea\@sbs.de"><img src="/contact.gif"></a>
</center>
</body>
</html>
EOT
;
		}
	}
}


sub parse_ref_record
{
    local($value) = @_;
    local(%rec, $length, $count, $result);

    while (length($value) > 0)
    {
	$what = unpack("C", $value);
	$value = substr($value, 1);
	if ($what == 0)
	{
	    # ID
	    $rec{"ID"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 1)
	{
	    # TIME
	    $rec{"TIME"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 2)
	{
	    # ACCESSED
	    $rec{"ACCESSED"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 3)
	{
	    # STATE
	    $rec{"STATE"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 4)
	{
	    # SIZE
	    $rec{"SIZE"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 5)
	{
	    # LINKS
	    $rec{"LINKS"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 6)
	{
	    # IMAGESIZE
	    $rec{"IMAGESIZE"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 7)
	{
	    # HOPCOUNT
	    $rec{"HOPCOUNT"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 8)
	{
	    # URL
	    $length = unpack("i", $value);
	    $rec{"URL"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 9)
	{
	    # HEAD
	    $length = unpack("i", $value);
	    $rec{"HEAD"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 10)
	{
	    # TITLE
	    $length = unpack("i", $value);
	    $rec{"TITLE"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 11)
	{
	    # DESCRIPTIONS
	    $count = unpack("i", $value);
	    $value = substr($value, 4);
	    $result = "";
	    foreach (1 .. $count)
	    {
		$length = unpack("i", $value);
		$result = $result . unpack("x4 A$length", $value) . "";
		$value = substr($value, 4 + $length);
	    }
	    chop $result;
	    $rec{"DESCRIPTIONS"} = $result;
	}
	elsif ($what == 12)
	{
	    # ANCHORS
	    $count = unpack("i", $value);
	    $value = substr($value, 4);
	    $result = "";
	    foreach (1 .. $count)
	    {
		$length = unpack("i", $value);
		$result = $result . unpack("x4 A$length", $value) . "";
		$value = substr($value, 4 + $length);
	    }
	    chop $result;
	    $rec{"ANCHORS"} = $result;
	}
	elsif ($what == 13)
	{
	    # EMAIL
	    $length = unpack("i", $value);
	    $rec{"EMAIL"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 14)
	{
	    # NOTIFICATION
	    $length = unpack("i", $value);
	    $rec{"NOTIFICATION"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 15)
	{
	    # SUBJECT
	    $length = unpack("i", $value);
	    $rec{"SUBJECT"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 16)
	{
	    # STRING (ignore, but unpack)
	    $length = unpack("i", $value);
	    $rec{"STRING"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 17)
	{
	    # METADSC
	    $length = unpack("i", $value);
	    $rec{"METADSC"} = unpack("x4 A$length", $value);
	    $value = substr($value, 4 + $length);
	}
	elsif ($what == 18)
	{
	    # BACKLINKS
	    $rec{"BACKLINKS"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
	elsif ($what == 19)
	{
	    # SIGNATURE
	    $rec{"SIG"} = unpack("i", $value);
	    $value = substr($value, 4);
	}
    }
    return %rec;
}

