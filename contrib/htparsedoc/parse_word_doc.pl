#!/usr/local/gnu/bin/perl

#########################################
#
# set this to your catdoc proggie
#
$CATDOC = "/usr/local/htdig/external_parsers/bin/catdoc";


# need some var's
#empty array
@allwords = ();
$x = 0;
$line = "";
@fields = ();
$calc = 0;

#
# okay. my programming style isn't that nice, but it works...

#for ($x=0; $x<@ARGV; $x++) {
#        print STDERR "$ARGV[$x]\n";
#}

open(CAT, "$CATDOC $ARGV[0] |") || die "Hmmm. Something is wrong.\n";
while ($line = <CAT>) {
        @fields = split(/\s+/,$line);
        for ($x=0; $x<@fields; $x++) {
                if ($fields[$x] =~ /\w/) {
                        @allwords = (@allwords, $fields[$x]);
                }
        }
}

close CAT;

#############################################
# print out the title
print "t\tWord Document $ARGV[0]\n";

#############################################
# now the words
for ($x=0; $x<@allwords; $x++) {
        $calc=int(1000*$x/@allwords);           # calculate rel. position (0-10
00)
        print "w\t$allwords[$x]\t$calc\t0\n";   # print out word, rel. pos. and
 text type (0)
}
