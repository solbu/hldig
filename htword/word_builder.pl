#
# word_builder.pl
#
# word_builder: Parses an ascii file and generate files from
#               templates (file.h.tmpl > file.h) according to the specifications.
#
# The format of the ascii file is as follows:
# 
# Lines starting with a # ignored
# All instructions enclosed between 
# Key and end, alone on a line. 
# The Definition section is made of sub sections whose name
# is the name of a field. Each sub section may contain two
# variables 
#           type = (String|C type)
#           bits = number of bits used (except for String)
# The EncodingOrder specifies the order in which the fields will be
# stored on disk. 
# The SortOrder specifies the order in which they will be sorted. Each
# field name may be followed by asc for ascending order or desc for
# descending order (think SQL).
# 
# There must be exactly one field of type String, its name must
# be Word, it must be last in EncodingOrder and first in SortOrder.
# All this is designed for inverted index and needs a word somewhere.
#
# Example file:
#
# Key 
#	Definition
#		Word
#			type 	= String
#		end
#		DocID
#			type	= unsigned int
#			bits	= 32
#		end
#		Location
#			type	= unsigned int
#			bits	= 16
#		end
#		Flags
#			type	= unsigned int
#			bits	= 8
#		end
#	end
#
#	EncodingOrder 		= Location,Flags,DocID,Word
#
#	SortOrder 		= Word asc,DocID asc,Flags asc,Location asc
#end
#
# This specification is parsed and stored in an internal structure described
# in the 'sub prepare' function. 
#
# Each file passed to the 'handle' function in the main is treated in the folowing
# way: 
# The text appearing between
#             #if TEMPLATE_DATA
# and
#             #endif /* TEMPLATE_DATA */
# is evaled and must return a sub whose prototype is my($file, $template, $assoc, $config) = @_;
# $file is the name of the file
# $template is a template obtained from the template_parse function with the content of the
#     file
# $assoc is the list of tags found in the file
# $config is the internal structure built by prepare
#
# the sub must fill the tags. When the sub returns, the $template is used to generate
# the file by removing the trailing .tmpl.
#
# Part of the ht://Dig package   <http://www.htdig.org/>
# Copyright (c) 1999 The ht://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU Public License version 2 or later
# <http://www.gnu.org/copyleft/gpl.html>
#
# $Id: word_builder.pl,v 1.2.2.2 1999/10/25 13:11:21 bosc Exp $
#
use strict;
use constant MAX_NFIELDS => 20;

use Carp;
use Cwd;

sub main {
    my($file) = @_;

    my($config) = config_load($file);

    prepare($config->{'Key'});
    handle("WordKey.h", $config->{'Key'});
}

#
# Catenate $prefix and $field in upper case
# Example field2define("DEFINED_", "Word") => "DEFINED_WORD"
#
sub field2define {
    my($prefix, $field) = @_;
    
    return $prefix . uc($field);
}

#
# Prepare file template <$file>.tmpl for substitution by
# <$function>. When done template_build is called and
# the result written to <$file>.
#
sub handle {
    my($file, @args) = @_;

    my($content) = readfile("$file.tmpl");
    $content =~ s|\#if\s+TEMPLATE_DATA\s*\n(.*)\#endif\s+/\*\s+TEMPLATE_DATA\s+\*/\n||s;
    my($function) = $1;
    eval "\$function = $function";
    croak($@) if($@);
    my($template) = template_parse($file, $content);
    my($assoc) = $template->{'assoc'};
    my($warning) = "// WARNING : this file was generated from $file.tmpl\n// by word_builder.pl using instructions from word.desc";
    &$function($file, $template, $assoc, @args);
    my($content) = template_build($template);
    system("rm -f $file");
    writefile($file, "$warning\n$content");
    system("chmod -w $file");
}

