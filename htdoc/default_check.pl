open(FILE, "<../htcommon/defaults.cc");
while(<FILE>) {
    chop;
    if(/^\s+{\"(\w+)\",\s+(.*)},\s*$/) {
	my($attr, $default) = ($1, $2);
	$default =~ s/^"(.*)"$/$1/;
	$defaults{$attr} = $default;
    }
}
close(FILE);

local($/) = undef;

open(FILE, "<attrs.html");
$_ = <FILE>;
close(FILE);

s|.*Alphabetical list of attributes.*?<hr>||s;

while(s/(.*?)<hr>//s) {
    my($entry) = $1;

    if(!($entry =~ s|<dl>.*?<a\s+name=\"(\w+)\".*?dt>||s)) {
	print STDERR "No attr name found in $entry\n";
	next;
    }
    my($attr) = $1;

    if(!($entry =~ s|.*?type:.*?<dd>\s+(\w+)||s)) {
	print STDERR "No type name found in $entry\n";
	next;
    }
    my($type) = $1;

    if(!($entry =~ s|.*?used by:.*?<dd>\s+(.*?)\s+</dd>||s)) {
	print STDERR "No used by found in $entry\n";
	next;
    }
    my(@used_by);
    my($used_by) = $1;
    while($used_by =~ m|href=\"(\w+).html\"|sg) {
	push(@used_by, $1);
    }
    $used_by = join(' ', @used_by);

    if(!($entry =~ s|.*?default:.*?<dd>\s+(.*?)\s+</dd>||s)) {
	print STDERR "No default found in $entry\n";
	next;
    }
    my($default) = $1;
    $default =~ s/&lt;/</gs;
    $default =~ s/&gt;/>/gs;
    $default =~ s/&amp;/&/gs;
    $default =~ s/<br>/ /gs;
    $default =~ s/\s+/ /gs;
    $default = "" if($default =~ m/<empty>/);

    if(!($entry =~ s|.*?descriptions?:.*?<dd>\s+(.*?)\s+</dd>||s)) {
	print STDERR "No description found in $entry\n";
	next;
    }
    my($description) = $1;
#    $description =~ s/&lt;/</gs;
#    $description =~ s/&gt;/>/gs;
#    $description =~ s/&amp;/&/gs;
#    $description =~ s/<br>/ /gs;
#    $description =~ s:(<em>|</em>|<strong>|</strong>|<i>|</i>|<b>|</b>|<tt>|</tt>):\*:gs;
#    $description =~ s:(<p>|</p>)::gs;
#    $description =~ s|<a\s+href="#(\w+)">.*?</a>|\`\`$1\'\'|gs;
#    $description =~ s:<a\s+href="(htfuzzy|htmerge|htdig|htsearch).html".*?</a>:$1:gs;
    $description =~ s/\A\s*/\t/gm;
    $description =~ s/^\s+/\t/gm;
    $description =~ s/"/\\"/gs;
    
    if(!($entry =~ s|.*?example:.*?($attr:.*?)\n||s)) {
	print STDERR "No example found in $entry\n";
	next;
    }
    my($example) = $1;
    $example =~ s/"/\\"/gs;

    if(!exists($defaults{$attr})) {
	print STDERR "Not found in defaults.cc\n";
    }
    if($defaults{$attr} ne $default) {
	print STDERR "Default value mismatch\n\tC++: $defaults{$attr}\n\thtml: $default\n";	
    }
    $defs{$attr} = [ "$defaults{$attr}", $type, $used_by, $example, $description ] if($attr ne 'include');
    print STDERR "attr = $attr, type = $type, used_by = $used_by, default = $default, example = $example\n$description\n\n";
    delete($defaults{$attr});
}

foreach $attr (keys(%defaults)) {
    $defs{$attr} = [ $defaults{$attr}, "", "", "" ];
    print STDERR "$attr was not described in .html\n";
}

foreach $attr (sort(keys(%defs))) {
    my($s) = $defs{$attr};
    print <<EOF;
{ "$attr", "$s->[0]", 
	"$s->[1]", "$s->[2]", "$s->[3]", "\n$s->[4]\n" },
EOF
}
