#!/usr/bin/perl -w
#
# Version 1.0	22-March-2000
#
# External converter for htdig 3.1.4 or later (Perl5 or later)
# Usage: (in htdig.conf)
#
#external_parsers:	application/rtf->text/html /opt/local/htdig-3.1.4/scripts/doc2html.pl \
#			text/rtf->text/html /opt/local/htdig-3.1.4/scripts/doc2html.pl \
#			application/pdf->text/html /opt/local/htdig-3.1.4/scripts/doc2html.pl \
#			application/postscript->text/html /opt/local/htdig-3.1.4/scripts/doc2html.pl \
#			application/msword->text/html /opt/local/htdig-3.1.4/scripts/doc2html.pl \
#			application/Wordperfect5.1->text/html /opt/local/htdig-3.1.4/scripts/doc2html.pl
#
#  Uses wp2html to convert Word and WordPerfect documents into HTML, and
#  falls back to using Catdoc if Wp2html is unable to convert.
#  If all else fails, attempts to read file without conversion.

$WP2HTML = "/opt/local/wp2html-3.2/bin/wp2html";

# rtf2html converts Rich Text Font documents to HTML
# (get it from: http://www.res.bbsrc.ac.uk/wp2html/):

$RTF2HTML = "/opt/local/rtf2html-1.1/bin/rtf2html";

# Catdoc converts MS Word to plain text
# (get it from: http://www.fe.msk.ru/~vitus/catdoc/):

# Catdoc converts MS Word to plain text
# (get it from: http://www.fe.msk.ru/~vitus/catdoc/):

#version of catdoc for Word6, Word97, etc. files:
$CATDOC = "/opt/local/catdoc-0.91.4/bin/catdoc";

#version of catdoc for Word2 files:
$CATDOC2 = "/opt/local/catdoc-0.91.4/bin/catdoc";

# PostScript to text converter
# (get it from the ghostscript 3.33 (or later) package):

$CATPS = "/usr/freeware/bin/ps2ascii";

# add to search path directory which contains gs:
$ENV{PATH} .= ":/usr/freeware/bin";

# PDF to text converter and pdfinfo tool
# (get them from the xpdf 0.90 package at http://www.foolabs.com/xpdf/):

$CATPDF = "/opt/local/xpdf-0.9/bin/pdftotext";
$PDFINFO = "/opt/local/xpdf-0.9/bin/pdfinfo";

########################################################################################
# Written by David Adams <d.j.adams@soton.ac.uk>.
# Based on conv_doc.pl written by Gilles Detillieux <grdetil@scrc.umanitoba.ca>,
#   which in turn was based on the parse_word_doc.pl script, written by
#   Jesse op den Brouw <MSQL_User@st.hhs.nl>.
########################################################################################

&start;			# set up variables, etc.
&read_magic;		# Magic reveals type

# see if document->HTML converter will work:
&try_html;
if ($Success) { exit 0 }

# try a document->text converter:
&try_text;
if ($Success) { exit 0 }

# see if a known problem
if (&cannot_do) { exit 0 }

# last-ditch attempt, try copying document
&try_plain;
if ($Success) {exit 0}

print STDERR "UNABLE to do ($Content)\n";
exit 1;

#------------------------------------------------------------------------------

