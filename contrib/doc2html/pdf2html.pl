#!/usr/bin/perl -w
use strict;
#
# Version 1.0	25-May-2001
# Written by David Adams <d.j.adams@soton.ac.uk>
#
# Uses pdftotext & pdfinfo utilities from the xpdf package
# to read an Adobe Acrobat file and produce HTML output.
#  
# Can be called directly from htdig as an external converter,
#  or may be called by doc2html.pl converter script. 
#

####--- Configuration ---####
# Full paths of pdtotext and pdfinfo
# (get them from the xpdf package at http://www.foolabs.com/xpdf/):

#### YOU MUST SET THESE  ####

my $PDFTOTEXT = "/... .../pdftotext";
my $PDFINFO = "/... .../pdfinfo";
#
# De-hyphenation option (only affects end-of-line hyphens):
my $Dehyphenate = 1;
#
# Set title to be used when none is found:
my $Default_title = "Adobe Acrobat Document";
#  
# make portable to win32 platform or unix:
my $null = "/dev/null";
if ($^O eq "MSWin32") {$null = "nul";}
####--- End of configuration ---###

if (! -x $PDFTOTEXT) { die "Unable to execute pdftotext" }

my $Input = $ARGV[0] || die "Usage: pdf2html.pl filename [mime-type] [URL]";
my $MIME_type = $ARGV[1] || '';
if ($MIME_type and ($MIME_type !~ m#^application/pdf#i)) {
  die "MIME/type $MIME_type wrong";
}

my $Name = $ARGV[2] || '';
$Name =~ s#^.*/##;
$Name =~ s/%([A-F0-9][A-F0-9])/pack("C", hex($1))/gie;

&pdf_head;
&pdf_body;
exit;

#------------------------------------------------------------------------------

sub pdf_head {
#
#  Contributed by Greg Holmes and Michael Fuller
#   (any errors by David Adams)
#
    my $title = '';
    my $subject = '';
    my $keywords = '';
    if (open(INFO, "$PDFINFO '$Input' 2>$null |")) {
        while (<INFO>) {
            if (m/^title:/i) {
                s/^title:\s+//i;
		$title = &clean_pdf($_);
	    } elsif (m/^subject:/i) {
                s/^subject:\s+//i;
                $subject = &clean_pdf($_);
            } elsif (m/^keywords:/i) {
                s/^keywords:\s+//i;
                $keywords = &clean_pdf($_);
            }

        }
        close INFO;
    } else { warn "cannot execute pdfinfo" }
    if (not length $title) {
      if ($Name) {
        $title = '[' . $Name . ']';
      } else {
        $title = $Default_title;
      }
    }

    print "<HTML>\n<HEAD>\n";
    print "<TITLE>$title</TITLE>\n";
    if (length $subject) {
      print '<META NAME="DESCRIPTION" CONTENT="' . $subject. "\">\n";
    }
    if (length $keywords) {
      print '<META NAME="KEYWORDS" CONTENT="' . $keywords . "\">\n";
    }
    print "</HEAD>\n";

###print STDERR "\n$Name:\n";
###print STDERR "\tTitle:\t$title\n";
###print STDERR "\tDescription:\t$subject\n";
###print STDERR "\tKeywords:\t$keywords\n";

}

#------------------------------------------------------------------------------

sub pdf_body {

  my $bline = '';
  open(CAT, "$PDFTOTEXT -raw '$Input' - |") || 
	  die "$PDFTOTEXT doesn't want to be opened using pipe\n";
  print "<BODY>\n";
  while (<CAT>) {
    while ( m/[A-Za-z\300-\377]-\s*$/ && $Dehyphenate) {
	  $_ .= <CAT>;
	  last if eof;
	  s/([A-Za-z\300-\377])-\s*\n\s*([A-Za-z\300-\377])/$1$2/s;
    }
    s/\255/-/g;	# replace dashes with hyphens
    # replace bell, backspace, tab. etc. with single space:
    s/[\000-\040]+/ /g;
    $_ = &HTML($_);
    if (length) {
      print $bline, $_, "\n";
      $bline = "<br>\n";
    } else {
      $bline = "<p>\n";
    }
  }
  close CAT;

  print "</BODY>\n</HTML>\n";
  return;
}

#------------------------------------------------------------------------------

sub HTML {

  my $text = shift;

  $text =~ s/\f/\n/gs;	# replace form feed
  $text =~ s/\s+/ /g;	# replace multiple spaces, etc. with a single space
  $text =~ s/\s+$//gm;	# remove trailing space
  $text =~ s/&/&amp;/g;
  $text =~ s/</&lt;/g;
  $text =~ s/>/&gt;/g;
  chomp $text;

  return $text;
}

#------------------------------------------------------------------------------

sub clean_pdf {
# removes odd pair of characters that may be in pdfinfo output
# Any double quotes are replaced with single

  my $text = shift;
  chomp $text;
  $text =~  s/\376\377//g;
  $text =~  s/\"/\'/g;
  return $text;
}
