#!/usr/local/bin/perl

##
## changehost.pl  (C) 1995 Andrew Scherpbier
##
## This program will change hostnames of URLs in the document database and index.
##
## usage:
##   changehost.pl database_base from to
##
## example:
##   changehost.pl /opt/www/htdig/sdsu www.sdsu.edu www.northpole.net
##
## Two new database will be created with a base of '/tmp/new'.
## These databases can then be used by htsearch.
##

use GDBM_File;

$base = $ARGV[0];
$from = $ARGV[1];
$to = $ARGV[2];

$dbfile = "$base.docdb";
$newfile = "/tmp/new.docdb";

##
## Convert the document database first.
##
tie(%newdb, GDBM_File, $newfile, GDBM_NEWDB, 0644) || die "$newfile: '$!'";
tie(%docdb, GDBM_File, $dbfile, GDBM_READER, 0) || die "$dbfile: $!";


while (($key, $value) = each %docdb)
{
    if ($key =~ /http:\/\/$from/i)
    {
	%record = parse_ref_record($value);
	$key =~ s/http:\/\/$from/http:\/\/$to/i;
	print "$key\n";
	$t = $record{"URL"};
	$t =~ s/http:\/\/$from/http:\/\/$to/i;
	$record{"URL"} = $t;

	$value = create_ref_record(%record);
    }

    $newdb{$key} = $value;
}

untie %newdb;
untie %docdb;

##
## Now create the document index
##
$newfile = "/tmp/new.docs.index";
$dbfile = "$base.docs.index";

tie(%newdb, GDBM_File, $newfile, GDBM_NEWDB, 0644) || die "$newfile: '$!'";
tie(%docdb, GDBM_File, $dbfile, GDBM_READER, 0) || die "$dbfile: $!";

while (($key, $value) = each %docdb)
{
    if ($value =~ /http:\/\/$from/i)
    {
	$value =~ s/http:\/\/$from/http:\/\/$to/i;
    }
    $newdb{$key} = $value;
}

untie %newdb;
untie %docdb;

######################################################################
sub create_ref_record
{
    local(%rec) = @_;
    local($s);

    if (exists $rec{"ID"})
    {
	$s .= pack("Ci", 0, $rec{"ID"});
    }
    if (exists $rec{"TIME"})
    {
	$s .= pack("Ci", 1, $rec{"TIME"});
    }
    if (exists $rec{"ACCESSED"})
    {
	$s .= pack("Ci", 2, $rec{"ACCESSED"});
    }
    if (exists $rec{"STATE"})
    {
	$s .= pack("Ci", 3, $rec{"STATE"});
    }
    if (exists $rec{"SIZE"})
    {
	$s .= pack("Ci", 4, $rec{"SIZE"});
    }
    if (exists $rec{"LINKS"})
    {
	$s .= pack("Ci", 5, $rec{"LINKS"});
    }
    if (exists $rec{"IMAGESIZE"})
    {
	$s .= pack("Ci", 6, $rec{"IMAGESIZE"});
    }
    if (exists $rec{"HOPCOUNT"})
    {
	$s .= pack("Ci", 7, $rec{"HOPCOUNT"});
    }
    if (exists $rec{"URL"})
    {
	$s .= pack("Ci", 8, length($rec{"URL"}));
	$s .= $rec{"URL"};
    }
    if (exists $rec{"HEAD"})
    {
	$s .= pack("Ci", 9, length($rec{"HEAD"}));
	$s .= $rec{"HEAD"};
    }
    if (exists $rec{"TITLE"})
    {
	$s .= pack("Ci", 10, length($rec{"TITLE"}));
	$s .= $rec{"TITLE"};
    }
    if (exists $rec{"DESCRIPTIONS"})
    {
	@v = split('', $rec{"DESCRIPTIONS"});
	$s .= pack("Ci", 11, $#v - 1);
	foreach (@v)
	{
	    $s .= pack("i", length($_));
	    $s .= $_;
	}
    }
    if (exists $rec{"ANCHORS"})
    {
	@v = split('', $rec{"ANCHORS"});
	$s .= pack("Ci", 12, $#v - 1);
	foreach (@v)
	{
	    $s .= pack("i", length($_));
	    $s .= $_;
	}
    }
    if (exists $rec{"EMAIL"})
    {
	$s .= pack("Ci", 13, length($rec{"EMAIL"}));
	$s .= $rec{"EMAIL"};
    }
    if (exists $rec{"NOTIFICATION"})
    {
	$s .= pack("Ci", 14, length($rec{"NOTIFICATION"}));
	$s .= $rec{"NOTIFICATION"};
    }
    if (exists $rec{"SUBJECT"})
    {
	$s .= pack("Ci", 15, length($rec{"SUBJECT"}));
	$s .= $rec{"SUBJECT"};
    }

    return $s;
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
    }
    return %rec;
}