sub start {

  # set = 1 for O/P on stderr if successful
  $Verbose = 1;

  # Directory for temporary files
  $TMP = "/tmp";

  # System command to delete a file
  $RM = "/bin/rm -f";

  # Line editor to do substitution
  $ED = "/bin/sed -e"; 
  if ($^O eq "MSWin32") {$ED = "$^X -pe"}

  $Success = 0;
  $Count = 0;
  $Prog = $0;
  $Prog =~ s#^.*/##; 
  $Prog =~ s/\..*?$//;

  ($Input = $ARGV[0]) || die "No filename given\n";
  $Content = $ARGV[1] || '?';
  $URL = $ARGV[2] || '?';
  $Name = $URL;
  $Name =~ s#^.*/##;
  if ($Verbose) { print STDERR "\n$Prog: $URL " }

  # make portable to win32 platform or unix
  $null = "/dev/null";
  if ($^O eq "MSWin32") {$null = "nul";}

  my ($magic,$cmnd,$cmdl,$type,$description);
  my %set;

  ####Document -> HTML converters####

  # WordPerfect documents
  if ((defined $WP2HTML) and (length $WP2HTML)) {
    $cmd = $WP2HTML;
    $cmdl = "$cmd -q -DTitle=\"[$Name]\" -c doc2html.cfg -s doc2html.sty -i '$Input' -O";
    $magic = '\377WPC';
    &store_html_method('WordPerfect',$cmd,$cmdl,$magic);
  }

  # Word documents
  if ((defined $WP2HTML) and (length $WP2HTML)) {
    $cmd = $WP2HTML;
    $cmdl = "$cmd -q -DTitle=\"[$Name]\" -c doc2html.cfg -s doc2html.sty -i '$Input' -O";
    $magic = '\320\317\021\340';
    &store_html_method('Word',$cmd,$cmdl,$magic);
  }

  # RTF documents
  if ((defined $RTF2HTML) and (length $RTF2HTML)) {
    $cmd = $RTF2HTML;
    # Rtf2html uses filename as title, change this:
    $cmdl = "$cmd '$Input' | $ED 's#^<TITLE>$Input</TITLE>#<TITLE>[$Name]</TITLE>#";
    $magic = '^{\134rtf';
    &store_html_method('RTF',$cmd,$cmdl,$magic);
  }

  ####Document -> Plain Text converters####

  # Word6, Word97, etc. documents
  if ((defined $CATDOC) and (length $CATDOC)) {
    $cmd = $CATDOC;
    # -b option increases chance of success:
    $cmdl = "$cmd -a -b -w '$Input'";
    $magic = '\320\317\021\340';
    &store_text_method('Word',$cmd,$cmdl,$magic,0,'&title');
  }

  # Word2 documents
  if ((defined $CATDOC2) and (length $CATDOC2)) {
    $cmd = $CATDOC2;
    $cmdl = "$cmd -a -b -w '$Input'";
    $magic = '\333\245-\000\000\000\011';
    &store_text_method('Word2',$cmd,$cmdl,$magic,0,'&title');
  }

  # PostScript files
  if ((defined $CATPS) and (length $CATPS)) {
    $cmd = $CATPS;
    # allow PS interpreter to give error messages
    $cmdl = "(cd $TMP; $cmd; $RM _temp_.???) < '$Input'";
    $magic = '%!|^\033%-12345.*\n%!';
    &store_text_method('PostScript',$cmd,$cmdl,$magic,0,'&title');
  }

  # Adobe PDF file
  if ((defined $CATPDF) and (length $CATPDF)) {
    $cmd = $CATPDF;
    $cmdl = "$cmd -raw '$Input' -";
    ## to handle single-column, strangely laid out PDFs, use coalescing feature:
    #$args = "'$Input' -";
    $magic = '%PDF-|\0PDF CARO\001\000\377';
    # PDFs often have hyphenated lines, so last argument is 1:
    # routine &pdf_title will be used for getting title of PDF files :
    &store_text_method('PDF',$cmd,$cmdl,$magic,1,'&pdf_title');
  }

  ####Documents that cannot be converted####

  # wrapped encapsulated Postscript
  $type = "EPS";
  $magic = '^\305\320\323\306\036';
  $description = 'wrapped Encapsulated Postscript';
  &store_cannot_do($type,$magic,$description);

  # Binary (data or whatever)
  $type = "BIN";
  $magic = '[\000-\007\016-\037]'; # rather crude test!
  $description = 'apparently binary';
  &store_cannot_do($type,$magic,$description);

  return;
}

#------------------------------------------------------------------------------

sub read_magic {

  # Read first bytes of file to check for file type (like file(1) does)
  open(FILE, "< $Input") || die "Can't open file $Input\n";
  read FILE,$Magic,8;
  close FILE;

  if ($Magic =~ m/^\0/) {	# possible MacBinary header or a PDF file
    open(FILE, "< $Input") || die "Can't open file $Input\n";
    read FILE,$Magic,136;	# let's hope converters can handle them!
    close FILE;
  }

  if ($Magic =~ /^\033%-12345/) {     # HP print job
    open(FILE, "< $Input") || die "Can't open file $Input\n";
    read FILE,$Magic,256;
    close FILE;
  }

  return;
}

#------------------------------------------------------------------------------

sub try_html  {

  my($set,$cmnd,$type);

  $Success = 0;
  foreach $type (keys %HTML_Method) {
    $set = $HTML_Method{$type};
    if ($Magic =~ m/$set->{'magic'}/s) { # found the method to use
      ###    print $type, "\t", $HTML_Method{$type},"\n";
      my $cmnd = $set->{'cmnd'};
      if (! -x $cmnd) {
	warn "Unable to execute $cmnd for $type document\n";
	return;
      }
      if (not open(CAT, "$set->{'command'} |")) {
	warn "$cmnd doesn't want to be opened using pipe\n";
	return;
      }
      while (<CAT>) {
	# getting something, so it is working
	$Success = 1;
        if ($_ !~ m/^<!--/) { # skip comment lines inserted by converter
	  print;
	  $Count += length;
        }
      }
      close CAT;
      if ($Success) { &announce('HTML',$type) }
      last;
    } 
  }
  return;
}

#------------------------------------------------------------------------------

