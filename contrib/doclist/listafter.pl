#!/usr/local/bin/perl

##
## listafter.pl  (C) 1996 Andrew Scherpbier
##
## This program will list all URLs which were modified after a specified date.
## For each URL, the following fields are displayed:
##   Title
##   Descriptions
##   URL
##   Last modification date (in ctime format)
##
## The date is specified as mm/dd/yyyy
##
## Example usage:
##   listafter.pl 1/1/1996 /opt/www/htdig/sdsu.docdb
##

use GDBM_File;
require('timelocal.pl');

$t = $ARGV[0];
$t =~ m,([0-9]+)/([0-9]+)/([0-9]+),;
$when = timelocal(0, 0, 0, $2, $1 - 1, $3 - 1900);
$dbfile = $ARGV[1];

tie(%docdb, GDBM_File, $dbfile, GDBM_READER, 0) || die "Unable to open $dbfile: $!";

while (($key, $value) = each %docdb)
{
    next if $key =~ /^nextDocID/;
    %record = parse_ref_record($value);
    if ($record{'TIME'} >= $when)
    {
	print "Title:        $record{'TITLE'}\n";
	print "Descriptions: $record{'DESCRIPTIONS'}\n";
	print "URL:          $record{'URL'}\n";
	$w = localtime($record{'TIME'} * 1);
	print "Modified:     $w\n";
	print "\n";
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
