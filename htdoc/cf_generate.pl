#
# cf_generate.pl
#
# cf_generate: Build the files cf_byprog.html, cf_byname.html and
#           attrs.html from the informations found 
#           in ../htcommon/defaults.cc. 
#           attrs.html : attrs_head.html + generation + attrs_tail.html
#           cf_byprog.html : cf_byprog_head.html + generation + cf_byprog_tail.html
#           cf_byname.html : cf_byname_head.html + generation + cf_byname_tail.html
#              
# Part of the ht://Dig package   <http://www.htdig.org/>
# Copyright (c) 1999-2003 The ht://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU Library General Public License (LGPL) version 2 or later
# <http://www.gnu.org/copyleft/lgpl.html>
#
# $Id: cf_generate.pl,v 1.5 2003/08/28 00:45:32 angusgb Exp $
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

my($dir);
if (scalar(@ARGV) == 0) {
    $dir = '..';
}
else {
    $dir = @ARGV[0];
}

local($/) = undef;
my($file) = $dir . "/htcommon/defaults.cc";
my($content);
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
close(FILE);

#
# Change curly to square brackets to generate perl arrays instead
# of hashes. Order is important.
#
$content =~ s/.*ConfigDefaults.*?\{(.*)\{0, 0.*/[$1]/s;
$content =~ s/\s*\\*$//mg;
$content =~ s/([\@\$])/\\$1/gs;
$content =~ s/^\{/\[/mg;
$content =~ s/^\"\s*\},$/\" \],/mg;
#
# Transform macro substituted strings by strings
#
$content =~ s|^(\[ \"\w+\", )([A-Z].*?),\n|$1\"$2\",\n|mg;
#$content =~ s/^(\[ \"\w+\", )\"(.*?)\"(.*?)\"(.*?)\",\n/$1\"$2\\\"$3\\\"$4\",\n/mg;
my($config);
eval "\$config = $content";

if(!$config) {
    die "could not extract any configuration info from $file";
}

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
    my($name, $default, $type, $programs, $block, $version, $category, $example, $description) = @$record;

    if($letter ne uc(substr($name, 0, 1))) {
	print BYNAME "\t</font> <br>\n" if($letter);
	$letter = uc(substr($name, 0, 1));
	print BYNAME "\t<strong>$letter</strong> <font face=\"helvetica,arial\" size=\"2\"><br>\n";
    }

    print BYNAME "\t <img src=\"dot.gif\" alt=\"*\" width=9 height=9> <a target=\"body\" href=\"attrs.html#$name\">$name</a><br>\n";

    my($used_by) = join(",\n\t\t\t",
			map {
			    my($top) = $_ eq 'htsearch' ? " target=\"_top\"" : "";
			    "<a href=\"$_.html\"$top>$_</a>";
			}
			split(' ', $programs));

    if ($block eq '') {
	$block = "Global";
    }

    if($version != 'all') {
	$version = "$version or later";
    }

    if(!($example =~ /^$name:/)) {
	$example = "\t\t\t  <tr> <td valign=\"top\"><em>No example provided</em></td> </tr>\n";
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
	$default = "<em>No default</em>";
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
			<em>block:</em>
		  </dt>
		  <dd>
			$block
		  </dd>
		  <dt>
		      <em>version:</em>
		  </dt>
		  <dd>
		      $version
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

open(FILE, "date |") or die "cannot open pipe to date command for reading : $!";
$content = <FILE>;
close(FILE);
my($date) = $content;

my($file) = "attrs_tail.html";
open(FILE, "<$file") or die "cannot open $file for reading : $!";
$content = <FILE>;
$content =~ s/Last modified: [^\n]*\n/Last modified: $date/;
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
    print BYPROG "\t<br><strong><a href=\"$prog.html\" $top>$prog</a></strong> <font face=\"helvetica,arial\" size=\"2\"><br>\n";
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