sub try_text  {

  my($set,$cmnd,$type);

  $Success = 0;
  foreach $type (keys %TEXT_Method) {
    $set = $TEXT_Method{$type};
    if ($Magic =~ m/$set->{'magic'}/s) { # found the method to use
      ###    print $type, "\t", $TEXT_Method{$type},"\n";
      my $cmnd = $set->{'cmnd'};
      if (! -x $cmnd) { die "Unable to execute $cmnd for $type document\n" }
      my $title;
      eval '$title = ' . $set->{'title'};
      print "<HTML>\n<head>\n";
      print "<title>$title</title>\n";
      print "</head>\n<body>\n<pre>\n";

      # Open file via selected converter, output its text:
      open(CAT, "$set->{'command'} |") || 
	  die "$cmnd doesn't want to be opened using pipe\n";
      $Success = 1;
      while (<CAT>) {
	while ( m/[A-Za-z\300-\377]-\s*$/ && $set->{'hyph'}) {
	  ($_ .= <CAT>) || last;
	  s/([A-Za-z\300-\377])-\s*\n\s*([A-Za-z\300-\377])/$1$2/s;
	}
	s/\255/-/g;	# replace dashes with hyphens
       	# replace non-printing characters with spaces:
	s/[^\w\s!@#$%^&*()\-_=+\\|`~[{\]};:'",<.>\/?]/ /g;
	s/\s+/ /g;      # replace multiple spaces, etc. with a single space
	if (length > 1) { # if not just a single character, eg space
	  print &HTML($_), "\n";
	  $Count += length;
	}
      }
      close CAT;

      print "</pre>\n</body>\n</HTML>\n";
      &announce('TEXT',$type);
      last;
    }

  }

  return;
}

#------------------------------------------------------------------------------

sub cannot_do  {

  my ($type,$set);

  # see if known, unconvertable type
  foreach $type (keys %BAD_type) {
    $set = $BAD_type{$type};
    if ($Magic =~ m/$set->{'magic'}/s) { # known problem
      print STDERR "CANNOT DO: ($Content) is $set->{'desc'} ";
      return 1;
    }
  }

  return 0;
}


#------------------------------------------------------------------------------

sub try_plain  {

  $Success = 0;
  if ($Magic =~ m/^[\w\s!@#$%^&*()\-_=+\\|`~[{\]};:'",<.>\/?]/) {
      # starts with ordinary character, go for it:
      open(FILE, "<$Input") || die "Error reading $Input\n";
      $Success = 1;
      my $title;
      $title = &title;
      print "<HTML>\n<head>\n";
      print "<title>$title</title>\n";
      print "</head>\n<body>\n<pre>\n";

      while (<FILE>) {
       	# ruthlessly replace non-word characters with spaces:
	s/\W/ /g;
	s/\s+/ /g;      # replace multiple spaces, etc. with a single space 
        if (length > 2) {
	  print &HTML($_), "\n";
	  $Count += length;
	}
      }
      close FILE;

      print "</pre>\n</body>\n</HTML>\n";
      &announce('TEXT','Plain Text');

  }

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

  return $text;
}

#------------------------------------------------------------------------------

sub store_html_method {

  my $type = shift;
  my $cmnd = shift;
  my $cline = shift;
  my $magic = shift;

  $HTML_Method{$type} = { 
    'magic' => $magic,
    'cmnd' => $cmnd,
    'command' => $cline,
    };

  return;
}

#------------------------------------------------------------------------------

sub store_text_method {

  my $type = shift;
  my $cmnd = shift;
  my $cline = shift;
  my $magic = shift;
  my $hyph = shift;
  my $title = shift;

  $TEXT_Method{$type} = { 
    'magic' => $magic,
    'cmnd' => $cmd,
    'command' => $cline,
    'hyph' => $hyph,
    'title' => $title,
    };

  return;
}

#------------------------------------------------------------------------------

sub store_cannot_do {

  my $type = shift;
  my $magic = shift;
  my $desc = shift;

  $BAD_type{$type} = { 
    'magic' => $magic,
    'desc' => $desc,
    };

  return;

}

#------------------------------------------------------------------------------

sub title {

  return "[" . $Name . "]";

}

#------------------------------------------------------------------------------

sub pdf_title {

    my $title = '';
    if (open(INFO, "$PDFINFO '$Input' 2>$null |")) {
        while (<INFO>) {
            if (m/^title:/i) {
                s/^title:\s+//i;
		$title = $_;
		chomp $title;
                last;
            }
        }
        close INFO;
    }
    if (length $title) {
	$title = '"' . $title . "\" " . &title;
    } else {
	$title = &title;

    }

   return $title;
}

#------------------------------------------------------------------------------

sub announce {

  if ($Verbose) {
    print STDERR "$_[0] $_[1] ($Count) ";
  } 

}
