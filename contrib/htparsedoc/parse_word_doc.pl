#!/usr/local/bin/perl

# 1998/12/10
# Added:        push @allwords, $fields[$x];   <carl@dpiwe.tas.gov.au>
# Replaced:     matching patterns. they match words starting or ending with ()[
]'`;:?.,! now, not when in between!
# Gone:         the variable $line is gone (using $_ now)
#########################################
#
# set this to your catdoc proggie
#
# get it from: http://www.fe.msk.ru/~vitus/catdoc/
#
$CATDOC = "/usr/local/htdig/external_parsers/catdoc/bin/catdoc";

# need some var's
@allwords = ();
@temp = ();
$x = 0;
@fields = ();
$calc = 0;
#
# okay. my programming style isn't that nice, but it works...

#for ($x=0; $x<@ARGV; $x++) {           # print out the args
#       print STDERR "$ARGV[$x]\n";
#}

open(CAT, "$CATDOC -a -w $ARGV[0] |") || die "Hmmm. Something is wrong.\n";
while (<CAT>) {
        s/[\(\)\[\]\\\^\;\:\"\'\`\.\,\?\!]\s|\s[\(\)\[\]\\\^\;\:\"\'\`\.\,\?\!]
|^[\(\)\[\]\\\^\;\:\"\'\`\.\,\?\!]/ /g;  # replace reading-chars with space (on
ly at end or begin of word)
        @fields = split;                        # split up line
        next if (@fields == 0);                 # skip if no fields
        for ($x=0; $x<@fields; $x++) {                  # check each field if s
tring length > 3
                if (length($fields[$x]) > 3) {
                        push @allwords, $fields[$x]     # add to list;
                }
        }
}

close CAT;

#############################################
# print out the title
@temp = split(/\//, $ARGV[2]);          # get the filename, get rid of basename
print "t\tWord Document $temp[-1]\n";   # print it


#############################################
# print out the head
$calc = @allwords;
print "h\t";
#if ($calc >100) {      # but not more than 100 words
#       $calc = 100;
#}
for ($x=0; $x<$calc; $x++) {            # print out the words for the exerpt
        print "$allwords[$x] ";
}
print "\n";


#############################################
# now the words
for ($x=0; $x<@allwords; $x++) {
        $calc=int(1000*$x/@allwords);           # calculate rel. position (0-10
00)
        print "w\t$allwords[$x]\t$calc\t0\n";   # print out word, rel. pos. and
 text type (0)
}

$calc=@allwords;
print STDERR "# of words indexed: $calc\n";
