#!/usr/bin/perl
# 
# handler.pl
# Sample ExternalTransport handler for HTTP and HTTPS using curl
# for the ht://Dig package 3.2.x and higher
# by Geoffrey Hutchison <ghutchis@wso.williams.edu>
# Copyright (c) 1999 under the terms of the GNU Public License vesion 2 (GPL)
#
# handler.pl protocol url config_file
#
# Really a simplistic example--this should probably use Perl's LWP for HTTP/HTTPS/FTP
# Right now it uses the program 'curl' to do HTTP or HTTPS transactions.
#

my $curl_path="/usr/local/bin/curl";
my $protocol=$ARGV[0];
my $url=$ARGV[1];
my $config_file=$ARGV[2];

open (DOC, "$curl_path -i $url |") || die "s:\t404\nr:\tCan't open curl!\n";
while ( my $line = <DOC> ) {
    if ( $line =~ /^HTTP.?\/\d.\d\s(\d\d\d)\s(.*)/io ) {
	print "s:\t$1\n";
	print "r:\t$2\n";
    } elsif ( $line =~ /^last-modified: (.*)$/io ) {
	print "m:\t$1\n";
    } elsif ( $line =~ /^content-type: (.*)$/io ) {
	print "t:\t$1\n";
    } elsif ( $line =~ /^content-length: (.*)$/io ) {
	print "l:\t$1\n";
    } elsif ( $line =~ /^location: (.*)$/io ) {
	print "u:\t$1\n";
    }

    last if ( $line =~ /^\s*$/ )
}

local($/) = undef;
my $text = <DOC>;
close(DOC);

print "\n$text";



