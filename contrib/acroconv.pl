#!/usr/local/bin/perl
#
# Sample external converter for htdig 3.1.4 or later, to convert PDFs
# using Adobe Acrobat 3's acroread -toPostScript option on UNIX systems.
# (Use it in place of conv_doc.pl if you have acroread but not pdftotext.)
# Written by Gilles Detillieux.
#
# Usage: (in htdig.conf)
#
# external_parsers: application/pdf->text/html /usr/local/bin/acroconv.pl
#
# This is a pretty quick and dirty implementation, but it does seem to
# give functionality equivalent to the now defunct htdig/PDF.cc parser.
# I'm not a Perl expert by any stretch of the imagination, so the code
# could probably use a lot of optimization to make it work better.
#

$watch = 0;
$bigspace = 0;
$putspace = 0;
$putbody = 1;

system("ln $ARGV[0] $ARGV[0].pdf; acroread -toPostScript $ARGV[0].pdf");
open(INP, "< $ARGV[0].ps") || die "Can't open $ARGV[0].ps\n";

print "<HTML>\n<head>\n";
while (<INP>) {
	if (/^%%Title: / && $putbody) {
		s/^%%Title: \((.*)\).*\n/$1/;
		s/\\222/'/g;
		s/\\267/*/g;
		s/\\336/fi/g;
		s/\\([0-7]{3})/pack(C, oct($1))/eig;
		s/\\([0-7]{2})/pack(C, oct($1))/eig;
		s/\\([0-7])/pack(C, oct($1))/eig;
		s/\\[nrtbf]/ /g;
		s/\\(.)/$1/g;
		s/&/\&amp\;/g;
		s/</\&lt\;/g;
		s/>/\&gt\;/g;
		print "<title>$_</title>\n";
		print "</head>\n<body>\n";
		$putbody = 0;
	} elsif (/^BT/) {
		$watch = 1;
	} elsif (/^ET/) {
		$watch = 0;
		if ($putspace) {
			print "\n";
			$putspace = 0;
		}
	} elsif ($watch) {
		if (/T[Jj]$/) {
			s/\)[^(]*\(//g;
			s/^[^(]*\((.*)\).*\n/$1/;
			s/\\222/'/g;
			s/\\267/*/g;
			s/\\336/fi/g;
			s/\\([0-7]{3})/pack(C, oct($1))/eig;
			s/\\([0-7]{2})/pack(C, oct($1))/eig;
			s/\\([0-7])/pack(C, oct($1))/eig;
			s/\\[nrtbf]/ /g;
			s/\\(.)/$1/g;
			if ($bigspace) {
				s/(.)/$1 /g;
			}
			s/&/\&amp\;/g;
			s/</\&lt\;/g;
			s/>/\&gt\;/g;
			if ($putbody) {
				print "</head>\n<body>\n";
				$putbody = 0;
			}
			print "$_";
			$putspace = 1;
		} elsif (/T[Ddm*]$/ && $putspace) {
			print "\n";
			$putspace = 0;
		} elsif (/Tc$/) {
			$bigspace = 0;
			if (/^([3-9]|[1-9][0-9]+)\..*Tc$/) {
				$bigspace = 1;
			}
		}
	}
}
if ($putbody) {
	print "</head>\n<body>\n";
}
print "</body>\n</HTML>\n";

close(INP);
system("rm -f $ARGV[0].pdf $ARGV[0].ps");
