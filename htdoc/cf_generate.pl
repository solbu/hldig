#
# cf_generate.pl
#
# cf_generate: Build the files cf_byprog.html, cf_byname.html and
#              attrs.html from the informations found 
#              in ../htcommon/defaults.cc. 
#              attrs.html : attrs_head.html + generation + attrs_tail.html
#              cf_byprog.html : cf_byprog_head.html + generation + cf_byprog_tail.html
#              cf_byname.html : cf_byname_head.html + generation + cf_byname_tail.html
#              
# Part of the ht:#Dig package   <http://www.htdig.org/>
# Copyright (c) 1999 The ht://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU Public License version 2 or later
# <http://www.gnu.org/copyleft/gpl.html>
#
# $Id: cf_generate.pl,v 1.1.2.3 1999/10/27 18:58:33 grdetil Exp $
#
use strict;

use vars qw(%char2quote);

%char2quote = (
			       '>' => '&gt;',
			       '<' => '&lt;',
			       '&' => '&amp;',
			       "'" => '&#39;',
			       '"' => '&quot;',
			       );

sub html_escape {
    my($toencode) = @_;

    return undef if(!defined($toencode));

    $toencode =~ s;([&\"<>\']);$char2quote{$1};ge;
    return $toencode;
}

#
# Read and parse attributes descriptions found in defaults.cc
#
local($/) = undef;
my($file) = "../htcommon/defaults.cc";
my($content);
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
close(FILE);

#
# Change curly to square brackets to generate perl arrays instead
# of hashes. Order is important.
#
$content =~ s/.*ConfigDefaults.*?\{(.*)\{0, 0.*/[$1]/s;
$content =~ s/([\@\$])/\\$1/gs;
$content =~ s/^\{/\[/mg;
$content =~ s/^\" \},$/\" \],/mg;
#
# Transform macro substituted strings by strings
#
$content =~ s|^(\[ \"\w+\", )([A-Z].*?), \n|$1\"$2\",\n|mg;
$content =~ s/^(\[ \"\w+\", )\"(.*?)\"(.*?)\"(.*?)\",\n/$1\"$2\\\"$3\\\"$4\",\n/mg;
my($config);
eval "\$config = $content";


#
# Spit the HTML pages
#

my($file);
#
# Complete list of attributes with descriptions and examples.
#
$file = "attrs.html";
open(ATTR, ">$file") or die "cannot open $file for writing : $!";

$file = "attrs_head.html";
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
print ATTR $content;
close(FILE);

#
# Index by attribute name
#
$file = "cf_byname.html";
open(BYNAME, ">$file") or die "cannot open $file for writing : $!";

$file = "cf_byname_head.html";
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
print BYNAME $content;
close(FILE);

my($letter) = '';
my($record);
foreach $record (@$config) {
    my($name, $default, $type, $programs, $example, $description) = @$record;

    if($letter ne uc(substr($name, 0, 1))) {
	print BYNAME "\t</font> <br>\n" if($letter);
	$letter = uc(substr($name, 0, 1));
	print BYNAME "\t<b>$letter</b> <font face=\"helvetica,arial\" size=\"2\"><br>\n";
    }

    print BYNAME "\t <img src=\"dot.gif\" alt=\"*\" width=9 height=9> <a target=\"body\" href=\"attrs.html#$name\">$name</a><br>\n";

    my($used_by) = join(",\n\t\t\t",
			map {
			    my($top) = $_ eq 'htsearch' ? " target=\"_top\"" : "";
			    "<a href=\"$_.html\"$top>$_</a>";
			}
			split(' ', $programs));

    if(!($example =~ /^$name:/)) {
	$example = "\t\t\t  <tr> <td valign=\"top\"><i>No example provided</i></td> </tr>\n";
    } elsif($example =~ /\A$name:\s*\Z/s) {
	$example = "\t\t\t  <tr> <td valign=\"top\">$name:</td> </tr>\n";
    } else {
	my($one);
	my($html) = '';
	foreach $one (split("$name:", $example)) {
	    next if($one =~ /^\s*$/);
	    $html .= <<EOF;
			  <tr>
				<td valign="top">
				  $name:
				</td>
				<td nowrap>
				    $one
				</td>
			  </tr>
EOF
	}
	$example = $html;
    }

    if($default =~ /^\s*$/) {
	$default = "<i>No default</i>";
    } else {
	$default =~ s/^([A-Z][A-Z_]*) \" (.*?)\"/$1 $2/;	# for PDF_PARSER
	$default = html_escape($default);
    }
    print ATTR <<EOF;
	<dl>
	  <dt>
		<strong><a name="$name">
		$name</a></strong>
	  </dt>
	  <dd>
		<dl>
		  <dt>
			<em>type:</em>
		  </dt>
		  <dd>
			$type
		  </dd>
		  <dt>
			<em>used by:</em>
		  </dt>
		  <dd>
			$used_by
		  </dd>
		  <dt>
			<em>default:</em>
		  </dt>
		  <dd>
			$default
		  </dd>
		  <dt>
			<em>description:</em>
		  </dt>
		  <dd>$description		  </dd>
		  <dt>
			<em>example:</em>
		  </dt>
		  <dd>
			<table border="0">
$example			</table>
		  </dd>
		</dl>
	  </dd>
	</dl>
	<hr>
EOF
    
}

my($file) = "attrs_tail.html";
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
print ATTR $content;
close(FILE);

my($file) = "cf_byname_tail.html";
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
print BYNAME $content;
close(FILE);

close(ATTR);
close(BYNAME);

#
# Index by program name
#
$file = "cf_byprog.html";
open(BYPROG, ">$file") or die "cannot open $file for writing : $!";

$file = "cf_byprog_head.html";
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
print BYPROG $content;
close(FILE);

my(%prog2attr);
foreach $record (@$config) {
    my($name, $default, $type, $programs, $example, $description) = @$record;

    my($prog);
    foreach $prog (split(' ', $programs)) {
	push(@{$prog2attr{$prog}}, $record);
    }
}

my($prog);
foreach $prog (sort(keys(%prog2attr))) {
    my($top) = $prog eq 'htsearch' ? "target=\"_top\"" : "target=\"body\"";
    print BYPROG "\t<br><b><a href=\"$prog.html\" $top>$prog</a></b> <font face=\"helvetica,arial\" size=\"2\"><br>\n";
    my($record);
    foreach $record (@{$prog2attr{$prog}}) {
	my($name, $default, $type, $programs, $example, $description) = @$record;
	print BYPROG "\t <img src=\"dot.gif\" alt=\"*\" width=9 height=9> <a target=\"body\" href=\"attrs.html#$name\">$name</a><br>\n";
    }
}

my($file) = "cf_byprog_tail.html";
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
print BYPROG $content;
close(FILE);

close(BYPROG);
