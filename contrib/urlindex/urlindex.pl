#!/usr/local/bin/perl

##
## urlindex.pl  (C) 1995 Andrew Scherpbier
##
## This program will build an index of all the URLs in the
## htdig document database.
##

use GDBM_File;
require('SDSU_www.pl');

$dbfile = "/gopher/www/htdig/sdsu3d.docdb";
$dbfile = "/tmp/db.docdb";
$exclude = "rohan.sdsu.edu\\/home\\/";

tie(%docdb, GDBM_File, $dbfile, GDBM_READER, 0) || die "Unable to open $dbfile: $!";

print "Reading...\n";

##
## Read in all the relevant data.
##
while (($key, $value) = each %docdb)
{
    next if $key =~ /^nextDocID/;
    %record = parse_ref_record($value);
    next if $record{"STATE"} eq 1;
    next if $key =~ /$exclude/;

    $title = $record{"TITLE"};
    
    ##
    ## Get rid of starting and trailing whitespace junk
    ##
    $title =~ s/^[ \t\n\r]*//;
    $title =~ s/[ \t\n\r]*$//;
    
    ##
    ## If the title starts with 'the', it will be taken out and added
    ## to the end of the title.  This means that a title like "The
    ## Homepage of X" will become "Homepage of X, The"
    ##
    if ($title =~ /^the /i)
    {
	$title = substr($title, 4) . ", " . substr($title, 0, 3);
    }
    if ($title =~ /^SDSU /)
    {
	$title = substr($title, 5) . ", " . substr($title, 0, 4);
    }
    if ($title =~ /^San Diego State University /i)
    {
	$title = substr($title, 27) . ", " . substr($title, 0, 26);
    }
    $value = $title;
    $value =~ tr/A-Z/a-z/;
    $titles{$value} = "$title\001$key";
    push(@unsorted, $value);
}

$current = " ";
open(M, ">index.html");
print M "<html><head><title>Index of all documents at SDSU</title></head>\n";
print M "<body>\n";
print M &www_logo_2("Index of all documents at SDSU");
print M "<p>This is a list of WWW documents that were found while indexing all\n";
print M "the publicly available WWW servers at San Diego State University.\n";
print M "The documents are indexed by their titles.\n";
print M "</p><h2>\n";

$previous = "";

print "Writing...\n";

foreach $value (sort @unsorted)
{
    next if $value eq $previous;
    $previous = $value;
    next if !($value =~ /^[a-zA-Z]/);

    ($title, $url) = split('\001', $titles{$value}, 2);

    $first = substr($title, 0, 1);
    if ($current =~ /$first/i)
    {
	print F "<li><a href=\"$url\">$title</a></li>\n";
    }
    else
    {
	##
	## New letter.  Open a new file for it
	##
	$current = $first;
	$current =~ tr/a-z/A-Z/;
	print F "</li></body></html>\n";
	close(F);
	open(F, ">index$current.html");
	print F "<html><head><title>Index for $current</title></head>\n";
	print F "<body>\n";
	print F &www_logo_2("Index for $current");
	print F "<ul>\n";
	print F "<li><a href=\"$url\">$title</a></li>\n";

	##
	## Add a reference to the main index for this letter
	##
	print M " <a href=\"index$current.html\">$current</a>\n";

	print "Index of $current\n";
    }
}

close(F);

print M "</h2></body></html>\n";
close(M);


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
    print "title = $rec{'TITLE'}\n";
    return %rec;
}







