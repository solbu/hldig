#!/usr/local/bin/perl

use GDBM_File;

##
## wordfreq.pl
## (C) 1995 Andrew Scherpbier <andrew@sdsu.edu>
##
## Will generate a list of words and how frequently they are used
##

tie(%worddb, GDBM_File, $ARGV[0], GDBM_READER, 0) || die "Unable to open $[ARGV[0]: $!";

while (($key, $value) = each %worddb)
{
    $length = length($value) / 20;
    $total = 0;
    foreach $i (0 .. $length - 1)
    {
	($count, $id, $weight, $anchor, $location) =
	    unpack("i i i i i", substr($value, $i * 20, 20));
	$total += $count;
    }
    print "$total\t$key\n";
}

