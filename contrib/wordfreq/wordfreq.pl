#!/usr/local/bin/perl 

use GDBM_File;
use BerkeleyDB;

##
## wordfreq.pl
## (C) 1995 Andrew Scherpbier <andrew@sdsu.edu>
##
## Will generate a list of words and how frequently they are used
##
## updated to deal with Berkeley db files 1998 Iosif Fettich <ifettich@netsoft.ro>
##


$filetype = 'DB';

if (not defined $ARGV[0] or defined ($ARGV[1]) and $ARGV[1] !~ /g/i) {
   print "\n\nThis program is used in conjunction with ht://Dig \n";
   print "to determine the frequency of words in a database containing word references.\n\n";
   print "Usage: $0 filename         (to use a Berkeley db2 wordlist)\n";
   print "       $0 filename g[dbm]  (to use a GDBM wordlist)\n\n\n";
   exit;
}

$filename = $ARGV[0];

if ($filename =~ /gdbm$/i or $ARGV[1] =~ /g/i) {
  $filetype = 'GDBM';
}

if ($filetype eq 'GDBM') {
   tie %worddb, 'GDBM_File', $ARGV[0], GDBM_READER, 0
       or die "Unable to open $ARGV[0] $!";
} else {
   tie %worddb, 'BerkeleyDB::Btree',
              -Filename  => $filename,
              -Flags     => DB_RDONLY
       or die "Cannot open file $filename: $! $BerkeleyDB::Error\n" ;
}

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