#
#
# Calculate information describing the structure of the definition.
# Populate the Definition hash with it.
#
# In the Definition hash you find:
#
# minimum_length = the minimum length in bytes of
#                  a compacted representation
#                  of the definition.
#
# types = a hash that has one entry for each distinct type
#         found in the definition. The value is the number
#         of times this type occurs.
#
# typemap = a hash that maps each C type to a symbolic name
#
# nfields = total number of fields 
#
# max_nfields = maximum allowed for nfields
#
# For each field you find in Definition->field hash:
#
# type = the type of the field, after mapping
#
# ctype = the type of the field, before mapping
#
# bits = the number of bits in the field
#
# bits_offset = absolute bit offset of the field
#
# bytes_offset = absolute offset of the byte containing
#                the first bit of the field.
#
# lowbits = the offset of the first bit to read, if
#           the beginning of the field is not on a
#           byte boundary.
#
# lastbits = the number of bits in
#            the last byte containing bits for the field
#            if the end of the field is not byte aligned.
#
# aligned = set if start and end of field is byte aligned
#
# aligned_start = set if start of field is byte aligned
#
# aligned_end = set if end of field is byte aligned
#
# bytesize = number of bytes to read starting from 
#            bytes_offset to get all the bits of the field.
#
# object_size = number of bytes occupied by the object
#               when in memory.
#
# Here are some examples:
#
# The | shows a byte boundary, the : marks the end of the field
# if not on a byte boundary. The vertical bars preceeding the
# field name show the end of the bits used by the field, included.
# The .... show a String.
#
#    |<DocID (5 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 0, byte size = 1)
#    |
#    |       |<Flags (8 bits, lowbits = 5, lastbits = 5, bytesize = 2, bits_offset = 5, byte size = 2)
#    |       |
#    |       |               |<Location (16 bits, lowbits = 5, lastbits = 5, bytesize = 3, bits_offset = 13, byte size = 3)
#    |       |               |
#    |       |               |.............<Word
#    |       |               |.............
#____:__|____:__|_______|____:
#^       ^       ^       ^       ^       ^       ^       ^       
#0123456701234567012345670123456701234567012345670123456701234567
#
#  |<DocID (3 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 0, byte size = 1)
#  |
#  | |<Location (2 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 3, byte size = 1)
#  | |
#  | |       |<Flags (8 bits, lowbits = 5, lastbits = 5, bytesize = 2, bits_offset = 5, byte size = 2)
#  | |       |
#  | |       |.............<Word
#  | |       |.............
#__:_:__|____:
#^       ^       ^       ^       ^       ^       ^       ^       
#0123456701234567012345670123456701234567012345670123456701234567
#
#  |<DocID (3 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 0, byte size = 1)
#  |
#  |    |<Location (5 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 3, byte size = 1)
#  |    |
#  |    |       |<Flags (8 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 8, byte size = 1)
#  |    |       |
#  |    |       |.............<Word
#  |    |       |.............
#__:____|_______|
#^       ^       ^       ^       ^       ^       ^       ^       
#0123456701234567012345670123456701234567012345670123456701234567
#
#  |<DocID (3 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 0, byte size = 1)
#  |
#  |      |<Location (7 bits, lowbits = 3, lastbits = 2, bytesize = 2, bits_offset = 3, byte size = 2)
#  |      |
#  |      |        |<Flags (9 bits, lowbits = 2, lastbits = 3, bytesize = 2, bits_offset = 10, byte size = 2)
#  |      |        |
#  |      |        |.............<Word
#  |      |        |.............
#__:____|_:_____|__:
#^       ^       ^       ^       ^       ^       ^       ^       
#0123456701234567012345670123456701234567012345670123456701234567
#
#  |<DocID (3 bits, lowbits = 0, lastbits = 0, bytesize = 1, bits_offset = 0, byte size = 1)
#  |
#  |      |<Location (7 bits, lowbits = 3, lastbits = 2, bytesize = 2, bits_offset = 3, byte size = 2)
#  |      |
#  |      |        |<Flags (9 bits, lowbits = 2, lastbits = 3, bytesize = 2, bits_offset = 10, byte size = 2)
#  |      |        |
#  |      |        |            |<Foo1 (13 bits, lowbits = 3, lastbits = 0, bytesize = 2, bits_offset = 19, byte size = 2)
#  |      |        |            |
#  |      |        |            |        |<Foo2 (9 bits, lowbits = 0, lastbits = 1, bytesize = 2, bits_offset = 32, byte size = 2)
#  |      |        |            |        |
#  |      |        |            |        |.............<Word
#  |      |        |            |        |.............
#__:____|_:_____|__:____|_______|_______|:
#^       ^       ^       ^       ^       ^       ^       ^       
#0123456701234567012345670123456701234567012345670123456701234567
#
sub prepare {
    my($config, $verbose) = @_;

    my($tab) = '';
    print "
The | shows a byte boundary, the : marks the end of the field
if not on a byte boundary. The vertical bars preceeding the
field name show the end of the bits used by the field, included.
The .... show a String.

" if($verbose);

    my($symbolic_type_base) = "TypeA";
    my($position) = 0;
    #
    # Total bits offset of the field
    #
    my($bits_offset) = 0;
    my($field);
    my($definition) = $config->{'Definition'};
    $definition->{'typemap'}->{'String'} = 'String';
    foreach $field (split(',', $config->{'EncodingOrder'})) {
	my($spec) = $definition->{$field};
	my($new_bits_offset) = $bits_offset;
	if($spec->{'type'} ne 'String' &&
	   !defined($definition->{'typemap'}->{$spec->{'type'}})) {
	    $definition->{'typemap'}->{$spec->{'type'}} = $symbolic_type_base;
	    $symbolic_type_base++;
	}
	#
	# Remap type into typedef
	#
	$spec->{'ctype'} = $spec->{'type'};
	$spec->{'type'} = $definition->{'typemap'}->{$spec->{'type'}};

	$definition->{'types'}->{$spec->{'type'}}++;
	if($spec->{'type'} ne 'String') {
	    my($bits) = $spec->{'bits'};
	    $new_bits_offset = $bits_offset + $bits;
	    #
	    # Search on which byte is the first bit ($bits_offset) and on
	    # which byte is the last bit ($new_bits_offset - 1).
	    #
	    my($old_bytes_offset) = int($bits_offset / 8);
	    my($new_bytes_offset) = int(($new_bits_offset - 1) / 8);
	    #
	    # How many bytes to consider to get all the bits of the field,
	    # starting at $old_bytes_offset included.
	    #
	    my($bytesize) = $new_bytes_offset - $old_bytes_offset + 1;
	    my($lowbits) = $bits_offset % 8;
	    my($lastbits) = $new_bits_offset % 8;

	    $spec->{'aligned_start'} = ($bits_offset % 8) == 0; 
	    $spec->{'aligned_end'} = ($new_bits_offset % 8) == 0;
	    $spec->{'aligned'} = $spec->{'aligned_start'} && $spec->{'aligned_end'};
	    $spec->{'bytesize'} = $bytesize;
	    $spec->{'bits_offset'} = $bits_offset;
	    $spec->{'bytes_offset'} = $old_bytes_offset;
	    $spec->{'lowbits'} = $lowbits;
	    $spec->{'lastbits'} = $lastbits;
	    $spec->{'object_size'} = int(($spec->{'bits'} + 7) / 8);
	    $tab .= " " x ($spec->{'bits'} - 1) . "|";
	} else {
	    #
	    # The string always starts at a byte boundary.
	    #
	    $spec->{'bytes_offset'} = int(($bits_offset + 7) / 8);
	    $spec->{'bits_offset'} = $bits_offset;
	    $spec->{'bytesize'} = -1;
	    $spec->{'lowbits'} = -1;
	    $spec->{'lastbits'} = -1;
	    $spec->{'bits'} = -1;
	    $tab .= ".............";
	}
	$spec->{'position'} = $position;

	print "$tab<$field" if($verbose);
	if($spec->{'type'} ne 'String') {
	    print " ($spec->{'bits'} bits, lowbits = $spec->{'lowbits'}, lastbits = $spec->{'lastbits'}, bytesize = $spec->{'bytesize'}, bits_offset = $spec->{'bits_offset'}, byte size = $spec->{'bytesize'})" if($verbose);
	}
	print "\n$tab\n" if($verbose);
	
	if($spec->{'type'} ne 'String') {
	    $bits_offset = $new_bits_offset;
	} else {
	    $config->{'Definition'}->{'minimum_length'} = $spec->{'bytes_offset'};
	}
	$position++;
    }
    $config->{'Definition'}->{'minimum_length'} = int(($bits_offset + 7) / 8) if(!$config->{'Definition'}->{'minimum_length'});

    $config->{'Definition'}->{'max_nfields'} = MAX_NFIELDS;

    $config->{'Definition'}->{'nfields'} = $position;

    #
    # Sanity checks
    #
    {
	if($config->{'Definition'}->{'nfields'} >
	   $config->{'Definition'}->{'max_nfields'} ) 
	{
	    print STDERR "nfields ($config->{'Definition'}->{'nfields'}) is greater than max_nfields ($config->{'Definition'}->{'max_nfields'})\n";
	    exit(1);
	}

	if($config->{'SortOrder'} !~ /^Word\s+asc/i)
	{
	    print STDERR "SortOrder of Word field must be *ascending*\n";
	    exit(1);
	}
	if($config->{'Definition'}->{'types'}->{'String'} != 1) {
	    print STDERR "There must exactly one field of type String\n";
	    exit(1);
	}
	if(!defined($config->{'Definition'}->{'Word'})) {
	    print STDERR "There must be one field named Word\n";
	    exit(1);
	}
	if(!defined($config->{'Definition'}->{'Word'}->{'type'})) {
	    print STDERR "The Word field must be of type String\n";
	    exit(1);
	}
    }
    if($verbose) {
	foreach $field (split(',', $config->{'EncodingOrder'})) {
	    my($spec) = $config->{'Definition'}->{$field};

	    next if($spec->{'type'} eq 'String');

	    my($bits) = $spec->{'bits'};
	    if($spec->{'lowbits'} > 0) {
		print "_" x ((8 - $spec->{'lowbits'}) - 1) . "|";;
		$bits -= 8 - $spec->{'lowbits'};
	    }
	    if($spec->{'bytesize'} > 1) {
		my($bytes) = $bits / 8;
		print "_______|" x $bytes;
	    } else {
		print "_" x ($bits - 1) . (($spec->{'bits_offset'} + $spec->{'bits'}) % 8 ? ":" : "|");
	    }

	    if($spec->{'lastbits'} > 0) {
		print "_" x ($spec->{'lastbits'} - 1) . (($spec->{'bits_offset'} + $spec->{'bits'}) % 8 ? ":" : "|");
	    }
	}
	print "\n^       ^       ^       ^       ^       ^       ^       ^       \n";
	print "0123456701234567012345670123456701234567012345670123456701234567\n";
    }
}

#
# General purpose utilities
#
sub readfile {
    my($file) = @_;

    open(FILE, "<$file") or croak("cannot open $file for reading : $!");
    local($/);
    undef($/);
    my($content);
    $content = <FILE>;
    close(FILE);
    return $content;
}

sub writefile {
    my($file, $content) = @_;

    open(FILE, ">$file") or croak("cannot open $file for writing : $!");
    print FILE $content;
    close(FILE);
}

#
# Given a path, relative or absolute, always return an absolute path
#
sub absolute_path {
    my($path) = @_;

    if($path =~ m;^/;) {
	return $path;
    } else {
	return getcwd() . "/$path";
    }
}

sub locate_file {
    my($file, $path) = @_;

    return $file if($file =~ m|^/|);

    my($found);

    my($dirs) = join(":", '.', ($path || ()), ($ENV{'DOCUMENT_ROOT'} || ()));
    my($dir);
    foreach $dir (split(':', $dirs)) {
	my($try) = "$dir/$file";
	if(-f $try) {
	    $found = $try;
	    last;
	}
    }

    return $found;
}

sub config_load {
    my($base) = @_;

    my($file) = locate_file($base, $ENV{'CONFIG_DIR'});
    $file = getcwd() . "/$file" if(defined($file) && $file !~ /^\//);
    if(!defined($file)) {
	print STDERR "no config file found in path " . ($ENV{'CONFIG_DIR'} || '') . " for $base";
    } elsif(!defined($::config{$file})) {
	my(%config);
	open(FILE, "<$file") or croak("cannot open $file for reading : $!");
	config_load_parse(\%config);
	close(FILE);
	$::config{$file} = \%config;
    }
    return defined($file) ? $::config{$file} : undef;
}

sub config_load_parse {
    my($config) = @_;

    while(<FILE>) {
	next if(/^\s*\#/o || /^\s*$/o);
	return if(/^\s*end\s*/io);
	if(/^\s*([\w_-]+)\s*$/) {
	    my($name) = $1;
	    $config->{$name} = {};
	    config_load_parse($config->{$name});
	} elsif(/^\s*([\S*]+?)\s*=\s*(.*?)\s*$/o) {
	    $config->{$1} = $2;
	}
    }
}

sub template_load {
    my($base, $defaults, $context) = @_;

    #
    # Change the base according to context, if any
    #
    if(defined($context)) {
	my($config) = config_load("templates.conf");
	my($style) = $config->{'style'};
	if(defined($style)) {
	    my($spec) = $style->{$context};
	    if(defined($spec) && exists($spec->{$base})) {
		$base = $spec->{$base};
	    }
	}
    }

    my($file) = template_file($base);

    if(!defined($file)) {
	return defined($defaults) ? $defaults->{$base} : undef;
    }

    #
    # Read in the whole file
    #
    my($content) = readfile($file);

    return template_parse($file, $content);
}

sub template_set {
    my($assoc, $key, $value) = @_;

    $assoc->{$key} = $value if(exists($assoc->{$key}));
}

sub template_parse {
    my($file, $content, $name) = @_;

    #
    # Extract subtemplates if any
    #
    my(%children);
    my @depth;
    while($content =~ /(<\!--\s*(start|end)\s+(\w+)\s*-->)/iog) {
	my($se,$subname) = ($2,$3);
	my $end = pos($content);
	my $len = length($1);
	if($se eq 'start') {
	    push(@depth,[$subname,$end,$len]);
	    next;
	}
	elsif(@depth) {
	    croak("template $file: missing end for $subname")
		unless ($subname eq $depth[-1]->[0]);
	    if(@depth > 1) {
	        pop @depth;
		next;
	    }
	}
	else {
	    croak("template $file: unexpected end for $subname");
	}
	my $start = $depth[0]->[1];
	my $sublen = $end - $len - $start;
	$children{$subname} = template_parse($file,
	            substr($content,$start,$sublen), $subname);
	my($tag) = '_SUBTEMPLATE' . $subname . '_';
	$start -=  $depth[0]->[2];
	substr($content,$start,$end-$start) = $tag;
	pos($content) = $start + length($tag);
	@depth = ();
    }

    #
    # Extract parameters if any
    #
    my(%params);
    my($params_re) = '<\!--\s*params\s+(.*?)\s*-->';
    if($content =~ /$params_re/io) {
	eval "package; \%params = ( $1 )";
	croak $@ if $@;	# or just warn? what's the policy?
    }
    
    #
    # Extract tag list
    #
    my(%assoc);
    while($content =~ /(?<![A-Z])(_[0-9A-Z_-]+_)(?![A-Z])/g) {
	my($tag) = $1;
	next if($tag =~ /^_SUBTEMPLATE/);
	$assoc{$tag} = undef;
    }
    
    return {
	'content' => $content,
	'assoc' => \%assoc,
	'children' => \%children,
	'params' => \%params,
	'filename' => $file,
	'name' => $name || 'whole',
    };
}

#
# The caller is expected to alter the structure returned
# by template_load in the following way:
#
# . put values in the tags specified in {'assoc'}
# . set the {'skip'} field if nothing is to be done
# . set the {'text'} field to replace the {'content'} before
#   tags replacement
#
# When template_build returns the structure has been restored to
# its old state, except for the skipped entries.
#
sub template_build {
    my($template) = @_;

    my($content) = template_fill(@_);
    template_clean(@_);

    my($include_root) = $ENV{'DOCUMENT_ROOT'} || '/etc/httpd/htdocs';
    
    #
    # Handle server includes directives recursively
    #
    while($content =~ /(<\!--\#include\s+virtual\s*=\s*\"[^\"]*\"-->)/i) {
	my($include) = $1;
	my($matched) = quotemeta($include);
	my($file) = $include =~ /virtual\s*=\s*\"([^\"]*)/;
	my($path) = "$include_root$file";
	my($included) = readfile($path);
	$content =~ s/$matched/$included/;
    }

    return $content;
}

sub template_fill {
    my($template, $parents) = @_;

    return "" if($template->{'skip'});

    if ($template->{params}->{pre_fill}) {
	my $sub = $template->{params}->{pre_fill};
	$template = eval { package ; &$sub($template, $parents||[]) };
	croak $@ if $@;	# or just warn? what's the policy?
	return "" if $template->{skip};
    }

    my($children) = $template->{'children'};
    my($assoc)    = $template->{'assoc'};
    my($text)     = defined($template->{'text'})
			? $template->{'text'}
			: $template->{'content'};

    while (my ($name,$value) = each %$children) {
	my($tag) = '_SUBTEMPLATE' . $name . '_';
	push @{ $parents ||= [] }, $template;
	my($sub_text) = template_fill($value, $parents);
	$text =~ s/$tag/$sub_text/g;
    }

    while (my ($key,$value) = each %$assoc) {
	$value = '' unless defined $value;
	$text =~ s/(?<=\b)$key(?=\b)/$value/g;
    }

    if ($template->{params}->{post_fill}) {
	my $sub = $template->{params}->{post_fill};
	$text = eval { &$sub($template, $parents||[], $text) };
	croak $@ if $@;	# or just warn? what's the policy?
    }

    return $text;
}

sub template_clean {
    my($template) = @_;

    return if(exists($template->{'noclean'}));

    delete($template->{'skip'});
    delete($template->{'text'});

    my($children) = $template->{'children'};
    foreach my $child (values %$children) {
	template_clean($child);
    }

    my($assoc) = $template->{'assoc'};
    foreach my $key (keys %$assoc) {
	$assoc->{$key} = undef;
    }
}

sub template_file {
    my($file) = @_;

    return locate_file($file, $ENV{'TEMPLATESDIR'});
}

sub template_exists {
    my($file) = @_;

    return -r template_file($file);
}


sub try {
    my($file);
    foreach $file (qw(word.try1 word.try2 word.try3 word.try4 word.try5)) {
	my($config) = config_load($file);
	print "\n========================== $file =============================\n";
	prepare($config->{'Key'}, 'verbose');
    }
}

if($ARGV[0] eq 'test') {
    try();
} else {
    main($ARGV[0]);
}
