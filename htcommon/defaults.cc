//
// defaults.cc
//
// defaults: default values for the ht programs through the
//           HtConfiguration class
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: defaults.cc,v 1.96 2003/10/23 02:10:55 angusgb Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtConfiguration.h"

// Fields and their values:
//	Attribute name
//	Default value ("" becomes "no default" in .html docs)
//	Type (boolean, number, integer, string, string list, quoted string list,
//				pattern list)
//	Commands using attribute (all, htdig, htsearch, htfuzzy,
//				htdump, htload, htnotify, htpurge)
//	Block (Global, Server, URL)
//	Versions for which attribute is present
//	Class	(Extra Output, External:Parsers, External:Protocols,
//		File Layout,
//		Indexing:Connection, Indexing:Out, Indexing:What,Indexing:Where,
//		Presentation:Files, Presentation:How, Presentation:Text,
//		Searching:Method, Searching:Ranking, Searching:UI,
//		URLs)
//	Example
//	Description

ConfigDefaults	defaults[] =
{

{ "accents_db", "${database_base}.accents.db",  \
	"string", "htfuzzy htsearch", "", "all", "File Layout", "accents_db: ${database_base}.uml.db", " \
	The database file used for the fuzzy \"accents\" search \
	algorithm. This database is created by \
	<a href=\"htfuzzy.html\">htfuzzy</a> and used by \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>. \
" }, \
{ "accept_language", "",  \
	"string list", "htdig", "Server", "3.2.0b4", "Indexing:Out", "accept_language: en-us en it", " \
	This attribute allows you to restrict the set of natural languages \
	that are preferred as a response to an HTTP request performed by the \
	digger. This can be done by putting one or more language tags \
	(as defined by RFC 1766) in the preferred order, separated by spaces. \
	By doing this, when the server performs a content negotiation based \
	on the 'accept-language' given by the HTTP user agent, a different \
	content can be shown depending on the value of this attribute. If \
	set to an empty list, no language will be sent and the server default \
	will be returned. \
" }, \
{ "add_anchors_to_excerpt", "true",  \
	"boolean", "htsearch", "", "3.1.0", "Presentation:How", "add_anchors_to_excerpt: no", " \
	If set to true, the first occurrence of each matched \
	word in the excerpt will be linked to the closest \
	anchor in the document. This only has effect if the \
	<strong>EXCERPT</strong> variable is used in the output \
	template and the excerpt is actually going to be displayed. \
" }, \
{ "allow_double_slash", "false",  \
	"boolean", "htdig", "", "3.2.0b4", "Indexing:Out", "allow_double_slash: true", " \
	If set to true, strings of multiple slashes ('/') in URL paths \
	will be left intact, rather than being collapsed. This is necessary \
	for some search engine URLs which use slashes to separate fields rather \
	than to separate directory components.  However, it can lead to multiple database \
	entries refering to the same file, and it causes '/foo//../' to \
	be equivalent to '/foo/', rather than to '/'. \
" }, \
{ "allow_in_form", "",  \
	"string list", "htsearch", "", "3.1.0", "Searching:UI", "allow_in_form: search_algorithm search_results_header", " \
	Allows the specified config file attributes to be specified \
	in search forms as separate fields. This could be used to \
	allow form writers to design their own headers and footers \
	and specify them in the search form. Another example would \
	be to offer a menu of search_algorithms in the form. \
	<table> \
	<tr> \
	<td nowrap> \
	<code> \
	&nbsp;&nbsp;&lt;SELECT NAME=\"search_algorithm\"&gt;<br> \
	&nbsp;&nbsp;&lt;OPTION VALUE=\"exact:1 prefix:0.6 synonyms:0.5 endings:0.1\" SELECTED&gt;fuzzy<br> \
	&nbsp;&nbsp;&lt;OPTION VALUE=\"exact:1\"&gt;exact<br> \
	&nbsp;&nbsp;&lt;/SELECT&gt; \
	</code></td> \
	</tr> \
	</table> \
	The general idea behind this is to make an input parameter out \
	of any configuration attribute that's not already automatically \
	handled by an input parameter. You can even make up your own \
	configuration attribute names, for purposes of passing data from \
	the search form to the results output. You're not restricted to \
	the existing attribute names. The attributes listed in the \
	allow_in_form list will be settable in the search form using \
	input parameters of the same name, and will be propagated to \
	the follow-up search form in the results template using template \
	variables of the same name in upper-case. \
	You can also make select lists out of any of these input \
	parameters, in the follow-up search form, using the \
	<a href=\"#build_select_lists\">build_select_lists</a> \
	configuration attribute. \
	<br>WARNING: Extreme care are should be taken with this option, as \
	allowing CGI scripts to set file names can open security holes.\
" }, \
{ "allow_numbers", "false",  \
	"boolean", "htdig htsearch", "", "all", "Indexing:What", "allow_numbers: true", " \
	If set to true, numbers are considered words. This \
	means that searches can be done on strings of digits as well as \
	regular words. All the same rules apply to numbers as \
	to words.  This does not cause numbers containing a decimal point or \
	commas to be treated as a single entity. \
	When allow_numbers is false, words are stil \
	allowed to contain digits, but they must also contain at \
	least one alphabetic character or \
	<a href=\"#extra_word_characters\">extra word</a> character. \
	To disallow digits in words, add the digits to \
	<a href=\"#valid_punctuation\">valid_punctuation</a>. \
" }, \
{ "allow_virtual_hosts", "true",  \
	"boolean", "htdig", "", "3.0.8b2", "Indexing:Where", "allow_virtual_hosts: false", " \
	If set to true, htdig will index virtual web sites as \
	expected. If false, all URL host names will be \
	normalized into whatever the DNS server claims the IP \
	address to map to. If this option is set to false, \
	there is no way to index either \"soft\" or \"hard\" \
	virtual web sites. \
" }, \
{ "anchor_target", "",  \
	"string", "htsearch", "", "3.1.6", "Presentation:How", "anchor_target: body", " \
	When the first matched word in the excerpt is linked \
	to the closest anchor in the document, this string \
	can be set to specify a target in the link so the \
	resulting page is displayed in the desired frame. \
	This value will only be used if the \
	<a href=\"#add_anchors_to_excerpt\">add_anchors_to_excerpt</a> \
	attribute is set to true, the <strong>EXCERPT</strong> \
	variable is used in the output template and the \
	excerpt is actually displayed with a link. \
" }, \
{ "any_keywords", "false",  \
	"boolean", "htsearch", "", "3.2.0b2", "Searching:Method", "any_keywords: yes", " \
	If set to true, the words in the <strong>keywords</strong> \
	input parameter in the search form will be joined with logical \
	ORs rather than ANDs, so that any of the words provided will do. \
	Note that this has nothing to do with limiting the search to \
	words in META keywords tags. See the <a href=\"hts_form.html\"> \
	search form</a> documentation for details on this. \
" }, \
{ "author_factor", "1",  \
	"number", "htsearch", "", "3.2.0b4", "Searching:Ranking", "author_factor: 1", " \
	Weighting applied to words in a &lt;meta name=\"author\" ... &gt; \
	tag.<br> \
	See also <a href=\"#heading_factor\">heading_factor</a>. \
" }, \
{ "authorization", "",  \
	"string", "htdig", "URL", "3.1.4", "Indexing:Out", "authorization: myusername:mypassword", " \
	This tells htdig to send the supplied \
	<em>username</em><strong>:</strong><em>password</em> with each HTTP request. \
	The credentials will be encoded using the \"Basic\" authentication \
	scheme. There <em>must</em> be a colon (:) between the username and \
	password.<br> \
	This attribute can also be specified on htdig's command line using \
	the -u option, and will be blotted out so it won't show up in a \
	process listing. If you use it directly in a configuration file, \
	be sure to protect it so it is readable only by you, and do not \
	use that same configuration file for htsearch. \
" }, \
{ "backlink_factor", "1000",  \
	"number", "htsearch", "", "3.1.0", "Searching:Ranking", "backlink_factor: 501.1", " \
	This is a weight of \"how important\" a page is, based on \
	the number of URLs pointing to it. It's actually \
	multiplied by the ratio of the incoming URLs (backlinks) \
	and outgoing URLs (links on the page), to balance out pages \
	with lots of links to pages that link back to them. The ratio \
	gives lower weight to \"link farms\", which often have many \
	links to them.  This factor can \
	be changed without changing the database in any way. \
	However, setting this value to something other than 0 \
	incurs a slowdown on search results. \
" }, \
{ "bad_extensions", ".wav .gz .z .sit .au .zip .tar .hqx .exe .com .gif .jpg .jpeg .aiff .class .map .ram .tgz .bin .rpm .mpg .mov .avi .css",  \
	"string list", "htdig", "URL", "all", "Indexing:Where", "bad_extensions: .foo .bar .bad", " \
	This is a list of extensions on URLs which are \
	considered non-parsable. This list is used mainly to \
	supplement the MIME-types that the HTTP server provides \
	with documents. Some HTTP servers do not have a correct \
	list of MIME-types and so can advertise certain \
	documents as text while they are some binary format. \
	If the list is empty, then all extensions are acceptable, \
	provided they pass other criteria for acceptance or rejection. \
	See also <a href=\"#valid_extensions\">valid_extensions</a>. \
" }, \
{ "bad_local_extensions", ".php .shtml .cgi",  \
	"string list", "htdig", "URL", "all", "Indexing:Where", "bad_extensions: .foo .bar .bad", " \
	This is a list of extensions on URLs which must be retrieved \
	using the URL's true transport mechanism (such as HTTP). \
	If <a href=\"#local_urls\">local_urls</a> is specified, URLs not \
	ending with these extensions may instead be retrieved through \
	the local filesystem for efficiency. \
" },
{ "bad_querystr", "",  \
	"pattern list", "htdig", "URL", "3.1.0", "Indexing:Where", "bad_querystr: forum=private section=topsecret&amp;passwd=required", " \
	This is a list of CGI query strings to be excluded from \
	indexing. This can be used in conjunction with CGI-generated \
	portions of a website to control which pages are \
	indexed. \
" }, \
{ "bad_word_list", "${common_dir}/bad_words",  \
	"string", "htdig htsearch", "", "all", "Indexing:What,Searching:Method", "bad_word_list: ${common_dir}/badwords.txt", " \
	This specifies a file which contains words which should \
	be excluded when digging or searching. This list should \
	include the most common words or other words that you \
	don't want to be able to search on (things like <em> \
	sex</em> or <em>smut</em> are examples of these.)<br> \
	The file should contain one word per line. A sample \
	bad words file is located in the <code>contrib/examples</code> \
	directory. \
" }, \
{ "bin_dir", BIN_DIR,  \
	"string", "all", "", "all", "File Layout", "bin_dir: /usr/local/bin", " \
	This is the directory in which the executables \
	related to ht://Dig are installed. It is never used \
	directly by any of the programs, but other attributes \
	can be defined in terms of this one. \
	<p> \
	The default value of this attribute is determined at \
	compile time. \
	</p> \
" }, \
{ "boolean_keywords", "and or not",  \
	"string list", "htsearch", "", "3.1.6", "Presentation:How", "boolean_keywords: et ou non", " \
	These three strings are used as the keywords used in \
	constructing the LOGICAL_WORDS template variable, \
	and in parsing the <a href=\"hts_form.html#words\">words</a> input \
	parameter when the <a href=\"hts_form.html#method\">method</a> \
	parameter or <a href=\"#match_method\">match_method</a> attribute \
	is set to <code>boolean</code>. \
	See also the \
	<a href=\"boolean_syntax_errors\">boolean_syntax_errors</a> attribute. \
" },
{ "boolean_syntax_errors", ">Expected \
	'a search word, a quoted phrase or a boolean expression between ()' \
		'at the end' 'instead of' 'end of expression' quotes",  \
	"quoted string list", "htsearch", "", "3.1.6", "Presentation:How",
	"boolean_syntax_errors: Attendait \"un mot\" \"&agrave; la fin\" \
	\"au lieu de\" \"fin d'expression\" \"guillemet\"", " \
	These six strings are used as the keywords used to \
	construct various syntax error messages for errors encountered in \
	parsing the <a href=\"hts_form.html#words\">words</a> input \
	parameter when the <a href=\"hts_form.html#method\">method</a> parameter \
	or <a href=\"#match_method\">match_method</a> attribute \
	is set to <code>boolean</code>. \
	They are used in conjunction with the \
	<a href=\"#boolean_keywords\">boolean_keywords</a> attribute, and \
	comprise all \
	English-specific parts of these error messages.  The order in which \
	the strings are put together may not be ideal, or even gramatically \
	correct, for all languages, but they can be used to make fairly \
	intelligible messages in many languages. \
" },
{ "build_select_lists", "",  \
	"quoted string list", "htsearch", "", "3.2.0b1", "Searching:UI", "build_select_lists: \
		MATCH_LIST matchesperpage matches_per_page_list \\<br> \
				1 1 1 matches_per_page \"Previous Amount\" \\<br> \
		RESTRICT_LIST,multiple restrict restrict_names 2 1 2 restrict \"\" \\<br> \
		FORMAT_LIST,radio format template_map 3 2 1 template_name \"\"", " \
	This list allows you to define any htsearch input parameter as \
	a select list for use in templates, provided you also define \
	the corresponding name list attribute which enumerates all the \
	choices to put in the list. It can be used for existing input \
	parameters, as well as any you define using the \
	<a href=\"#allow_in_form\">allow_in_form</a> \
	attribute. The entries in this list each consist of an octuple, \
	a set of eight strings defining the variables and how they are to \
	be used to build a select list. The attribute can contain many \
	of these octuples. The strings in the string list are merely \
	taken eight at a time. For each octuple of strings specified in \
	build_select_lists, the elements have the following meaning:  \
	<ol> \
	   <li>the name of the template variable to be defined as a list, \
	   optionally followed by a comma and the type of list, and \
	   optional formatting codes \
	   <li>the input parameter name that the select list will set  \
	   <li>the name of the user-defined attribute containing the \
	   name list \
	   <li>the tuple size used in the name list above  \
	   <li>the index into a name list tuple for the value  \
	   <li>the index for the corresponding label on the selector \
	   <li>the configuration attribute where the default value for \
	   this input parameter is defined \
	   <li>the default label, if not an empty string, which will be \
	   used as the label for an additional list item for the current \
	   input parameter value if it doesn't match any value in the \
	   given list \
	</ol> \
	See the <a href=\"hts_selectors.html\">select list documentation</a> \
	for more information on this attribute. \
" }, \
{ "caps_factor", "1",  \
	"number", "htsearch", "", "??", "Searching:Ranking", "caps_factor: 1", " \
	TO BE COMPLETED<br> \
	See also <a href=\"#heading_factor\">heading_factor</a>. \
" }, \
{ "case_sensitive", "true",  \
	"boolean", "htdig", "", "3.1.0b2", "Indexing:Where", "case_sensitive: false", " \
	This specifies whether ht://Dig should consider URLs \
	case-sensitive or not. If your server is case-insensitive, \
	you should probably set this to false. \
" }, \
{ "check_unique_date", "false",  \
	"boolean", "htdig", "Global", "3.2.0b3", "", "check_unique_date: false", " \
	Include the modification date of the page in the MD5 hash, to reduce the \
	problem with identical but physically separate pages in different parts of the tree pointing to \
	different pages.  \
" }, \
{ "check_unique_md5", "false",  \
	"boolean", "htdig", "Global", "3.2.0b3", "", "check_unique_md5: false", " \
	Uses the MD5 hash of pages to reject aliases, prevents multiple entries \
	in the index caused by such things as symbolic links \
	Note: May not do the right thing for incremental update \
" }, \
{ "collection_names", "", \
	"string list", "htsearch", "", "3.2.0b2", "", "collection_names: htdig_docs htdig_bugs", " \
	This is a list of config file names that are used for searching multiple databases. \
	Simply put, htsearch will loop through the databases specified by each of these config \
	files and present the result of the search on all of the databases. \
	The corresponding config files are looked up in the <a href=\"#config_dir\">config_dir</a> directory. \
	Each listed config file <strong>must</strong> exist, as well as the corresponding databases. \
" }, \
{ "common_dir", COMMON_DIR,  \
	"string", "all", "", "all", "File Layout", "common_dir: /tmp", " \
	Specifies the directory for files that will or can be \
	shared among different search databases. The default \
	value for this attribute is defined at compile time. \
" }, \
{ "common_url_parts", "http:// http://www. ftp:// ftp://ftp. /pub/ .html .htm .gif .jpg .jpeg /index.html /index.htm .com/ .com mailto:",  \
	"string list", "all", "", "3.1.0", "URLs", "common_url_parts: http://www.htdig.org/ml/ \\<br> \
.html \\<br> \
http://dev.htdig.org/ \\<br> \
http://www.htdig.org/", " \
	Sub-strings often found in URLs stored in the \
	database.  These are replaced in the database by an \
	internal space-saving encoding.  If a string \
	specified in <a href=\"#url_part_aliases\">url_part_aliases</a>, \
	overlaps any string in common_url_parts, the \
	common_url_parts string is ignored.<br> \
	Note that when this attribute is changed, the \
	database should be rebuilt, unless the effect of \
	\"changing\" the affected URLs in the database is \
	wanted.<br> \
" }, \
{ "compression_level", "6",  \
	"integer", "htdig", "", "3.1.0", "Indexing:How", "compression_level: 0", " \
	If non-zero and the \
	<a href=\"http://www.cdrom.com/pub/infozip/zlib/\">zlib</a> \
	compression library was available when compiled, \
	this attribute controls the amount of compression used in the \
	<a href=\"#doc_excerpt\">doc_excerpt</a> file. \
	<br/>This must be in the range 0-9, and must be non-zero when \
	<a href=\"#wordlist_compress_zlib\">wordlist_compress_zlib</a> \
	is used. \
" }, \
{ "config", "",  \
	"string", "all", "", "??", "File Layout", "", " \
	Name of configuration file to load. \
	For security reasons, restrictions are placed on the values which \
	can be specified on the command line to \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>. \
	The default value of this attribute is determined at \
	compile time. \
" }, \
{ "config_dir", CONFIG_DIR,  \
	"string", "all", "", "all", "File Layout", "config_dir: /var/htdig/conf", " \
	This is the directory which contains all configuration \
	files related to ht://Dig. It is never used \
	directly by any of the programs, but other attributes \
	or the <a href=\"#include\">include</a> directive \
	can be defined in terms of this one. \
	<p> \
	The default value of this attribute is determined at \
	compile time. \
	</p> \
" },
{ "content_classifier", "${bin_dir}/HtFileType",  \
	"string", "htdig", "", "3.2.0b4", "Indexing:What", "content_classifier: file -i -b", " \
	When ht://Dig can't determine the type of a <code>file://</code> \
	URL from its extension, this program is used to determine the type. \
	The program is called with one argument, the name of (possibly a \
	temporary copy of) the file. \
	<p> \
	See also <a href=\"#mime_types\">mime_types</a>.\
	</p> \
" }, \
{ "cookies_input_file", "",  \
	"string", "htdig", "", "3.2.0b4", "Indexing:Connection", "cookies_input_file: ${common_dir}/cookies.txt", " \
	Specifies the location of the file used for importing cookies \
	for the crawl. These cookies will be preloaded into htdig's \
	in-memory cookie jar, but aren't written back to the file. \
	Cookies are specified according to Netscape's format \
	(tab-separated fields). If this attribute is left blank, \
	no cookie file will be read. \
	For more information, see the sample cookies.txt file in the \
	ht://Dig source distribution. \
" }, \
{ "create_image_list", "false",  \
	"boolean", "htdig", "", "all", "Extra Output", "create_image_list: yes", " \
	If set to true, a file with all the image URLs that \
	were seen will be created, one URL per line. This list \
	will not be in any order and there will be lots of \
	duplicates, so after htdig has completed, it should be \
	piped through <code>sort -u</code> to get a unique list. \
" }, \
{ "create_url_list", "false",  \
	"boolean", "htdig", "", "all", "Extra Output", "create_url_list: yes", " \
	If set to true, a file with all the URLs that were seen \
	will be created, one URL per line. This list will not \
	be in any order and there will be lots of duplicates, \
	so after htdig has completed, it should be piped \
	through <code>sort -u</code> to get a unique list. \
" }, \
{ "database_base", "${database_dir}/db",  \
	"string", "all", "", "all", "File Layout", "database_base: ${database_dir}/sales", " \
	This is the common prefix for files that are specific \
	to a search database. Many different attributes use \
	this prefix to specify filenames. Several search \
	databases can share the same directory by just changing \
	this value for each of the databases. \
" }, \
{ "database_dir", DATABASE_DIR,  \
	"string", "all", "", "all", "File Layout", "database_dir: /var/htdig", " \
	This is the directory which contains all database and \
	other files related to ht://Dig. It is never used \
	directly by any of the programs, but other attributes \
	are defined in terms of this one. \
	<p> \
	The default value of this attribute is determined at \
	compile time. \
	</p> \
" }, \
{ "date_factor", "0",  \
	"number", "htsearch", "", "3.1.0", "Searching:Ranking", "date_factor: 0.35", " \
	This factor, gives higher \
	rankings to newer documents and lower rankings to older \
	documents. Before setting this factor, it's advised to \
	make sure your servers are returning accurate dates \
	(check the dates returned in the long format). \
	Additionally, setting this to a nonzero value incurs a \
	small performance hit on searching. \
" }, \
{ "date_format", "",  \
	"string", "htsearch", "", "3.1.2", "Presentation:How", "date_format: %Y-%m-%d", " \
	This format string determines the output format for \
	modification dates of documents in the search results. \
	It is interpreted by your system's <em>strftime</em> \
	function. Please refer to your system's manual page \
	for this function, for a description of available \
	format codes. If this format string is empty, as it \
	is by default,  \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a> \
	will pick a format itself. In this case, the <a \
	href=\"#iso_8601\">iso_8601</a> attribute can be used \
	to modify the appearance of the date. \
" }, \
{ "description_factor", "150",  \
	"number", "htsearch", "", "3.1.0b3", "Searching:Ranking", "description_factor: 350", " \
	Plain old \"descriptions\" are the text of a link pointing \
	to a document. This factor gives weight to the words of \
	these descriptions of the document. Not surprisingly, \
	these can be pretty accurate summaries of a document's \
	content. See also <a href=\"#heading_factor\">heading_factor</a> \
	and <a href=\"#meta_description_factor\">meta_description_factor</a>. \
" }, \
{ "description_meta_tag_names", "description",  \
	"string list", "htsearch", "", "3.1.6", "Searching:Ranking", "description_meta_tag_names: \"description htdig-description\"", " \
	The words in this list are used to search for descriptions in HTML \
	<em>META</em> tags. This list can contain any number of strings \
	that each will be seen as the name for whatever description \
	convention is used. While words in any of the specified \
	description contents will be indexed, only the last meta tag \
	containing a description will be kept as the meta description \
	field for the document, for use in search results. The order in \
	which the names are specified in this configuration attribute \
	is irrelevant, as it is the order in which the tags appear in \
	the documents that matters.<br> The <em>META</em> tags have the \
	following format:<br> \
	<tt> &nbsp;&nbsp;&lt;META name=\"<em>somename</em>\" \
	                       content=\"<em>somevalue</em>\"&gt; </tt><br> \
	See also <a href=\"#meta_description_factor\">meta_description_factor</a>. \
" }, \
{ "disable_cookies", "true",  \
	"boolean", "htdig", "Server", "3.2.0b4", "Indexing:Connection", "disable_cookies: true", " \
        This option, if set to true, will disable HTTP cookies. \
" }, \
{ "doc_db", "${database_base}.docdb",  \
	"string", "all", "", "all", "File Layout", "doc_db: ${database_base}documents.db", " \
	This file will contain a Berkeley database of documents \
	indexed by document number. It contains all the information \
	gathered for each document, except the document excerpts \
	which are stored in the <a href=\"#doc_excerpt\"><em> \
	doc_excerpt</em></a> file. \
" }, \
{ "doc_excerpt", "${database_base}.excerpts",  \
	"string", "all", "", "3.2.0b1", "File Layout", "doc_excerpt: ${database_base}excerpts.db", " \
	This file will contain a Berkeley database of document excerpts \
	indexed by document number. It contains all the text \
	gathered for each document, so this file can become \
	rather large if <a href=\"#max_head_length\"><em> \
	max_head_length</em></a> is set to a large value. \
	The size can be reduced by setting the \
	<a href=\"#compression_level\"><em>compression_level</em></a>, \
	if supported on your system. \
" }, \
{ "doc_index", "${database_base}.docs.index",  \
	"string", "htdig", "", "all", "File Layout", "doc_index: documents.index.db", " \
	This file contains a mapping of document numbers to URLs and is \
	used by htdig during indexing. It is used on updates if it exists. \
" }, \
{ "doc_list", "${database_base}.docs",  \
	"string", "htdig htdump htload", "", "all", "File Layout", "doc_list: /tmp/documents.text", " \
	This file is basically a text version of the file \
	specified in <em><a href=\"#doc_db\">doc_db</a></em>. Its \
	only use is to have a human readable database of all \
	documents. The file is easy to parse with tools like \
	perl or tcl. \
" }, \
{ "endday", "",  \
	"integer", "htsearch", "", "3.1.6", "Searching:Method", "endday: 31", " \
	Day component of last date allowed as last-modified date \
	of returned docutments. \
	This is most usefully specified as a \
	<a href=\"hts_form.html#startyear\">GCI argument</a>. \
	See also <a href=\"#startyear\">startyear</a>. \
" }, \
{ "end_ellipses", "<strong><code> ...</code></strong>",  \
	"string", "htsearch", "", "all", "Presentation:Text", "end_ellipses: ...", " \
	When excerpts are displayed in the search output, this \
	string will be appended to the excerpt if there is text \
	following the text displayed. This is just a visual \
	reminder to the user that the excerpt is only part of \
	the complete document. \
" }, \
{ "end_highlight", "</strong>",  \
	"string", "htsearch", "", "3.1.4", "Presentation:Text", "end_highlight: &lt;/font&gt;", " \
	When excerpts are displayed in the search output, matched \
	words will be highlighted using <a href=\"#start_highlight\"> \
	start_highlight</a> and this string. \
	You should ensure that highlighting tags are balanced, \
	that is, this string should close any formatting \
	tag opened by start_highlight. \
" }, \
{ "endings_affix_file", "${common_dir}/english.aff",  \
	"string", "htfuzzy", "", "all", "File Layout", "endings_affix_file: /var/htdig/affix_rules", " \
	Specifies the location of the file which contains the \
	affix rules used to create the endings search algorithm \
	databases. Consult the documentation on \
	<a href=\"htfuzzy.html\">htfuzzy</a> for more information on the \
	format of this file. \
" }, \
{ "endings_dictionary", "${common_dir}/english.0",  \
	"string", "htfuzzy", "", "all", "File Layout", "endings_dictionary: /var/htdig/dictionary", " \
	Specifies the location of the file which contains the \
	dictionary used to create the endings search algorithm \
	databases. Consult the documentation on \
	<a href=\"htfuzzy.html\">htfuzzy</a> for more information on the \
	format of this file. \
" }, \
{ "endings_root2word_db", "${common_dir}/root2word.db",  \
	"string", "htfuzzy htsearch", "", "all", "File Layout", "endings_root2word_db: /var/htdig/r2w.db", " \
	This attributes specifies the database filename to be \
	used in the 'endings' fuzzy search algorithm. The \
	database maps word roots to all legal words with that \
	root. For more information about this and other fuzzy \
	search algorithms, consult the \
	<a href=\"htfuzzy.html\">htfuzzy</a> documentation.<br> \
	Note that the default value uses the \
	<a href=\"#common_dir\">common_dir</a> attribute instead of the \
	<a href=\"#database_dir\">database_dir</a> attribute. \
	This is because this database can be shared with \
	different search databases. \
" }, \
{ "endings_word2root_db", "${common_dir}/word2root.db",  \
	"string", "htfuzzy htsearch", "", "all", "File Layout", "endings_word2root_db: /var/htdig/w2r.bm", " \
	This attributes specifies the database filename to be \
	used in the 'endings' fuzzy search algorithm. The \
	database maps words to their root. For more information \
	about this and other fuzzy search algorithms, consult \
	the <a href=\"htfuzzy.html\">htfuzzy</a> \
	documentation.<br> \
	Note that the default value uses the \
	<a href=\"#common_dir\">common_dir</a> attribute instead of the \
	<a href=\"#database_dir\">database_dir</a> attribute. \
	This is because this database can be shared with \
	different search databases. \
" }, \
{ "endmonth", "",  \
	"integer", "htsearch", "", "3.1.6", "Searching:Method", "endmonth: 12", " \
	Month component of last date allowed as last-modified date \
	of returned docutments. \
	This is most usefully specified as a \
	<a href=\"hts_form.html#startyear\">GCI argument</a>. \
	See also <a href=\"#startyear\">startyear</a>. \
" }, \
{ "endyear", "",  \
	"integer", "htsearch", "", "3.1.6", "Searching:Method", "endyear: 2002", " \
	Year component of last date allowed as last-modified date \
	of returned docutments. \
	This is most usefully specified as a \
	<a href=\"hts_form.html#startyear\">GCI argument</a>. \
	See also <a href=\"#startyear\">startyear</a>. \
" }, \
{ "excerpt_length", "300",  \
	"integer", "htsearch", "", "all", "Presentation:How", "excerpt_length: 500", " \
	This is the maximum number of characters the displayed \
	excerpt will be limited to. The first matched word will \
	be highlighted in the middle of the excerpt so that there is \
	some surrounding context.<br> \
	The <em><a href=\"#start_ellipses\"> \
	start_ellipses</a></em> and \
	<em><a href=\"#end_ellipses\">end_ellipses</a></em> are used to \
	indicate that the document contains text before and \
	after the displayed excerpt respectively. \
	The <em><a href=\"#start_highlight\">start_highlight</a></em> and \
	<em><a href=\"#end_highlight\">end_highlight</a></em> are used to \
	specify what formatting tags are used to highlight matched words. \
" }, \
{ "excerpt_show_top", "false",  \
	"boolean", "htsearch", "", "all", "Presentation:How", "excerpt_show_top: yes", " \
	If set to true, the excerpt of a match will always show \
	the top of the matching document. If it is false (the \
	default), the excerpt will attempt to show the part of \
	the document that actually contains one of the words. \
" }, \
{ "exclude", "",  \
	"pattern list", "htsearch", "", "3.2.0b4", "Searching:Method", "exclude: myhost.com/mailarchive/", " \
	If a URL contains any of the space separated patterns, it will be \
	discarded in the searching phase. This is used to exclude certain \
	URLs from search results. The list can be specified from within \
	the configuration file, and can be overridden with the \"exclude\" \
	input parameter in the search form. \
" }, \
{ "exclude_urls", "/cgi-bin/ .cgi",  \
	"pattern list", "htdig", "URL", "all", "Indexing:Where", "exclude_urls: students.html cgi-bin", " \
	If a URL contains any of the space separated patterns, \
	it will be rejected. This is used to exclude such \
	common things such as an infinite virtual web-tree \
	which start with cgi-bin. \
" }, \
{ "external_parsers", "",  \
	"quoted string list", "htdig", "", "3.0.7", "External:Parsers", "external_parsers: text/html /usr/local/bin/htmlparser \\<br> \
	application/pdf /usr/local/bin/parse_doc.pl \\<br> \
	application/msword-&gt;text/plain \"/usr/local/bin/mswordtotxt -w\" \\<br> \
	application/x-gunzip-&gt;user-defined /usr/local/bin/ungzipper", " \
	This attribute is used to specify a list of \
	content-type/parsers that are to be used to parse \
	documents that cannot by parsed by any of the internal \
	parsers. The list of external parsers is examined \
	before the builtin parsers are checked, so this can be \
	used to override the internal behavior without \
	recompiling htdig.<br> \
	 The external parsers are specified as pairs of \
	strings. The first string of each pair is the \
	content-type that the parser can handle while the \
	second string of each pair is the path to the external \
	parsing program. If quoted, it may contain parameters, \
	separated by spaces.<br> \
	 External parsing can also be done with external \
	converters, which convert one content-type to \
	another. To do this, instead of just specifying \
	a single content-type as the first string \
	of a pair, you specify two types, in the form \
	<em>type1</em><strong>-&gt;</strong><em>type2</em>, \
	as a single string with no spaces. The second \
	string will define an external converter \
	rather than an external parser, to convert \
	the first type to the second. If the second \
	type is <strong>user-defined</strong>, then \
	it's up to the converter script to put out a \
	\"Content-Type:&nbsp;<em>type</em>\" header followed \
	by a blank line, to indicate to htdig what type it \
	should expect for the output, much like what a CGI \
	script would do. The resulting content-type must \
	be one that htdig can parse, either internally, \
	or with another external parser or converter.<br> \
	 Only one external parser or converter can be \
	specified for any given content-type. However, \
	an external converter for one content-type can be \
	chained to the internal parser for the same type, \
	by appending <strong>-internal</strong> to the \
	second type string (e.g. text/html->text/html-internal) \
	to perform external preprocessing on documents of \
	this type before internal parsing. \
	There are two internal parsers, for text/html and \
	text/plain.<p> \
	The parser program takes four command-line \
	parameters, not counting any parameters already \
	given in the command string:<br> \
	<em>infile content-type URL configuration-file</em><br> \
	<table border=\"1\"> \
	  <tr> \
		<th> Parameter </th> \
		<th> Description </th> \
		<th> Example </th> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> infile </td> \
		<td> A temporary file with the contents to be parsed.  </td> \
		<td> /var/tmp/htdext.14242 </td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> content-type </td> \
		<td> The MIME-type of the contents.  </td> \
		<td> text/html </td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> URL </td> \
		<td> The URL of the contents.  </td> \
		<td> http://www.htdig.org/attrs.html </td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> configuration-file </td> \
		<td> The configuration-file in effect.  </td> \
		<td> /etc/htdig/htdig.conf </td> \
	  </tr> \
	</table><p> \
	The external parser is to write information for \
	htdig on its standard output. Unless it is an \
	external converter, which will output a document \
	of a different content-type, then its output must \
	follow the format described here.<br> \
	 The output consists of records, each record terminated \
	with a newline. Each record is a series of (unless \
	expressively allowed to be empty) non-empty tab-separated \
	fields. The first field is a single character \
	that specifies the record type. The rest of the fields \
	are determined by the record type. \
	<table border=\"1\"> \
	  <tr> \
		<th> Record type </th> \
		<th> Fields </th> \
		<th> Description </th> \
	  </tr> \
	  <tr> \
		<th rowspan=\"3\" valign=\"top\"> w </th> \
		<td valign=\"top\"> word </td> \
		<td> A word that was found in the document.  </td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> location </td> \
		<td> \
		  A number indicating the normalized location of \
		  the word within the document. The number has to \
		  fall in the range 0-1000 where 0 means the top of \
		  the document. \
		</td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> heading level </td> \
		<td> \
		  A heading level that is used to compute the \
		  weight of the word depending on its context in \
		  the document itself. The level is in the range of \
		  0-11 and are defined as follows: \
		  <dl compact> \
			<dt> 0 </dt> <dd> Normal text </dd> \
			<dt> 1 </dt> <dd> Title text </dd> \
			<dt> 2 </dt> <dd> Heading 1 text </dd> \
			<dt> 3 </dt> <dd> Heading 2 text </dd> \
			<dt> 4 </dt> <dd> Heading 3 text </dd> \
			<dt> 5 </dt> <dd> Heading 4 text </dd> \
			<dt> 6 </dt> <dd> Heading 5 text </dd> \
			<dt> 7 </dt> <dd> Heading 6 text </dd> \
			<dt> 8 </dt> <dd> text alternative to images </dd> \
			<dt> 9 </dt> <dd> Keywords </dd> \
			<dt> 10 </dt> <dd> Meta-description </dd> \
			<dt> 11 </dt> <dd> Author </dd> \
		  </dl> \
		</td> \
	  </tr> \
	  <tr> \
		<th rowspan=\"2\" valign=\"top\"> u </th> \
		<td valign=\"top\"> document URL </td> \
		<td> \
		  A hyperlink to another document that is \
		  referenced by the current document.  It must be \
		  complete and non-relative, using the URL parameter to \
		  resolve any relative references found in the document. \
		</td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> hyperlink description </td> \
		<td> \
		  For HTML documents, this would be the text \
		  between the &lt;a href...&gt; and &lt;/a&gt; \
		  tags. \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> t </th> \
		<td valign=\"top\"> title </td> \
		<td> The title of the document </td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> h </th> \
		<td valign=\"top\"> head </td> \
		<td> \
		  The top of the document itself. This is used to \
		  build the excerpt. This should only contain \
		  normal ASCII text \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> a </th> \
		<td valign=\"top\"> anchor </td> \
		<td> \
		  The label that identifies an anchor that can be \
		  used as a target in an URL. This really only \
		  makes sense for HTML documents. \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> i </th> \
		<td valign=\"top\"> image URL </td> \
		<td> \
		  An URL that points at an image that is part of \
		  the document. \
        </td> \
	  </tr> \
	  <tr> \
		<th rowspan=\"3\" valign=\"top\"> m </th> \
		<td valign=\"top\"> http-equiv </td> \
		<td> \
		  The HTTP-EQUIV attribute of a \
		  <a href=\"meta.html\"><em>META</em> tag</a>. \
		  May be empty. \
		</td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> name </td> \
		<td> \
		  The NAME attribute of this \
		  <a href=\"meta.html\"><em>META</em> tag</a>. \
		  May be empty. \
		</td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> contents </td> \
		<td> \
		  The CONTENTS attribute of this \
		  <a href=\"meta.html\"><em>META</em> tag</a>. \
		  May be empty. \
		</td> \
	  </tr> \
	</table> \
	<p><em>See also FAQ questions <a href=\"FAQ.html#q4.8\">4.8</a> and \
	<a href=\"FAQ.html#q4.9\">4.9</a> for more examples.</em></p> \
" }, \
{ "external_protocols", "", \
	"quoted string list", "htdig", "", "3.2.0b1", "External:Protocols", "external_protocols: https /usr/local/bin/handler.pl \\<br> \
	ftp /usr/local/bin/ftp-handler.pl", " \
	This attribute is a bit like \
	<a href=\"#external_parsers\">external_parsers</a> since it specifies \
	a list of protocols/handlers that are used to download documents \
	that cannot be retrieved using the internal methods. This enables \
	htdig to index documents with URL schemes it does not understand, \
	or to use more advanced authentication for the documents it is \
	retrieving. This list is checked before HTTP or other methods, \
	so this can override the internal behavior without writing additional \
	code for htdig.<br> \
	  The external protocols are specified as pairs of strings, the first \
	being the URL scheme that the script can handle while the second \
	is the path to the script itself. If the second is \
	quoted, then additional command-line arguments may be given.<br> \
	If the external protocol does not contain a colon (:), it is assumed \
	to have the standard format \
	\"protocol://[usr[:password]@]address[:port]/path\". \
	If it ends with a colon, then it is assumed to have the simpler format \
	\"protocol:path\". If it ends with \"://\" then the standard form is \
	again assumed. <br> \
	  The program takes three command-line parameters, not counting any \
	parameters already given in the command string:<br> \
	<em>protocol URL configuration-file</em><br> \
	<table border=\"1\"> \
	  <tr> \
		<th> Parameter </th> \
		<th> Description </th> \
		<th> Example </th> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> protocol </td> \
		<td> The URL scheme to be used.  </td> \
		<td> https </td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> URL </td> \
		<td> The URL to be retrieved.  </td> \
		<td> https://www.htdig.org:8008/attrs.html </td> \
	  </tr> \
	  <tr> \
		<td valign=\"top\"> configuration-file </td> \
		<td> The configuration-file in effect.  </td> \
		<td> /etc/htdig/htdig.conf </td> \
	  </tr> \
	</table><p> \
	The external protocol script is to write information for htdig on the  \
	standard output. The output must follow the form described here. The \
	output consists of a header followed by a blank line, followed by \
	the contents of the document. Each record in the header is terminated \
	with a newline.  Each record is a series of (unless expressively \
	allowed to be empty) non-empty tab-separated fields. The first field \
	is a single character that specifies the record type. The rest of \
	the fields are determined by the record type. \
	<table border=\"1\"> \
	  <tr> \
		<th> Record type </th> \
		<th> Fields </th> \
		<th> Description </th> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> s </th> \
		<td valign=\"top\"> status code </td> \
		<td> \
		  An HTTP-style status code, e.g. 200, 404. Typical codes include: \
		    <dl compact> \
			<dt> 200 </dt> \
			    <dd> Successful retrieval </dd> \
			<dt> 304 </dt> \
			<dd> \
			  Not modified (for example, if the document hasn\'t \
			  changed since the last dig) \
			</dd> \
			<dt> 301 </dt> \
			    <dd> Redirect (to another URL) </dd> \
			<dt> 401 </dt> \
			    <dd> Not authorized </dd> \
			<dt> 404 </dt> \
			    <dd> Not found </dd> \
		    </dl> \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> r </th> \
		<td valign=\"top\"> reason </td> \
		<td> \
		  A text string describing the status code, \
		  e.g \"Redirect\" or \"Not Found.\" \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> m </th> \
		<td valign=\"top\"> status code </td> \
		<td> \
		  The modification time of this document. While the code is \
		  fairly flexible about the time/date formats it accepts, it \
		  is recommended to use something standard, like \
		  RFC1123: Sun, 06 Nov 1994 08:49:37 GMT, or \
		  ISO-8601:  1994-11-06 08:49:37 GMT. \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> t </th> \
		<td valign=\"top\"> content-type </td> \
		<td> \
		  A valid MIME type for the document, like text/html or text/plain. \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> l </th> \
		<td valign=\"top\"> content-length </td> \
		<td> \
		  The length of the document on the server, which may not \
		  necessarily be the length of the buffer returned. \
		</td> \
	  </tr> \
	  <tr> \
		<th valign=\"top\"> u </th> \
		<td valign=\"top\"> url </td> \
		<td> \
		  The URL of the document, or in the case of a redirect, the \
		  URL that should be indexed as a result of the redirect. \
		</td> \
	  </tr> \
      </table>	   \
" }, \
{ "extra_word_characters", "",  \
	"string", "htdig htsearch", "", "3.1.2", "Indexing:What", "extra_word_characters: _", " \
	These characters are considered part of a word. \
	In contrast to the characters in the \
	<a href=\"#valid_punctuation\">valid_punctuation</a> \
	attribute, they are treated just like letter \
	characters.  See also the <a href=\"#allow_numbers\">allow_numbers</a>\
	attribute.<br> \
	Note that the <a href=\"#locale\">locale</a> attribute \
	is normally used to configure which characters \
	constitute letter characters.<br> \
	Note also that it is an error to have characters in both \
	extra_word_characters and \
	<a href=\"#valid_punctuation\">valid_punctuation</a>. \
	To add one of the characters in the default valid_punctuation to \
	extra_word_characters, an explicit valid_punctuation entry must be \
	added to the configuration file.<br> \
	See also the comments about special characters at \
	<a href=\"#valid_punctuation\">valid_punctuation</a>. \
" }, \
{ "head_before_get", "true",  \
	"boolean", "htdig", "Server", "3.2.0b1", "Indexing:Connection", "head_before_get: false", " \
    If set to true an HTTP/1.1 <em>HEAD</em> \
	call is made in order to retrieve header information about a document. \
	If the status code and the content-type returned let the document be parsable, \
	then a following 'GET' call is made. \
    In general, it is recommended to leave this attribute set to 'true' (especially when used \
    with persistent connections performances can really improve), especially during an \
    incremental dig (in this case 'htdig' can request the server if the document has been \
    modified since last dig with a more proper and correct HEAD call). \
    However there are a couple of cases when it is better to switch it off: \
    <ul> \
    <li>the majority of documents is parsable (HTML and in general for those types of documents \
    that have a proper external parser) and must be retrieved anyway (initial dig);</li> \
    <li>the server does not support the HEAD method or it is disabled;</li> \
    <li>in some cases persistent connections \
    (<a href=\"#persistent_connections\">persistent_connections</a>) may not work properly \
    and either the 'head_before_get' attribute or the 'persistent_connections' one may be \
    turned off. \
    </li> \
    </ul> \
" }, \
{ "heading_factor", "5",  \
	"number", "htsearch", "", "3.2.0b1", "Searching:Ranking", "heading_factor: 20", " \
			This is a factor which will be used to multiply the \
			weight of words between &lt;h1&gt; and &lt;/h1&gt; \
			tags, as well as headings of levels &lt;h2&gt; through \
			&lt;h6&gt;. It is used to assign the level of importance \
			to headings. Setting a factor to 0 will cause words \
			in these headings to be ignored. The number may be a \
	floating point number. See also \
	<a href=\"#author_factor\">author_factor</a> \
	<a href=\"#backlink_factor\">backlink_factor</a> \
	<a href=\"#caps_factor\">caps_factor</a> \
	<a href=\"#date_factor\">date_factor</a> \
	<a href=\"#description_factor\">description_factor</a> \
	<a href=\"#keywords_factor\">keywords_factor</a> \
	<a href=\"#meta_description_factor\">meta_description_factor</a> \
	<a href=\"#text_factor\">text_factor</a> \
	<a href=\"#title_factor\">title_factor</a> \
	<a href=\"#url_text_factor\">url_text_factor</a> \
" }, \
{ "htnotify_prefix_file", "", \
    "string", "htnotify", "", "3.2.0b3", "Extra Output", "htnotify_prefix_file: ${common_dir}/notify_prefix.txt", " \
	Specifies the file containing text to be inserted in each mail  \
	message sent by htnotify before the list of expired webpages. If omitted,  \
	nothing is inserted. \
" }, \
{ "htnotify_replyto", "", \
    "string", "htnotify", "", "3.2.0b3", "Extra Output", "htnotify_replyto: design-group@foo.com", " \
	This specifies the email address that htnotify email messages \
	include in the Reply-to: field. \
" }, \
{ "htnotify_sender", "webmaster@www",  \
	"string", "htnotify", "", "all", "Extra Output", "htnotify_sender: bigboss@yourcompany.com", " \
	This specifies the email address that htnotify email \
	messages get sent out from. The address is forged using \
	/usr/lib/sendmail. Check htnotify/htnotify.cc for \
	detail on how this is done. \
" }, \
{ "htnotify_suffix_file", "", \
    "string", "htnotify", "", "3.2.0b3", "Extra Output", "htnotify_suffix_file: ${common_dir}/notify_suffix.txt", " \
	Specifies the file containing text to be inserted in each mail message  \
	sent by htnotify after the list of expired webpages. If omitted, htnotify  \
	will insert a standard message. \
" }, \
{ "htnotify_webmaster",	 "ht://Dig Notification Service", \
    "string", "htnotify", "", "3.2.0b3", "Extra Output", "htnotify_webmaster: Notification Service", " \
	This provides a name for the From field, in addition to the email \
	address for the email messages sent out by htnotify. \
" }, \
{ "http_proxy", "",  \
	"string", "htdig", "URL", "3.0", "Indexing:Connection", "http_proxy: http://proxy.bigbucks.com:3128", " \
	When this attribute is set, all HTTP document \
	retrievals will be done using the HTTP-PROXY protocol. \
	The URL specified in this attribute points to the host \
	and port where the proxy server resides.<br> \
	Later, this should be able to be overridden by the \
	<code>http_proxy</code> environement variable, but it currently cannot.\
	The use of a proxy server greatly improves performance \
	of the indexing process.<br> \
	See also \
	<a href=\"#http_proxy_authorization\">http_proxy_authorization</a> and \
	<a href=\"#http_proxy_exclude\">#http_proxy_exclude</a>. \
" }, \
{ "http_proxy_authorization", "",  \
	"string", "htdig", "URL", "3.2.0b4", "Indexing:Connection", "http_proxy_authorization: myusername:mypassword", " \
	This tells htdig to send the supplied \
	<em>username</em><strong>:</strong><em>password</em> with each HTTP request, \
	when using a proxy with authorization requested. \
	The credentials will be encoded using the \"Basic\" authentication \
	scheme. There <em>must</em> be a colon (:) between the username and \
	password.<br> \
	If you use this option, be sure to protect the configuration file \
	so it is readable only by you, and do not \
	use that same configuration file for htsearch. \
" }, \
{ "http_proxy_exclude", "", \
	"pattern list", "htdig", "", "3.1.0b3", "Indexing:Connection", "http_proxy_exclude: http://intranet.foo.com/", " \
	When this is set, URLs matching this will not use the \
	proxy. This is useful when you have a mixture of sites \
	near to the digging server and far away. \
" }, \
{ "ignore_alt_text", "false",  \
	"boolean", "htdig", "", "3.1.6", "Indexing:What", "ignore_alt_text: true", " \
	If set, this causes the text of the ALT field in an &lt;IMG...&gt; tag \
	not to be indexed as part of the text of the document, nor included in \
	excerpts. \
" }, \
{ "ignore_dead_servers", "true",  \
	"boolean", "htdig", "", "3.1.6", "Indexing:Connection", "ignore_dead_servers: false", " \
	Determines whether htdig will continue to index URLs from a \
	server after an attempted connection to the server fails as \
	&quot;no host found&quot; or &quot;host not found (port).&quot; If \
	set to false, htdig will try <em>every</em> URL from that server. \
" }, \
{ "image_list", "${database_base}.images",  \
	"string", "htdig", "", "all", "Extra Output", "image_list: allimages", " \
	This is the file that a list of image URLs gets written \
	to by <a href=\"htdig.html\">htdig</a> when the \
	<a href=\"#create_image_list\">create_image_list</a> is set to \
	true. As image URLs are seen, they are just appended to \
	this file, so after htdig finishes it is probably a \
	good idea to run <code>sort -u</code> on the file to \
	eliminate duplicates from the file. \
" }, \
{ "image_url_prefix", IMAGE_URL_PREFIX,  \
	"string", "htsearch", "", "all", "Presentation:Text", "image_url_prefix: /images/htdig", " \
	This specifies the directory portion of the URL used \
	to display star images. This attribute isn't directly \
	used by htsearch, but is used in the default URL for \
	the <a href=\"#star_image\">star_image</a> and \
	<a href=\"#star_blank\">star_blank</a> attributes, and \
	other attributes may be defined in terms of this one. \
	<p> \
	The default value of this attribute is determined at \
	compile time. \
	</p> \
" }, \
{ "include", "", \
	"string", "all", "", "3.1.0", "", "include: ${config_dir}/htdig.conf", " \
	This is not quite a configuration attribute, but \
	rather a directive. It can be used within one \
	configuration file to include the definitions of \
	another file. The last definition of an attribute \
	is the one that applies, so after including a file, \
	any of its definitions can be overridden with \
	subsequent definitions. This can be useful when \
	setting up many configurations that are mostly the \
	same, so all the common attributes can be maintained \
	in a single configuration file. The include directives \
	can be nested, but watch out for nesting loops. \
" }, \
{ "iso_8601", "false",  \
	"boolean", "htsearch htnotify", "", "3.1.0b2", "Presentation:How,Extra Output", "iso_8601: true", " \
	This sets whether dates should be output in ISO 8601 \
	format. For example, this was written on: 1998-10-31 11:28:13 EST. \
	See also the <a \
	href=\"#date_format\">date_format</a> attribute, which \
	can override any date format that \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a> \
	picks by default.<br> \
	This attribute also affects the format of the date \
	<a href=\"htnotify.html\">htnotify</a> expects to find \
	in a <strong>htdig-notification-date</strong> field. \
" }, \
{ "keywords", "", \
    "string list", "htsearch", "", "??", "Searching:Method", "keywords: documentation", " \
	Keywords which <strong>must</strong> be found on all pages returned, \
    	even if the \"or\" (\"Any\") <a href=\"#method\">method</a> is \
	selected. \
" }, \
{ "keywords_factor", "100",  \
	"number", "htsearch", "", "all", "Searching:Ranking", "keywords_factor: 12", " \
	This is a factor which will be used to multiply the \
	weight of words in the list of keywords of a document. \
	The number may be a floating point number. See also the \
	<a href=\"#heading_factor\">heading_factor</a> attribute. \
" }, \
{ "keywords_meta_tag_names", "keywords htdig-keywords",  \
	"string list", "htdig", "", "3.0.6", "Indexing:What", "keywords_meta_tag_names: keywords description", " \
	The words in this list are used to search for keywords \
	in HTML <em>META</em> tags. This list can contain any \
	number of strings that each will be seen as the name \
	for whatever keyword convention is used.<br> \
	The <em>META</em> tags have the following format:<br> \
<code> \
&nbsp;&nbsp;&lt;META name=\"<em>somename</em>\" content=\"<em>somevalue</em>\"&gt; \
</code> \
" }, \
{ "limit_normalized", "",  \
	"pattern list", "htdig", "", "3.1.0b2", "Indexing:Where", "limit_normalized: http://www.mydomain.com", " \
	This specifies a set of patterns that all URLs have to \
	match against in order for them to be included in the \
	search. Unlike the limit_urls_to attribute, this is done \
	<strong>after</strong> the URL is normalized and the \
	<a href=\"#server_aliases\">server_aliases</a> \
	attribute is applied. This allows filtering after any \
	hostnames and DNS aliases are resolved. Otherwise, this \
	attribute is the same as the <a \
	href=\"#limit_urls_to\">limit_urls_to</a> attribute. \
" }, \
{ "limit_urls_to", "${start_url}",  \
	"pattern list", "htdig", "", "all", "Indexing:Where", "limit_urls_to: .sdsu.edu kpbs [.*\\.html]", " \
	This specifies a set of patterns that all URLs have to \
	match against in order for them to be included in the \
	search. Any number of strings can be specified, \
	separated by spaces. If multiple patterns are given, at \
	least one of the patterns has to match the URL.<br> \
	Matching, by default, is a case-insensitive string match on the URL \
	to be used, unless the <a href=\"#case_sensitive\">case_sensitive</a> \
	attribute is set. The match will be performed <em>after</em> \
	the relative references have been converted to a valid \
	URL. This means that the URL will <em>always</em> start \
	with a transport specifier (<code>http://</code> if none is \
	specified).<br> \
	Granted, this is not the perfect way of doing this, \
	but it is simple enough and it covers most cases.<br> \
	To limit URLs in htsearch, use \
	<a href=\"restrict\">restrict</a>. \
" }, \
{ "local_default_doc", "index.html",  \
	"string list", "htdig", "Server", "3.0.8b2", "Indexing:Where", "local_default_doc: default.html default.htm index.html index.htm", " \
	Set this to the default documents in a directory used by the \
	server. This is used for local filesystem access, \
	using <a href=\"#local_urls\">local_urls</a>, to \
	translate URLs like http://foo.com/ into something like \
	/home/foo.com/index.html \
	(see also <a href=\"#remove_default_doc\">remove_default_doc</a>). \
	<br>The list should only contain names that the local server \
	recognizes as default documents for directory URLs, as defined \
	by the DirectoryIndex setting in Apache's srm.conf, for example. \
	As of version 3.1.5, this can be a string list rather than a single \
	name, and htdig will use the first name that works. Since this \
	requires a loop, setting the most common name first will improve \
	performance.  Special characters can be embedded in these names \
	using %xx hex encoding. \
" }, \
{ "local_urls", "",  \
	"string list", "htdig", "", "3.0.8b2", "Indexing:Where", "local_urls: http://www.foo.com/=/usr/www/htdocs/", " \
	Set this to tell ht://Dig to access certain URLs through \
	local filesystems. At first ht://Dig will try to access \
	pages with URLs matching the patterns through the \
	filesystems specified. If it cannot find the file, or \
	if it doesn't recognize the file name extension, it will \
	try the URL through HTTP instead. Note the example--the \
	equal sign and the final slashes in both the URL and the \
	directory path are critical. \
	<br>The fallback to HTTP can be disabled by setting the \
	<a href=\"#local_urls_only\">local_urls_only</a> attribute to true. \
	To access user directory URLs through the local filesystem, \
	set <a href=\"#local_user_urls\">local_user_urls</a>.  \
	File types which need processing by the HTTP server may be \
	specified by the \
	<a href=\"#bad_local_extensions\">bad_local_extensions</a> \
	attribute. \
	As of version 3.1.5, you can provide multiple mappings of a given \
	URL to different directories, and htdig will use the first \
	mapping that works. \
	Special characters can be embedded in these names using %xx hex encoding. \
	For example, you can use %3D to embed an \"=\" sign in an URL pattern. \
	<br> \
	See also <a href=\"#local_user_urls\">local_user_urls</a>. \
" }, \
{ "local_urls_only", "false",  \
	"boolean", "htdig", "", "3.1.4", "Indexing:Where", "local_urls_only: true", " \
	Set this to tell ht://Dig to access files only through the  \
	local filesystem, for URLs matching the patterns in the \
	<a href=\"#local_urls\">local_urls</a> or \
	<a href=\"#local_user_urls\">local_user_urls</a> attribute. If it \
	cannot find the file, it will give up rather than trying HTTP or \
	another protocol. \
" }, \
{ "local_user_urls", "",  \
	"string list", "htdig", "", "3.0.8b2", "Indexing:Where", "local_user_urls: http://www.my.org/=/home/,/www/", " \
	Set this to access user directory URLs through the local \
	filesystem. If you leave the \"path\" portion out, it will \
	look up the user's home directory in /etc/password (or NIS \
	or whatever). As with <a href=\"#local_urls\">local_urls</a>, \
	if the files are not found, ht://Dig will try with HTTP or the \
	appropriate protocol. Again, note the \
	example's format. To map http://www.my.org/~joe/foo/bar.html \
	to /home/joe/www/foo/bar.html, try the example below. \
	<br>The fallback to HTTP can be disabled by setting the \
	<a href=\"#local_urls_only\">local_urls_only</a> attribute to true. \
	As of version 3.1.5, you can provide multiple mappings of a given \
	URL to different directories, and htdig will use the first \
	mapping that works. \
	Special characters can be embedded in these names using %xx hex encoding. \
	For example, you can use %3D to embed an \"=\" sign in an URL pattern. \
" }, \
{ "locale", "C",  \
	"string", "htdig", "", "3.0", "Indexing:What,Presentation:How", "locale: en_US", " \
	Set this to whatever locale you want your search \
	database cover. It affects the way international \
	characters are dealt with. On most systems a list of \
	legal locales can be found in /usr/lib/locale. Also \
	check the <strong>setlocale(3C)</strong> man page. \
	Note that depending the locale you choose, and whether \
	your system's locale implementation affects floating \
	point input, you may need to specify the decimal point \
	as a comma rather than a period. This will affect \
	settings of <a href=\"#search_algorithm\">search_algorithm</a> \
	and any of the scoring factors. \
" }, \
{ "logging", "false",  \
	"boolean", "htsearch", "", "3.1.0b2", "Extra Output", "logging: true", " \
	This sets whether htsearch should use the syslog() to log \
	search requests. If set, this will log requests with a \
	default level of LOG_INFO and a facility of LOG_LOCAL5. For \
	details on redirecting the log into a separate file or other \
	actions, see the <strong>syslog.conf(5)</strong> man \
	page. To set the level and facility used in logging, change \
	LOG_LEVEL and LOG_FACILITY in the include/htconfig.h file \
	before compiling. \
	<dl> \
	  <dt> \
	    Each line logged by htsearch contains the following: \
	  </dt> \
	  <dd> \
	    REMOTE_ADDR [config] (match_method) [words] \
	    [logicalWords] (matches/matches_per_page) - \
	    page, HTTP_REFERER \
	  </dd> \
	</dl> \
	where any of the above are null or empty, it \
	either puts in '-' or 'default' (for config). \
" }, \
{ "maintainer", "bogus@unconfigured.htdig.user",  \
	"string", "htdig", "Server", "all", "Indexing:Out", "maintainer: ben.dover@uptight.com", " \
	This should be the email address of the person in \
	charge of the digging operation. This string is added \
	to the user-agent: field when the digger sends a \
	request to a server. \
" }, \
{ "match_method", "and",  \
	"string", "htsearch", "", "3.0", "Searching:Method", "match_method: boolean", " \
	This is the default method for matching that htsearch \
	uses. The valid choices are: \
	<ul> \
	  <li> or </li> \
	  <li> and </li> \
	  <li> boolean </li> \
	</ul> \
	This attribute will only be used if the HTML form that \
	calls htsearch didn't have the \
	<a href=\"hts_form.html#method\">method</a> value set. \
" }, \
{ "matches_per_page", "10",  \
	"integer", "htsearch", "", "3.0", "Searching:Method", "matches_per_page: 999", " \
	If this is set to a relatively small number, the \
	matches will be shown in pages instead of all at once. \
	This attribute will only be used if the HTML form that \
	calls htsearch didn't have the \
	<a href=\"hts_form.html#matchesperpage\">matchesperpage</a> value set. \
" }, \
{ "max_connection_requests", "-1", \
	"integer", "htdig", "", "3.2.0b1", "Indexing:Connection", "max_connection_requests: 100", " \
	This attribute tells htdig to limit the number of requests it will \
	send to a server using a single, persistent HTTP connection. This \
	only applies when the \
	<a href=\"#persistent_connections\">persistent_connections</a> \
	attribute is set. You may set the limit as high as you want, \
	but it must be at least 1. A value of -1 specifies no limit. \
	Requests in the queue for a server will be combined until either \
	the limit is reached, or the queue is empty. \
" }, \
{ "max_description_length", "60",  \
	"integer", "htdig", "", "all", "Indexing:What", "max_description_length: 40", " \
	While gathering descriptions of URLs, \
	<a href=\"htdig.html\">htdig</a> will only record those \
	descriptions which are shorter than this length (in bytes). This \
	is used mostly to deal with broken HTML. (If a \
	hyperlink is not terminated with a &lt;/a&gt; the \
	description will go on until the end of the document.) \
" }, \
{ "max_descriptions", "5",  \
	"integer", "htdig", "", "all", "Indexing:What", "max_descriptions: 15", " \
	While gathering descriptions of URLs, \
	<a href=\"htdig.html\">htdig</a> will only record up to this \
	number of descriptions, in the order in which it encounters \
	them. This is used to prevent the database entry for a document \
	from growing out of control if the document has a huge number \
	of links to it. \
" }, \
{ "max_doc_size", "100000",  \
	"integer", "htdig", "URL", "3.0", "Indexing:What", "max_doc_size: 5000000", " \
	This is the upper limit to the amount of data retrieved \
	for documents (in bytes). This is mainly used to prevent \
	unreasonable memory consumption since each document \
	will be read into memory by <a href=\"htdig.html\"> \
	htdig</a>. \
" }, \
{ "max_excerpts", "1",  \
	"integer", "htsearchg", "URL", "3.1.6", "Presentation:How", "max_excerpts: 10", " \
	This value determines the maximum number of excerpts \
	that can be displayed for one matching document in the \
	search results. \
" }, \
{ "max_head_length", "512",  \
	"integer", "htdig", "", "all", "Indexing:How", "max_head_length: 50000", " \
	For each document retrieved, the top of the document is \
	stored. This attribute determines the size of this \
	block (in bytes). The text that will be stored is only the text; \
	no markup is stored.<br> \
	We found that storing 50,000 bytes will store about \
	95% of all the documents completely. This really \
	depends on how much storage is available and how much \
	you want to show. \
" }, \
{ "max_hop_count", "999999",  \
	"integer", "htdig", "", "all", "Indexing:Where", "max_hop_count: 4", " \
	Instead of limiting the indexing process by URL \
	pattern, it can also be limited by the number of hops \
	or clicks a document is removed from the starting URL. \
	<br> \
	The starting page or pages will have hop count 0. \
" }, \
{ "max_keywords", "-1",  \
	"integer", "htdig", "", "3.2.0b1", "Indexing:What", "max_keywords: 10", " \
	This attribute can be used to limit the number of keywords \
	per document that htdig will accept from meta keywords tags. \
	A value of -1 or less means no limit. This can help combat meta \
	keyword spamming, by limiting the amount of keywords that will be \
	indexed, but it will not completely prevent irrelevant matches \
	in a search if the first few keywords in an offending document \
	are not relevant to its contents. \
" }, \
{ "max_meta_description_length", "512",  \
	"integer", "htdig", "", "3.1.0b1", "Indexing:How", "max_meta_description_length: 1000", " \
	While gathering descriptions from meta description tags, \
	<a href=\"htdig.html\">htdig</a> will only store up to  \
	this much of the text (in bytes) for each document. \
" }, \
{ "max_prefix_matches", "1000",  \
	"integer", "htsearch", "", "3.1.0b1", "Searching:Method", "max_prefix_matches: 100", " \
	The Prefix <a href=\"#search_algorithm\">fuzzy algorithm</a> \
	could potentially match a \
	very large number of words. This value limits the \
	number of words each prefix can match. Note \
	that this does not limit the number of documents that \
	are matched in any way. \
" }, \
{ "max_retries", "3",  \
	"integer", "htdig", "", "3.2.0b1", "Indexing:Connection", "max_retries: 6", " \
	 This option set the maximum number of retries when retrieving a document \
	 fails (mainly for reasons of connection). \
" }, \
{ "max_stars", "4",  \
	"integer", "htsearch", "", "all", "Presentation:How", "max_stars: 6", " \
	When stars are used to display the score of a match, \
	this value determines the maximum number of stars that \
	can be displayed. \
" }, \
{ "maximum_page_buttons", "${maximum_pages}",  \
	"integer", "htsearch", "", "3.2.0b3", "Presentation:How", "maximum_page_buttons: 20", " \
	This value limits the number of page links that will be \
	included in the page list at the bottom of the search \
	results page. By default, it takes on the value of the \
	<a href=\"#maximum_pages\">maximum_pages</a> \
	attribute, but you can set it to something lower to allow \
	more pages than buttons. In this case, pages above this \
	number will have no corresponding button. \
" }, \
{ "maximum_pages", "10",  \
	"integer", "htsearch", "", "all", "Presentation:How", "maximum_pages: 20", " \
	This value limits the number of page links that will be \
	included in the page list at the bottom of the search \
	results page. As of version 3.1.4, this will limit the \
	total number of matching documents that are shown. \
	You can make the number of page buttons smaller than the \
	number of allowed pages by setting the \
	<a href=\"#maximum_page_buttons\">maximum_page_buttons</a> \
	attribute. \
" }, \
{ "maximum_word_length", "32",  \
	"integer", "htdig htsearch htfuzzy", "", "3.1.3", "Indexing:What", "maximum_word_length: 15", " \
	This sets the maximum length of words that will be \
	indexed. Words longer than this value will be silently \
	truncated when put into the index, or searched in the \
	index. \
" }, \
{ "md5_db", "${database_base}.md5hash.db",  \
	"string", "htdig", "", "3.2.0b3", "File Layout", "md5_db: ${database_base}.md5.db", " \
	This file holds a database of md5 and date hashes of pages to \
	catch and eliminate duplicates of pages. See also the \
	<a href=\"#check_unique_md5\">check_unique_md5</a> and \
	<a href=\"#check_unique_date\">check_unique_date</a> attributes. \
" }, \
{ "meta_description_factor", "50",  \
	"number", "htsearch", "", "3.1.0b1", "Searching:Ranking", "meta_description_factor: 20", " \
	This is a factor which will be used to multiply the \
	weight of words in any META description tags in a document. \
	The number may be a floating point number. See also the \
	<a href=\"#heading_factor\">heading_factor</a> attribute and the \
	<a href=\"#description_factor\">description_factor</a> attribute. \
" }, \
{ "metaphone_db", "${database_base}.metaphone.db",  \
	"string", "htfuzzy htsearch", "", "all", "File Layout", "metaphone_db: ${database_base}.mp.db", " \
	The database file used for the fuzzy \"metaphone\" search \
	algorithm. This database is created by \
	<a href=\"htfuzzy.html\">htfuzzy</a> and used by \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>. \
" }, \
{ "method_names", "and All or Any boolean Boolean",  \
	"quoted string list", "htsearch", "", "all", "Searching:UI", "method_names: or Or and And", " \
	These values are used to create the <strong> \
	method</strong> menu. It consists of pairs. The first \
	element of each pair is one of the known methods, the \
	second element is the text that will be shown in the \
	menu for that method. This text needs to be quoted if \
	it contains spaces. \
	See the <a href=\"hts_selectors.html\">select list documentation</a> \
	for more information on how this attribute is used. \
" }, \
{ "mime_types", "${config_dir}/mime.types", \
	"string", "htdig", "", "3.2.0b1", "Indexing:Where", "mime_types: /etc/mime.types", " \
	This file is used by htdig for local file access and resolving \
	file:// URLs to ensure the files are parsable. If you are running \
	a webserver with its own MIME file, you should set this attribute \
	to point to that file. \
	<p> \
	See also <a href=\"#content_classifier\">content_classifier</a>.\
"}, \
{ "minimum_prefix_length", "1",  \
	"integer", "htsearch", "", "3.1.0b1", "Searching:Method", "minimum_prefix_length: 2", " \
	This sets the minimum length of prefix matches used by the \
	\"prefix\" fuzzy matching algorithm. Words shorter than this \
	will not be used in prefix matching. \
" }, \
{ "minimum_speling_length", "5",  \
	"integer", "htsearch", "", "3.2.0b1", "Searching:Method", "minimum_speling_length: 3", " \
	This sets the minimum length of words used by the \
	\"speling\" fuzzy matching algorithm. Words shorter than this \
	will not be used in this fuzzy matching. \
" }, \
{ "minimum_word_length", "3",  \
	"integer", "htdig htsearch", "", "all", "Indexing:What", "minimum_word_length: 2", " \
	This sets the minimum length of words that will be \
	indexed. Words shorter than this value will be silently \
	ignored but still put into the excerpt.<br> \
	Note that by making this value less than 3, a lot more \
	words that are very frequent will be indexed. It might \
	be advisable to add some of these to the \
	<a href=\"#bad_word_list\">bad_words list</a>. \
" }, \
{ "multimatch_factor", "1",  \
	"number", "htsearch", "", "3.1.6", "Searching:Ranking", "multimatch_factor: 1000", " \
    	This factor gives higher rankings to documents that have more than \
	one matching search word when the <strong>or</strong> \
	<a href=\"#match_method\">match_method</a> is used. \
	In version 3.1.6, the matching words' combined scores were multiplied \
	by this factor for each additional matching word.  Currently, this \
	multiplier is applied at most once. \
" },
{ "next_page_text", "[next]",  \
	"string", "htsearch", "", "3.1.0", "Presentation:Text", "next_page_text: &lt;img src=\"/htdig/buttonr.gif\"&gt;", " \
	The text displayed in the hyperlink to go to the next \
	page of matches. \
" }, \
{ "no_excerpt_show_top", "false",  \
	"boolean", "htsearch", "", "3.1.0b3", "Presentation:How", "no_excerpt_show_top: yes", " \
	If no excerpt is available, this option will act the \
	same as <a \
	href=\"#excerpt_show_top\">excerpt_show_top</a>, that is, \
	it will show the top of the document. \
" }, \
{ "no_excerpt_text", "<em>(None of the search words were found in the top of this document.)</em>",  \
	"string", "htsearch", "", "3.0", "Presentation:Text", "no_excerpt_text:", " \
	This text will be displayed in place of the excerpt if \
	there is no excerpt available. If this attribute is set \
	to nothing (blank), the excerpt label will not be \
	displayed in this case. \
" }, \
{ "no_next_page_text", "[next]",  \
	"string", "htsearch", "", "3.0", "Presentation:Text", "no_next_page_text:", " \
	The text displayed where there would normally be a \
	hyperlink to go to the next page of matches. \
" }, \
{ "no_page_list_header", "",  \
	"string", "htsearch", "", "3.0", "Presentation:Text", "no_page_list_header: &lt;hr noshade size=2&gt;All results on this page.&lt;br&gt;", " \
	This text will be used as the value of the PAGEHEADER \
	variable, for use in templates or the \
	<a href=\"#search_results_footer\">search_results_footer</a> \
	file, when all search results fit on a single page. \
" }, \
{ "no_page_number_text", "",  \
	"quoted string list", "htsearch", "", "3.0", "Presentation:Text", "no_page_number_text: \
				  &lt;strong&gt;1&lt;/strong&gt; &lt;strong&gt;2&lt;/strong&gt; \\<br> \
				  &lt;strong&gt;3&lt;/strong&gt; &lt;strong&gt;4&lt;/strong&gt; \\<br> \
				  &lt;strong&gt;5&lt;/strong&gt; &lt;strong&gt;6&lt;/strong&gt; \\<br> \
				  &lt;strong&gt;7&lt;/strong&gt; &lt;strong&gt;8&lt;/strong&gt; \\<br> \
				  &lt;strong&gt;9&lt;/strong&gt; &lt;strong&gt;10&lt;/strong&gt; \
", " \
	The text strings in this list will be used when putting \
	together the PAGELIST variable, for use in templates or \
	the <a href=\"#search_results_footer\">search_results_footer</a> \
	file, when search results fit on more than page. The PAGELIST \
	is the list of links at the bottom of the search results page. \
	There should be as many strings in the list as there are \
	pages allowed by the <a href=\"#maximum_page_buttons\">maximum_page_buttons</a> \
	attribute. If there are not enough, or the list is empty, \
	the page numbers alone will be used as the text for the links. \
	An entry from this list is used for the current page, as the \
	current page is shown in the page list without a hypertext link, \
	while entries from the <a href=\"#page_number_text\"> \
	page_number_text</a> list are used for the links to other pages. \
	The text strings can contain HTML tags to highlight page numbers \
	or embed images. The strings need to be quoted if they contain \
	spaces. \
" }, \
{ "no_prev_page_text", "[prev]",  \
	"string", "htsearch", "", "3.0", "Presentation:Text", "no_prev_page_text:", " \
	The text displayed where there would normally be a \
	hyperlink to go to the previous page of matches. \
" }, \
{ "no_title_text", "filename",  \
	"string", "htsearch", "", "3.1.0", "Presentation:Text", "no_title_text: \"No Title Found\"", " \
	This specifies the text to use in search results when no \
	title is found in the document itself. If it is set to \
	filename, htsearch will use the name of the file itself, \
	enclosed in brackets (e.g. [index.html]). \
" }, \
{ "noindex_end", "<!--/htdig_noindex--> </SCRIPT>",  \
	"quoted string list", "htdig", "", "3.1.0", "Indexing:What", "noindex_end: &lt;/SCRIPT&gt;", " \
	This string marks the end of a section of an HTML file that should be \
	completely ignored when indexing. See also \
	<a href=\"#noindex_start\">noindex_start</a>. \
" }, \
{ "noindex_start", "<!--htdig_noindex--> <SCRIPT",  \
	"quoted string list", "htdig", "", "3.1.0", "Indexing:What", "noindex_start: &lt;SCRIPT", " \
	These strings mark the start of a section of an HTML file that should \
	be completely ignored when indexing. They work together with \
	<a href=\"#noindex_end\">noindex_end</a>.  Once a string in \
	noindex_start is found, text is ignored until the string at the \
	<em>same position</em> within <a href=\"#noindex_end\">noindex_end</a> \
	is encountered.  The sections marked off this way cannot overlap. \
	As in the first default pattern, this can be SGML comment \
	declarations that can be inserted anywhere in the documents to exclude \
	different sections from being indexed. However, existing tags can also \
	be used; this is especially useful to exclude some sections from being \
	indexed where the files to be indexed can not be edited. The second \
	default pattern shows how SCRIPT sections in 'uneditable' documents \
	can be skipped; note how noindex_start does not contain an ending \
	&gt;: this allows for all SCRIPT tags to be matched regardless of \
	attributes defined (different types or languages). \
	Note that the match for this string is case insensitive. \
" }, \
{ "nothing_found_file", "${common_dir}/nomatch.html",  \
	"string", "htsearch", "", "all", "Presentation:Files", "nothing_found_file: /www/searching/nothing.html", " \
	This specifies the file which contains the <code> \
	HTML</code> text to display when no matches were found. \
	The file should contain a complete <code>HTML</code> \
	document.<br> \
	Note that this attribute could also be defined in \
	terms of <a href=\"#database_base\">database_base</a> to \
	make is specific to the current search database. \
" }, \
{ "nph", "false",  \
	"boolean", "htsearch", "", "3.2.0b2", "Presentation:How", "nph: true", " \
	This attribute determines whether htsearch sends out full HTTP \
	headers as required for an NPH (non-parsed header) CGI. Some \
	servers assume CGIs will act in this fashion, for example MS \
	IIS. If your server does not send out full HTTP headers, you \
	should set this to true. \
" }, \
{ "page_list_header", "<hr noshade size=2>Pages:<br>",  \
	"string", "htsearch", "", "3.0", "Presentation:Text", "page_list_header:", " \
	This text will be used as the value of the PAGEHEADER \
	variable, for use in templates or the \
	<a href=\"#search_results_footer\">search_results_footer</a> \
	file, when all search results fit on more than one page. \
" }, \
{ "page_number_separator", "\" \"",  \
	"quoted string list", "htsearch", "", "3.1.4", "Presentation:Text", "page_number_separator: \"&lt;/td&gt; &lt;td&gt;\"", " \
	The text strings in this list will be used when putting \
	together the PAGELIST variable, for use in templates or \
	the <a href=\"#search_results_footer\">search_results_footer</a> \
	file, when search results fit on more than page. The PAGELIST \
	is the list of links at the bottom of the search results page. \
	The strings in the list will be used in rotation, and will \
	separate individual entries taken from \
	<a href=\"#page_number_text\">page_number_text</a> and \
	<a href=\"#no_page_number_text\">no_page_number_text</a>. \
	There can be as many or as few strings in the list as you like. \
	If there are not enough for the number of pages listed, it goes \
	back to the start of the list. If the list is empty, a space is \
	used. The text strings can contain HTML tags. The strings need \
	to be quoted if they contain spaces, or to specify an empty string. \
" }, \
{ "page_number_text", "",  \
	"quoted string list", "htsearch", "", "3.0", "Presentation:Text", "page_number_text: \
				  &lt;em&gt;1&lt;/em&gt; &lt;em&gt;2&lt;/em&gt; \\<br> \
				  &lt;em&gt;3&lt;/em&gt; &lt;em&gt;4&lt;/em&gt; \\<br> \
				  &lt;em&gt;5&lt;/em&gt; &lt;em&gt;6&lt;/em&gt; \\<br> \
				  &lt;em&gt;7&lt;/em&gt; &lt;em&gt;8&lt;/em&gt; \\<br> \
				  &lt;em&gt;9&lt;/em&gt; &lt;em&gt;10&lt;/em&gt; \
", " \
	The text strings in this list will be used when putting \
	together the PAGELIST variable, for use in templates or \
	the <a href=\"#search_results_footer\">search_results_footer</a> \
	file, when search results fit on more than page. The PAGELIST \
	is the list of links at the bottom of the search results page. \
	There should be as many strings in the list as there are \
	pages allowed by the <a href=\"#maximum_page_buttons\">maximum_page_buttons</a> \
	attribute. If there are not enough, or the list is empty, \
	the page numbers alone will be used as the text for the links. \
	Entries from this list are used for the links to other pages, \
	while an entry from the <a href=\"#no_page_number_text\"> \
	no_page_number_text</a> list is used for the current page, as the \
	current page is shown in the page list without a hypertext link. \
	The text strings can contain HTML tags to highlight page numbers \
	or embed images. The strings need to be quoted if they contain \
	spaces. \
" }, \
{ "persistent_connections", "true",  \
	"boolean", "htdig", "Server", "3.2.0b1", "Indexing:Connection", "persistent_connections: false", " \
	If set to true, when servers make it possible, htdig can take advantage \
	of persistent connections, as defined by HTTP/1.1 (<em>RFC2616</em>). This permits \
	to reduce the number of open/close operations of connections, when retrieving \
	a document with HTTP. \
" }, \
{ "plural_suffix", "s", \
	"string", "htsearch", "", "3.2.0b2", "Presentation: Text", "plural_suffix: en", " \
	Specifies the value of the PLURAL_MATCHES template \
	variable used in the header, footer and template files. \
	This can be used for localization for non-English languages \
	where 's' is not the appropriate suffix. \
" }, \
{ "prefix_match_character", "*",  \
	"string", "htsearch", "", "3.1.0b1", "Searching:Method", "prefix_match_character: ing", " \
	A null prefix character means that prefix matching should be \
	applied to every search word. Otherwise prefix matching is \
	done on any search word ending with the characters specified \
	in this string, with the string being stripped off before \
	looking for matches. The \"prefix\" algorithm must be enabled \
	in <a href=\"#search_algorithm\">search_algorithm</a> \
	for this to work. You may also want to set the <a \
	href=\"#max_prefix_matches\">max_prefix_matches</a> and <a \
	href=\"#minimum_prefix_length\">minimum_prefix_length</a> attributes \
	to get it working as you want.<br> As a special case, in version \
	3.1.6 and later, if this string is non-null and is entered alone \
	as a search word, it is taken as a wildcard that matches all \
	documents in the database. If this string is null, the wildcard \
	for this special case will be <strong>*</strong>. This wildcard \
	doesn't require the prefix algorithm to be enabled. \
" }, \
{ "prev_page_text", "[prev]",  \
	"string", "htsearch", "", "3.0", "Presentation:Text", "prev_page_text: &lt;img src=\"/htdig/buttonl.gif\"&gt;", " \
	The text displayed in the hyperlink to go to the \
	previous page of matches. \
" }, \
{ "regex_max_words", "25",  \
	"integer", "htsearch", "", "3.2.0b1", "Searching:Method", "regex_max_words: 10", " \
	The \"regex\" <a href=\"#search_algorithm\">fuzzy algorithm</a> \
	could potentially match a \
	very large number of words. This value limits the \
	number of words each regular expression can match. Note \
	that this does not limit the number of documents that \
	are matched in any way. \
" }, \
{ "remove_bad_urls", "true",  \
	"boolean", "htpurge", "Server", "all", "Indexing:How", "remove_bad_urls: true", " \
	If TRUE, htpurge will remove any URLs which were marked \
	as unreachable by htdig from the database. If FALSE, it \
	will not do this. When htdig is run in initial mode, \
	documents which were referred to but could not be \
	accessed should probably be removed, and hence this \
	option should then be set to TRUE, however, if htdig is \
	run to update the database, this may cause documents on \
	a server which is temporarily unavailable to be \
	removed. This is probably NOT what was intended, so \
	hence this option should be set to FALSE in that case. \
" }, \
{ "remove_default_doc", "index.html",  \
	"string list", "htdig", "", "3.1.0", "Indexing:How", "remove_default_doc: default.html default.htm index.html index.htm", " \
	Set this to the default documents in a directory used by the \
	servers you are indexing. These document names will be stripped \
	off of URLs when they are normalized, if one of these names appears \
	after the final slash, to translate URLs like \
	http://foo.com/index.html into http://foo.com/<br> \
	Note that you can disable stripping of these names during \
	normalization by setting the list to an empty string. \
	The list should only contain names that all servers you index \
	recognize as default documents for directory URLs, as defined \
	by the DirectoryIndex setting in Apache's srm.conf, for example. \
	This does not apply to  file:///  or  ftp://  URLS. \
	<br>See also <a href=\"#local_default_doc\">local_default_doc</a>. \
" }, \
{ "remove_unretrieved_urls", "false",  \
	"boolean", "htpurge", "Server", "3.2.0b1", "Indexing:How", "remove_unretrieved_urls: true", " \
	If TRUE, htpurge will remove any URLs which were discovered \
	and included as stubs in the database but not yet retrieved. If FALSE, it \
	will not do this. When htdig is run in initial mode with no restrictions  \
	on hopcount or maximum documents, these should probably be removed and set \
	to true. However, if you are hoping to index a small set of documents and  \
	eventually get to the rest, you should probably leave this as false. \
" }, \
{ "restrict", "",  \
	"pattern list", "htsearch", "", "3.2.0b4", "Searching:Method", "restrict: http://www.acme.com/widgets/", " \
	This specifies a set of patterns that all URLs have to \
	match against in order for them to be included in the search \
	results. Any number of strings can be specified, separated by \
	spaces. If multiple patterns are given, at least one of the \
	patterns has to match the URL. The list can be specified \
	from within the configuration file, and can be overridden \
	with the \"restrict\" input parameter in the search form. Note \
	that the restrict list does not take precedence over the \
	<a href=\"#exclude\">exclude</a> list - if a URL matches patterns \
	in both lists it is still excluded from the search results. \
	<br>To restrict URLs in htdig, use \
	<a href=\"limit_urls_to\">limit_urls_to</a>. \
" }, \
{ "robotstxt_name", "htdig",  \
	"string", "htdig", "Server", "3.0.7", "Indexing:Out", "robotstxt_name: myhtdig", " \
	Sets the name that htdig will look for when parsing \
	robots.txt files. This can be used to make htdig appear \
	as a different spider than ht://Dig. Useful to \
	distinguish between a private and a global index. \
" }, \
{ "script_name", "",  \
	"string", "htsearch", "", "3.1.4", "Presentation:Text", "script_name: /search/results.shtml", " \
	Overrides the value of the SCRIPT_NAME \
	environment attribute. This is useful if \
	htsearch is not being called directly as a CGI \
	program, but indirectly from within a dynamic \
	.shtml page using SSI directives. Previously, \
	you needed a wrapper script to do this, but \
	this configuration attribute makes wrapper \
	scripts obsolete for SSI and possibly for \
	other server scripting languages, as \
	well. (You still need a wrapper script when \
	using PHP, though.)<br> \
	Check out the <code>contrib/scriptname</code> \
	directory for a small example. Note that this \
	attribute also affects the value of the <a \
	href=\"hts_templates.html#CGI\">CGI</a> variable \
	used in htsearch templates. \
" }, \
{ "search_algorithm", "exact:1",  \
	"string list", "htsearch", "", "all", "Searching:Method", "search_algorithm: exact:1 soundex:0.3", " \
			Specifies the search algorithms and their weight to use \
			when searching. Each entry in the list consists of the \
			algorithm name, followed by a colon (:) followed by a \
			weight multiplier. The multiplier is a floating point \
			number between 0 and 1. Note that depending on your \
			<a href=\"#locale\">locale</a> setting, and whether your \
			system's locale implementation affects floating point \
			input, you may need to specify the decimal point as a \
			comma rather than a period.<br> \
			<strong>Note:</strong>If the exact  \
			method is not listed, the search may not work since the  \
			original terms will not be used.<br> \
			Current algorithms supported are: \
			<dl> \
			  <dt> \
				exact \
			  </dt> \
			  <dd> \
				The default exact word matching algorithm. This \
				will find only exactly matched words. \
			  </dd> \
			  <dt> \
				soundex \
			  </dt> \
			  <dd> \
				Uses a slightly modified <a href=\"http://www.sog.org.uk/cig/vol6/605tdrake.pdf\">soundex</a> algorithm to match \
				words. This requires that the soundex database be \
				present. It is generated with the \
				<a href=\"htfuzzy.html\">htfuzzy</a> program. \
			  </dd> \
			  <dt> \
				metaphone \
			  </dt> \
			  <dd> \
				Uses the metaphone algorithm for matching words. \
				This algorithm is more specific to the english \
				language than soundex. It requires the metaphone \
				database, which is generated with the <a \
				href=\"htfuzzy.html\">htfuzzy</a> program. \
			  </dd> \
			  <dt> \
				accents \
			  </dt> \
			  <dd> \
				Uses the accents algorithm for matching words. \
				This algorithm will treat all accented letters \
				as equivalent to their unaccented counterparts. \
				It requires the accents database, which is \
				generated with the <a \
				href=\"htfuzzy.html\">htfuzzy</a> program. \
			  </dd> \
			  <dt> \
				endings \
			  </dt> \
			  <dd> \
				This algorithm uses language specific word endings \
				to find matches. Each word is first reduced to its \
				word root and then all known legal endings are used \
				for the matching. This algorithm uses two databases \
				which are generated with <a href=\"htfuzzy.html\"> \
				htfuzzy</a>. \
			  </dd> \
			  <dt> \
				synonyms \
			  </dt> \
			  <dd> \
				Performs a dictionary lookup on all the words. This \
				algorithm uses a database generated with the <a \
				href=\"htfuzzy.html\">htfuzzy</a> program. \
			  </dd> \
			<dt> \
			substring \
			</dt> \
			<dd> \
			  Matches all words containing the queries as \
			  substrings. Since this requires checking every word in \
			  the database, this can really slow down searches \
			  considerably. \
			<dd> \
			<dt> \
			  prefix \
			</dt> \
			<dd> \
			  Matches all words beginning with the query \
			  strings. Uses the option <a \
			  href=\"#prefix_match_character\">prefix_match_character</a> \
			  to decide whether a query requires prefix \
			  matching. For example \"abc*\" would perform prefix \
			  matching on \"abc\" since * is the default \
			  prefix_match_character. \
			</dd> \
			<dt> \
			regex \
			</dt> \
			<dd> \
			  Matches all words that match the patterns given as regular  \
			  expressions. Since this requires checking every word in \
			  the database, this can really slow down searches \
			  considerably.  The config file used for searching \
			  must include the regex meta-characters (^$\\[-]|.*) \
			  included in <a href=\"extra_word_characters\">extra_word_characters</a>, \
			  while the config file used for digging should not.\
			<dd> \
			<dt> \
			speling \
			</dt> \
			<dd> \
			  A simple fuzzy algorithm that tries to find one-off spelling  \
			  mistakes, such as transposition of two letters or an extra character. \
			  Since this usually generates just a few possibilities, it is  \
			  relatively quick. \
			<dd> \
			</dl> \
" }, \
{ "search_results_contenttype", "text/html",  \
	"string", "htsearch", "", "all", "Presentation:Files", "search_results_contenttype: text/xml", " \
	This specifies a Content-type to be output as an HTTP header \
	at the start of search results. If set to an empty string, \
	the Content-type header will be omitted altogether. \
" },
{ "search_results_footer", "${common_dir}/footer.html",  \
	"string", "htsearch", "", "all", "Presentation:Files", "search_results_footer: /usr/local/etc/ht/end-stuff.html", " \
			This specifies a filename to be output at the end of \
			search results. While outputting the footer, some \
			variables will be expanded. Variables use the same \
			syntax as the Bourne shell. If there is a variable VAR, \
			the following will all be recognized: \
			<ul> \
			  <li> \
				$VAR \
			  </li> \
			  <li> \
				$(VAR) \
			  </li> \
			  <li> \
				${VAR} \
			  </li> \
			</ul> \
	The following variables are available.  See \
	<a href=\"hts_template.html\">hts_template.html</a> for a complete \
	list. \
			<dl> \
			  <dt> \
				MATCHES \
			  </dt> \
			  <dd> \
				The number of documents that were matched. \
			  </dd> \
			  <dt> \
				PLURAL_MATCHES \
			  </dt> \
			  <dd> \
				If MATCHES is not 1, this will be the string \"s\", \
				else it is an empty string. This can be used to say \
				something like \"$(MATCHES) \
				document$(PLURAL_MATCHES) were found\" \
			  </dd> \
			  <dt> \
				MAX_STARS \
			  </dt> \
			  <dd> \
				The value of the <a href=\"#max_stars\">max_stars</a> \
				attribute. \
			  </dd> \
			  <dt> \
				LOGICAL_WORDS \
			  </dt> \
			  <dd> \
				A string of the search words with either \"and\" or \
				\"or\" between the words, depending on the type of \
				search. \
			  </dd> \
			  <dt> \
				WORDS \
			  </dt> \
			  <dd> \
				A string of the search words with spaces in \
				between. \
			  </dd> \
			  <dt> \
				PAGEHEADER \
			  </dt> \
			  <dd> \
				This expands to either the value of the \
				<a href=\"#page_list_header\">page_list_header</a> or \
				<a href=\"#no_page_list_header\">no_page_list_header</a> \
				attribute depending on how many pages there are. \
			  </dd> \
			</dl> \
			Note that this file will <strong>NOT</strong> be output \
			if no matches were found. In this case the \
			<a href=\"#nothing_found_file\">nothing_found_file</a> \
			attribute is used instead. \
			Also, this file will not be output if it is \
			overridden by defining the \
			<a href=\"#search_results_wrapper\">search_results_wrapper</a> \
			attribute. \
" }, \
{ "search_results_header", "${common_dir}/header.html",  \
	"string", "htsearch", "", "all", "Presentation:Files", "search_results_header: /usr/local/etc/ht/start-stuff.html", " \
			This specifies a filename to be output at the start of \
			search results. While outputting the header, some \
			variables will be expanded. Variables use the same \
			syntax as the Bourne shell. If there is a variable VAR, \
			the following will all be recognized: \
			<ul> \
			  <li> \
				$VAR \
			  </li> \
			  <li> \
				$(VAR) \
			  </li> \
			  <li> \
				${VAR} \
			  </li> \
			</ul> \
	The following variables are available.  See \
	<a href=\"hts_template.html\">hts_template.html</a> for a complete \
	list. \
	<!-- Do these need to be listed for both _footer and _header? --> \
			<dl> \
			  <dt> \
				MATCHES \
			  </dt> \
			  <dd> \
				The number of documents that were matched. \
			  </dd> \
			  <dt> \
				PLURAL_MATCHES \
			  </dt> \
			  <dd> \
				If MATCHES is not 1, this will be the string \"s\", \
				else it is an empty string. This can be used to say \
				something like \"$(MATCHES) \
				document$(PLURAL_MATCHES) were found\" \
			  </dd> \
			  <dt> \
				MAX_STARS \
			  </dt> \
			  <dd> \
				The value of the <a href=\"#max_stars\">max_stars</a> \
				attribute. \
			  </dd> \
			  <dt> \
				LOGICAL_WORDS \
			  </dt> \
			  <dd> \
				A string of the search words with either \"and\" or \
				\"or\" between the words, depending on the type of \
				search. \
			  </dd> \
			  <dt> \
				WORDS \
			  </dt> \
			  <dd> \
				A string of the search words with spaces in \
				between. \
			  </dd> \
			</dl> \
			Note that this file will <strong>NOT</strong> be output \
			if no matches were found. In this case the \
			<a href=\"#nothing_found_file\">nothing_found_file</a> \
			attribute is used instead. \
			Also, this file will not be output if it is \
			overridden by defining the \
			<a href=\"#search_results_wrapper\">search_results_wrapper</a> \
			attribute. \
" }, \
{ "search_results_order", "", \
	"string list", "htsearch", "", "3.2.0b2", "Searching:Ranking", "search_results_order:  \
	 /docs/|faq.html * /maillist/ /testresults/", " \
	This specifies a list of patterns for URLs in \
	search results.  Results will be displayed in the \
	specified order, with the search algorithm result \
	as the second order.  Remaining areas, that do not \
	match any of the specified patterns, can be placed \
	by using * as the pattern.  If no * is specified, \
	one will be implicitly placed at the end of the \
	list.<br> \
	See also <a href=\"#url_seed_score\">url_seed_score</a>. \
" }, \
{ "search_results_wrapper", "",  \
	"string", "htsearch", "", "3.1.0", "Presentation:Files", "search_results_wrapper: ${common_dir}/wrapper.html", " \
	This specifies a filename to be output at the start and \
	end of search results. This file replaces the \
	<a href=\"#search_results_header\">search_results_header</a> and \
	<a href=\"#search_results_footer\">search_results_footer</a> \
	files, with the contents of both in one file, and uses the \
	pseudo-variable <strong>$(HTSEARCH_RESULTS)</strong> as a \
	separator for the header and footer sections. \
	If the filename is not specified, the file is unreadable, \
	or the pseudo-variable above is not found, htsearch reverts \
	to the separate header and footer files instead. \
	While outputting the wrapper, \
	some variables will be expanded, just as for the \
	<a href=\"#search_results_header\">search_results_header</a> and \
	<a href=\"#search_results_footer\">search_results_footer</a> \
	files.<br> \
	Note that this file will <strong>NOT</strong> be output \
	if no matches were found. In this case the \
	<a href=\"#nothing_found_file\">nothing_found_file</a> \
	attribute is used instead. \
" }, \
{ "search_rewrite_rules", "",
	"string list", "htsearch", "", "3.1.6", "URLs", "search_rewrite_rules: http://(.*)\\\\.mydomain\\\\.org/([^/]*)  http://\\\\2.\\\\1.com \\<br> \
	       http://www\\\\.myschool\\\\.edu/myorgs/([^/]*)  http://\\\\1.org", " \
	This is a list of pairs, <em>regex</em> <em>replacement</em>, used \
	to rewrite URLs in the search results. The left hand string is a \
	regular expression; the right hand string is a literal string with \
	embedded placeholders for fragments that matched inside brackets in \
	the regular expression. \\0 is the whole matched string, \\1 to \\9 \
	are bracketted substrings. The backslash must be doubled-up in the \
	attribute setting to get past the variable expansion parsing. Rewrite \
	rules are applied sequentially to each URL before it is displayed \
	or checked against the <a href=\"#restrict\">restrict</a> or \
	<a href=\"#exclude\">exclude</a> lists. Rewriting does not stop once a \
	match has been made, so multiple rules may affect a given URL. See \
	also <a href=\"#url_part_aliases\">url_part_aliases</a> which allows \
	URLs to be of one form during indexing and translated for results, \
	and <a href=\"#url_rewrite_rules\">url_rewrite_rules</a> which allows \
	URLs to be rewritten while indexing. \
" },
{ "server_aliases", "",  \
	"string list", "htdig", "", "3.1.0b2", "Indexing:Where", "server_aliases: \
				  foo.mydomain.com:80=www.mydomain.com:80 \\<br> \
				  bar.mydomain.com:80=www.mydomain.com:80 \
", " \
	This attribute tells the indexer that servers have several \
	DNS aliases, which all point to the same machine and are NOT \
	virtual hosts. This allows you to ensure pages are indexed \
	only once on a given machine, despite the alias used in a URL. \
	As shown in the example, the mapping goes from left to right, \
	so the server name on the right hand side is the one that is \
	used. As of version 3.1.3, the port number is optional, and is \
	assumed to be 80 if omitted. There is no easy way to map all \
	ports from one alias to another without listing them all. \
" }, \
{ "server_max_docs", "-1",  \
	"integer", "htdig", "Server", "3.1.0b3", "Indexing:Where", "server_max_docs: 50", " \
	This attribute tells htdig to limit the dig to retrieve a maximum \
	number of documents from each server. This can cause \
	unusual behavior on update digs since the old URLs are \
	stored alphabetically. Therefore, update digs will add \
	additional URLs in pseudo-alphabetical order, up to the \
	limit of the attribute. However, it is most useful to \
	partially index a server as the URLs of additional \
	documents are entered into the database, marked as never \
	retrieved.<br> \
	A value of -1 specifies no limit. \
" }, \
{ "server_wait_time", "0",  \
	"integer", "htdig", "Server", "3.1.0b3", "Indexing:Connection", "server_wait_time: 20", " \
	This attribute tells htdig to ensure a server has had a \
	delay (in seconds) from the beginning of the last \
	connection. This can be used to prevent \"server abuse\" \
	by digging without delay. It's recommended to set this \
	to 10-30 (seconds) when indexing servers that you don't \
	monitor yourself. Additionally, this attribute can slow \
	down local indexing if set, which may or may not be what \
	you intended. \
" }, \
{ "sort", "score",  \
	"string", "htsearch", "", "3.1.0", "Presentation:How", "sort: revtime", " \
	This is the default sorting method that htsearch \
	uses to determine the order in which matches are displayed. \
	The valid choices are: \
	<table border=\"0\"> \
	<tr> \
	<td> \
	<ul> \
	     <li> score </li> \
	     <li> time </li> \
	     <li> title </li> \
	</ul> \
	</td> \
	<td> \
	<ul> \
	     <li> revscore </li> \
	     <li> revtime </li> \
	     <li> revtitle </li> \
	</ul> \
	</td> \
	</tr> \
	</table> \
	This attribute will only be used if the HTML form that \
	calls htsearch didn't have the <strong>sort</strong> \
	value set. The words date and revdate can be used instead \
	of time and revtime, as both will sort by the time that \
	the document was last modified, if this information is \
	given by the server. The default is to sort by the score, \
	which ranks documents by best match. The sort methods that \
	begin with \"rev\" simply reverse the order of the \
	sort. Note that setting this to something other than \
	\"score\" will incur a slowdown in searches. \
" }, \
{ "sort_names", "score Score time Time title Title revscore 'Reverse Score' revtime 'Reverse Time' revtitle 'Reverse Title'",  \
	"quoted string list", "htsearch", "", "3.1.0", "Searching:UI", "sort_names: \
				  score 'Best Match' time Newest title A-Z \\<br> \
				  revscore 'Worst Match' revtime Oldest revtitle Z-A \
", " \
	These values are used to create the <strong> \
	sort</strong> menu. It consists of pairs. The first \
	element of each pair is one of the known sort methods, the \
	second element is the text that will be shown in the \
	menu for that sort method. This text needs to be quoted if \
	it contains spaces. \
	See the <a href=\"hts_selectors.html\">select list documentation</a> \
	for more information on how this attribute is used. \
" }, \
{ "soundex_db", "${database_base}.soundex.db",  \
	"string", "htfuzzy htsearch", "", "all", "File Layout", "soundex_db: ${database_base}.snd.db", " \
	The database file used for the fuzzy \"soundex\" search \
	algorithm. This database is created by \
	<a href=\"htfuzzy.html\">htfuzzy</a> and used by \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>. \
" }, \
{ "star_blank", "${image_url_prefix}/star_blank.gif",  \
	"string", "htsearch", "", "all", "Presentation:Text", "star_blank: http://www.somewhere.org/icons/noelephant.gif", " \
	This specifies the URL to use to display a blank of the \
	same size as the star defined in the \
	<a href=\"#star_image\">star_image</a> attribute or in the \
	<a href=\"#star_patterns\">star_patterns</a> attribute. \
" }, \
{ "star_image", "${image_url_prefix}/star.gif",  \
	"string", "htsearch", "", "all", "Presentation:Text", "star_image: http://www.somewhere.org/icons/elephant.gif", " \
	This specifies the URL to use to display a star. This \
	allows you to use some other icon instead of a star. \
	(We like the star...)<br> \
	The display of stars can be turned on or off with the \
	<em><a href=\"#use_star_image\">use_star_image</a></em> \
	attribute and the maximum number of stars that can be \
	displayed is determined by the \
	<em><a href=\"#max_stars\">max_stars</a></em> attribute.<br> \
	Even though the image can be changed, the ALT value \
	for the image will always be a '*'. \
" }, \
{ "star_patterns", "",  \
	"string list", "htsearch", "", "3.0", "Presentation:How", "star_patterns: \
				  http://www.sdsu.edu /sdsu.gif \\<br> \
				  http://www.ucsd.edu /ucsd.gif \
", " \
	This attribute allows the star image to be changed \
	depending on the URL or the match it is used for. This \
	is mainly to make a visual distinction between matches \
	on different web sites. The star image could be \
	replaced with the logo of the company the match refers \
	to.<br> \
	It is advisable to keep all the images the same size \
	in order to line things up properly in a short result \
	listing.<br> \
	The format is simple. It is a list of pairs. The first \
	element of each pair is a pattern, the second element \
	is a URL to the image for that pattern. \
" }, \
{ "startday", "",  \
	"integer", "htsearch", "", "3.1.6", "Searching:Method", "startday: 1", " \
	Day component of first date allowed as last-modified date \
	of returned docutments. \
	This is most usefully specified as a \
	<a href=\"hts_form.html#startyear\">GCI argument</a>. \
	See also <a href=\"#startyear\">startyear</a>. \
" }, \
{ "start_ellipses", "<strong><code>... </code></strong>",  \
	"string", "htsearch", "", "all", "Presentation:Text", "start_ellipses: ...", " \
	When excerpts are displayed in the search output, this \
	string will be prepended to the excerpt if there is \
	text before the text displayed. This is just a visual \
	reminder to the user that the excerpt is only part of \
	the complete document. \
" }, \
{ "start_highlight", "<strong>",  \
	"string", "htsearch", "", "3.1.4", "Presentation:Text", "start_highlight: &lt;font color=\"#FF0000\"&gt;", " \
	When excerpts are displayed in the search output, matched \
	words will be highlighted using this string and \
	<a href=\"#end_highlight\"> end_highlight</a>. \
	You should ensure that highlighting tags are balanced, \
	that is, any formatting tags that this string \
	opens should be closed by end_highlight. \
" }, \
{ "startmonth", "",  \
	"integer", "htsearch", "", "3.1.6", "Searching:Method", "startmonth: 1", " \
	Month component of first date allowed as last-modified date \
	of returned docutments. \
	This is most usefully specified as a \
	<a href=\"hts_form.html#startyear\">GCI argument</a>. \
	See also <a href=\"#startyear\">startyear</a>. \
" }, \
{ "start_url", "http://www.htdig.org/",  \
	"string list", "htdig", "", "all", "Indexing:Where", "start_url: http://www.somewhere.org/alldata/index.html", " \
	This is the list of URLs that will be used to start a \
	dig when there was no existing database. Note that \
	multiple URLs can be given here. \
	<br>Note also that the value of <em>start_url</em> \
	will be the default value for \
	<a href=\"#limit_urls_to\">limit_urls_to</a>, so if \
	you set start_url to the URLs for specific files, \
	rather than a site or subdirectory URL, you may need \
	to set limit_urls_to to something less restrictive \
	so htdig doesn't reject links in the documents. \
" }, \
{ "startyear", "1970",  \
	"integer", "htsearch", "", "3.1.6", "Searching:Method", "startyear: 2001", " \
	This specifies the year of the cutoff start date for \
	search results. If the start or end date are specified, \
	only results with a last modified date within this \
	range are shown. \
	See also <a href=\"#startday\">startday</a>, \
	<a href=\"#startmonth\">startmonth</a>, \
	<a href=\"#endday\">endday</a>, \
	<a href=\"#endmonth\">endmonth</a>, \
	<a href=\"endyear\">endyear</a>. \
	These are most usefully specified as a \
	<a href=\"hts_form.html#startyear\">GCI argument</a>.<br> \
	For each component, if a negative number is given, \
	it is taken as relative to the current date. \
	Relative days can span several months or even years if desired, \
	and relative months can span several years. A startday of \
	-90 will select matching documents modified within \
	the last 90 days. \
" }, \
{ "substring_max_words", "25",  \
	"integer", "htsearch", "", "3.0.8b1", "Searching:Method", "substring_max_words: 100", " \
	The Substring <a href=\"#search_algorithm\">fuzzy algorithm</a> \
	could potentially match a \
	very large number of words. This value limits the \
	number of words each substring pattern can match. Note \
	that this does not limit the number of documents that \
	are matched in any way. \
" }, \
{ "synonym_db", "${common_dir}/synonyms.db",  \
	"string", "htsearch htfuzzy", "", "3.0", "File Layout", "synonym_db: ${database_base}.syn.db", " \
	Points to the database that <a href=\"htfuzzy.html\"> \
	htfuzzy</a> creates when the <strong>synonyms</strong> \
	algorithm is used.<br> \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a> \
	uses this to perform synonym dictionary lookups. \
" }, \
{ "synonym_dictionary", "${common_dir}/synonyms",  \
	"string", "htfuzzy", "", "3.0", "File Layout", "synonym_dictionary: /usr/dict/synonyms", " \
	This points to a text file containing the synonym \
	dictionary used for the synonyms search algorithm.<br> \
	Each line of this file has at least two words. The \
	first word is the word to replace, the rest of the \
	words are synonyms for that word. \
" }, \
{ "syntax_error_file", "${common_dir}/syntax.html",  \
	"string", "htsearch", "", "all", "Presentation:Files", "syntax_error_file: ${common_dir}/synerror.html", " \
	This points to the file which will be displayed if a \
	boolean expression syntax error was found. \
" }, \
{ "tcp_max_retries", "1",  \
	"integer", "htdig", "Server", "3.2.0b1", "Indexing:Connection", "tcp_max_retries: 6", " \
	 This option set the maximum number of attempts when a connection \
	 <A href=\"#timeout\">timeout</A>s. \
	 After all these retries, the connection attempt results <timed out>. \
" }, \
{ "tcp_wait_time", "5",  \
	"integer", "htdig", "Server", "3.2.0b1", "Indexing:Connection", "tcp_wait_time: 10", " \
	 This attribute sets the wait time (in seconds) after a connection \
	 fails and the <A href=\"#timeout\">timeout</A> is raised. \
" }, \
{ "template_map", "Long builtin-long builtin-long Short builtin-short builtin-short",  \
	"quoted string list", "htsearch", "", "3.0", "Presentation:Files,Searching:UI", "template_map: \
				  Short short ${common_dir}/short.html \\<br> \
				  Normal normal builtin-long \\<br> \
				  Detailed detail ${common_dir}/detail.html \
", " \
	This maps match template names to internal names and \
	template file names. It is a list of triplets. The \
	first element in each triplet is the name that will be \
	displayed in the FORMAT menu. The second element is the \
	name used internally and the third element is a \
	filename of the template to use.<br> \
	There are two predefined templates, namely <strong> \
	builtin-long</strong> and <strong> \
	builtin-short</strong>. If the filename is one of \
	those, they will be used instead.<br> \
	More information about templates can be found in the \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a> \
	documentation.  The particular template is selecterd by the \
	<a href=\"hts_form.html#format\">format</a> cgi argument, and the \
	default is given by <a href=\"#template_name\">template_name</a> in \
	the config file. \
" }, \
{ "template_name", "builtin-long",  \
	"string", "htsearch", "", "3.0", "Searching:UI,Presentation:How", "template_name: long", " \
	Specifies the default template if no \
	<a href=\"hts_form.html#format\">format</a> field is given by the \
	search form. This needs to map to the \
	<a href=\"#template_map\">template_map</a>. \
" }, \
{ "template_patterns", "",  \
	"string list", "htsearch", "", "3.1.4", "Presentation:How", "template_patterns: \
				  http://www.sdsu.edu ${common_dir}/sdsu.html \\<br> \
				  http://www.ucsd.edu ${common_dir}/ucsd.html \
", " \
	This attribute allows the results template to be changed \
	depending on the URL or the match it is used for. This \
	is mainly to make a visual distinction between matches \
	on different web sites. The results for each site could \
	thus be shown in a style matching that site.<br> \
	The format is simply a list of pairs. The first \
	element of each pair is a pattern, the second element \
	is the name of the template file for that pattern.<br> \
	More information about templates can be found in the \
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a> \
	documentation.<br> \
	Normally, when using this template selection method, you \
	would disable user selection of templates via the <strong>format</strong> \
	input parameter in search forms, as the two methods were not \
	really designed to interact. Templates selected by URL patterns \
	would override any user selection made in the form. If you want \
	to use the two methods together, see the notes on \
	<a href=\"hts_selectors.html#template_patterns\">combining</a> \
	them for an example of how to do this. \
" }, \
{ "text_factor", "1",  \
	"number", "htsearch", "", "3.0", "Searching:Ranking", "text_factor: 0", " \
	This is a factor which will be used to multiply the \
	weight of words that are not in any special part of a \
	document. Setting a factor to 0 will cause normal words \
	to be ignored. The number may be a floating point \
	number. See also the <a href=\"#heading_factor\"> heading_factor</a> \
	attribute. \
" }, \
{ "timeout", "30",  \
	"integer", "htdig", "Server", "all", "Indexing:Connection", "timeout: 42", " \
	Specifies the time the digger will wait to complete a \
	network read. This is just a safeguard against \
	unforeseen things like the all too common \
	transformation from a network to a notwork.<br> \
	The timeout is specified in seconds. \
" }, \
{ "title_factor", "100",  \
	"number", "htsearch", "", "all", "Searching:Ranking", "title_factor: 12", " \
	This is a factor which will be used to multiply the \
	weight of words in the title of a document. Setting a \
	factor to 0 will cause words in the title to be \
	ignored. The number may be a floating point number. See \
	also the <a href=\"#heading_factor\"> \
	heading_factor</a> attribute. \
" }, \
{ "translate_latin1", "true",  \
	"boolean", "htdig htsearch", "", "3.2.0b5", "Indexing:What", "translate_latin1: false", " \
	If set to false, the SGML entities for ISO-8859-1 (or \
	Latin 1) characters above &amp;nbsp; (or &amp;#160;) \
	will not be translated into their 8-bit equivalents. \
	This attribute should be set to false when using a \
	<a href=\"#locale\">locale</a> that doesn't use the \
	ISO-8859-1 character set, to avoid these entities \
	being mapped to inappropriate 8-bit characters, or \
	perhaps more importantly to avoid 8-bit characters from \
	your locale being mapped back to Latin 1 SGML entities \
	in search results. \
" }, \
{ "url_list", "${database_base}.urls",  \
	"string", "htdig", "", "all", "Extra Output", "url_list: /tmp/urls", " \
	This file is only created if \
	<em><a href=\"#create_url_list\">create_url_list</a></em> is set to \
	true. It will contain a list of all URLs that were \
	seen. \
" }, \
{ "url_log", "${database_base}.log",  \
	"string", "htdig", "", "3.1.0", "Extra Output", "url_log: /tmp/htdig.progress", " \
	If <a href=\"htdig.html\">htdig</a> is run with the -l option \
	and interrupted, it will write out its progress to this \
	file. Note that if it has a large number of URLs to write, \
	it may take some time to exit. This can especially happen \
	when running update digs and the run is interrupted soon \
	after beginning. \
" }, \
{ "url_part_aliases", "",  \
	"string list", "all", "", "3.1.0", "URLs", "url_part_aliases: \
				   http://search.example.com/~htdig *site \\<br> \
				   http://www.htdig.org/this/ *1 \\<br> \
				   .html *2 \
url_part_aliases: \
				   http://www.htdig.org/ *site \\<br> \
				   http://www.htdig.org/that/ *1 \\<br> \
				   .htm *2 \
", " \
	A list of translations pairs <em>from</em> and \
	<em>to</em>, used when accessing the database. \
	If a part of an URL matches with the \
	<em>from</em>-string of each pair, it will be \
	translated into the <em>to</em>-string just before \
	writing the URL to the database, and translated \
	back just after reading it from the database.<br> \
	This is primarily used to provide an easy way to \
	rename parts of URLs for e.g. changing \
	www.example.com/~htdig to www.htdig.org.  Two \
	different configuration files for digging and \
	searching are then used, with url_part_aliases \
	having different <em>from</em> strings, but \
	identical <em>to</em>-strings.<br> \
	See also <a \
	href=\"#common_url_parts\">common_url_parts</a>.<br> \
	Strings that are normally incorrect in URLs or \
	very seldom used, should be used as \
	<em>to</em>-strings, since extra storage will be \
	used each time one is found as normal part of a \
	URL.  Translations will be performed with priority \
	for the leftmost longest match.	 Each \
	<em>to</em>-string must be unique and not be a \
	part of any other <em>to</em>-string.  It also helps \
	to keep the <em>to</em>-strings short to save space \
	in the database. Other than that, the choice of \
	<em>to</em>-strings is pretty arbitrary, as they \
	just provide a temporary, internal encoding in the \
	databases, and none of the characters in these \
	strings have any special meaning.<br> \
	Note that when this attribute is changed, the \
	database should be rebuilt, unless the effect of \
	\"moving\" the affected URLs in the database is \
	wanted, as described above.<br> \
	<strong>Please note:</strong> Don't just copy the \
	example below into a single configuration file. \
	There are two separate settings of \
	<em>url_part_aliases</em> below; the first one is \
	for the configuration file to be used by htdig, \
	htmerge, and htnotify, and the second one is for the \
	configuration file to be used by htsearch. \
	In this example, htdig will encode the URL \
	\"http://search.example.com/~htdig/contrib/stuff.html\" \
	as \"*sitecontrib/stuff*2\" in the databases, and \
	htsearch will decode it as \
	\"http://www.htdig.org/contrib/stuff.htm\".<br> \
	As of version 3.1.6, you can also do more complex \
	rewriting of URLs using \
	<a href=\"#url_rewrite_rules\">url_rewrite_rules</a> and \
	<a href=\"#search_rewrite_rules\">search_rewrite_rules</a>. \
" }, \
{ "url_rewrite_rules", "", \
    "string list", "htdig", "", "3.2.0b3", "URLs", "url_rewrite_rules:	(.*)\\\\?JServSessionIdroot=.*		\\\\1 \\<br> \
			(.*)\\\\&amp;JServSessionIdroot=.*		\\\\1 \\<br> \
			(.*)&amp;context=.*				\\\\1<br>", " \
	This is a list of pairs, <em>regex</em> <em>replacement</em> used to \
	permanently rewrite URLs as they are indexed. The left hand string is \
	a regular expression; the right hand string is  a literal string with \
	embedded placeholders for fragments that matched  inside brackets in \
	the regex. \\0 is the whole matched string, \\1 to \\9 are  bracketted \
	substrings. Rewrite rules are applied sequentially to each  \
	incoming URL  before normalization occurs. Rewriting does not stop \
	once a match has been made, so multiple rules may affect a given URL. \
	See also <a href=\"#url_part_aliases\">url_part_aliases</a> which \
	allows URLs to be of one  \
form during indexing and translated for results. \
"}, \
{ "url_seed_score", "", \
    "string list", "htsearch", "", "3.2.0b2", "Searching::Ranking", "url_seed_score:  \
	      /mailinglist/ *.5-1e6 <br> \
	      /docs/|/news/ *1.5 <br> \
	      /testresults/ &quot;*.7 -200&quot; <br> \
	      /faq-area/ *2+10000", " \
	This is a list of pairs, <em>pattern</em> \
	<em>formula</em>, used to weigh the score of \
	hits, depending on the URL of the document.<br> \
	The <em>pattern</em> part is a substring to match \
	against the URL.  Pipe ('|') characters can be \
	used in the pattern to concatenate substrings for \
	web-areas that have the same formula.<br> \
	The formula describes a <em>factor</em> and a \
	<em>constant</em>, by which the hit score is \
	weighed.  The <em>factor</em> part is multiplied \
	to the original score, then the <em>constant</em> \
	part is added.<br> \
	The format of the formula is the factor part: \
	&quot;*<em>N</em>&quot; optionally followed by comma and \
	spaces, followed by the constant part : \
	&quot;+<em>M</em>&quot;, where the plus sign may be emitted \
	for negative numbers.  Either part is optional, \
	but must come in this order.<br> \
	The numbers <em>N</em> and <em>M</em> are floating \
	point constants.<br> \
	More straightforward is to think of the format as \
	&quot;newscore = oldscore*<em>N</em>+<em>M</em>&quot;, \
	but with the &quot;newscore = oldscore&quot; part left out. \
" }, \
{ "url_text_factor", "1",  \
	"number", "htsearch", "", "??", "Searching:Ranking", "url_text_factor: 1", " \
	TO BE COMPLETED<br> \
	See also <a href=\"#heading_factor\">heading_factor</a>. \
" }, \
{ "use_doc_date", "false",  \
	"boolean", "htdig", "", "3.2.0b1", "Indexing:How", "use_doc_date: true", " \
	If set to true, htdig will use META date tags in documents, \
	overriding the modification date returned by the server. \
	Any documents that do not have META date tags will retain \
	the last modified date returned by the server or found on \
	the local file system. \
	As of version 3.1.6, in addition to META date tags, htdig will also \
	recognize dc.date, dc.date.created and dc.date.modified. \
" }, \
{ "use_meta_description", "false",  \
	"boolean", "htsearch", "", "3.1.0b1", "Presentation:How", "use_meta_description: true", " \
	If set to true, any META description tags will be used as \
	excerpts by htsearch. Any documents that do not have META \
	descriptions will retain their normal excerpts. \
" }, \
{ "use_star_image", "true",  \
	"boolean", "htsearch", "", "all", "Presentation:How", "use_star_image: no", " \
	If set to true, the <em><a href=\"#star_image\"> \
	star_image</a></em> attribute is used to display upto \
	<em><a href=\"#max_stars\">max_stars</a></em> images for \
	each match. \
" }, \
{ "user_agent", "htdig",  \
	"string", "htdig", "Server", "3.1.0b2", "Indexing:Out", "user_agent: htdig-digger", " \
	This allows customization of the user_agent: field sent when \
	the digger requests a file from a server. \
" }, \
{ "valid_extensions", "",  \
	"string list", "htdig", "URL", "3.1.4", "Indexing:Where", "valid_extensions: .html .htm .shtml", " \
	This is a list of extensions on URLs which are \
	the only ones considered acceptable. This list is used to \
	supplement the MIME-types that the HTTP server provides \
	with documents. Some HTTP servers do not have a correct \
	list of MIME-types and so can advertise certain \
	documents as text while they are some binary format. \
	If the list is empty, then all extensions are acceptable, \
	provided they pass other criteria for acceptance or rejection. \
	If the list is not empty, only documents with one of the \
	extensions in the list are parsed. \
	See also <a href=\"#bad_extensions\">bad_extensions</a>. \
" }, \
{ "valid_punctuation", ".-_/!#\\$%^&'",  \
	"string", "htdig htsearch", "", "all", "Indexing:What", "valid_punctuation: -'", " \
	This is the set of characters which may be deleted \
	from the document before determining what a word is. \
	This means that if a document contains something like \
	<code>half-hearted</code> the digger will see this as the three \
	words <code> half</code>, <code>hearted</code> and \
	<code>halfhearted</code>.<br> \
	These characters are also removed before keywords are passed to the \
	search engine, so a search for \"half-hearted\" works as expected.<br> \
	Note that the dollar sign ($) and backslash (\\) must be escaped by a \
	backslash in both valid_punctuation and extra_word_characters. \
	Moreover, the backslash should not be the last character on the line. \
	There is currently no way to include a back-quote (`) in \
	extra_word_characters or valid_punctuation.<br> \
	See also the \
	<a href=\"#extra_word_characters\">extra_word_characters</a> \
	and <a href=\"#allow_numbers\">allow_numbers</a> \
	attributes.  \
" }, \
{ "version", VERSION,  \
	"string", "htsearch", "", "all", "Presentation:Text", "version: 3.2.0", " \
	This specifies the value of the VERSION \
	variable which can be used in search templates. \
	The default value of this attribute is determined \
	at compile time, and will not normally be set \
	in configuration files. \
" }, \
{ "word_db", "${database_base}.words.db",  \
	"string", "all", "", "all", "File Layout", "word_db: ${database_base}.allwords.db", " \
	This is the main word database. It is an index of all \
	the words to a list of documents that contain the \
	words. This database can grow large pretty quickly. \
" }, \
{ "word_dump", "${database_base}.worddump",  \
	"string", "htdig htdump htload", "", "3.2.0b1", "File Layout", "word_dump: /tmp/words.txt", " \
	This file is basically a text version of the file \
	specified in <em><a href=\"#word_db\">word_db</a></em>. Its \
	only use is to have a human readable database of all \
	words. The file is easy to parse with tools like \
	perl or tcl. \
" }, \
{ "wordlist_cache_dirty_level", "10000",  \
	"integer", "htdig", "", "3.2.0b4", "Indexing:How", "wordlist_cache_dirty_level: 2", " \
	Maximum ratio of dirty pages to clean pages in the cache.  If fewer \
	are clean, then all pages are written out (but kept in the cache). \
	Useful values are between 1 (slow, minimal chance of allocation ) \
	and about 3000 (fastest, but may cause problems with small page \
	sizes if <a href\"#wordlist_compress\">compression</a> is used).\
" }, \
{ "wordlist_cache_size", "10000000",  \
	"integer", "all", "", "3.2.0b1", "Indexing:How", "wordlist_cache_size: 40000000", " \
	Size (in bytes) of memory cache used by Berkeley DB (DB used by the indexer) \
	IMPORTANT: It  makes a <strong>huge</strong> difference. The rule  \
	is that the cache size should be at least 2% of the expected index size. The \
	Berkeley DB file has 1% of internal pages that <em>must</em> be cached for good \
	performances. Giving an additional 1% leaves room for caching leaf pages. \
" }, \
{ "wordlist_compress", "true",  \
	"boolean", "all", "", "3.2.0b1", "Indexing:How", "wordlist_compress: false", " \
	Enables or disables the default compression system for the indexer. \
	This currently attempts to compress the index by a factor of 8. If the \
	Zlib library is not found on the system, the default is false. \
" }, \
{ "wordlist_compress_zlib", "true",  \
	"boolean", "all", "", "3.2.0b4", "Indexing:How", "wordlist_compress_zlib: false", " \
	Enables or disables the zlib compression system for the indexer. \
	Both <a href=\"#wordlist_compress\">wordlist_compress</a> and \
	<a href=\"#compression_level\">compression_level</a> must be true \
	(non-zero) to use this option!\
" }, \
{ "wordlist_monitor", "false", \
	"boolean", "all", "", "3.2.0b1", "Extra Output", "wordlist_monitor: true", " \
	This enables monitoring of what's happening in the indexer. \
	It can help to detect performance/configuration problems. \
" }, \
{ "wordlist_monitor_period","0", \
	"number", "all", "", "3.2.0b1", "Extra Output", "wordlist_monitor_period: .1", " \
	Sets the number of seconds between each monitor output. \
" }, \
{ "wordlist_monitor_output","", \
	"string", "all", "", "3.2.0b1", "Extra Output", "wordlist_monitor_output: myfile", " \
	Print monitoring output on file instead of the default stderr. \
" }, 
{ "wordlist_page_size", "0",  \
	"integer", "all", "", "3.2.0b1", "Indexing:How", "wordlist_page_size: 8192", " \
	Size (in bytes) of pages used by Berkeley DB (DB used by the indexer). \
	Must be a power of two. \
" }, \
{ "wordlist_verbose", "",  \
	"integer", "", "", "", "", "wordlist_verbose: true", " \
	wordlist_verbose 1 walk logic<br>    \
	wordlist_verbose 2 walk logic details<br>    \
	wordlist_verbose 2 walk logic lots of details<br>    \
" }, \
{ "wordlist_wordkey_description", "Word/DocID 32/Flags 8/Location 16", \
	"string", "all", "", "3.2.0b1", "Indexing:How", "**this should not be configured by user**", " \
	Internal key description: *not user configurable* \
" }, \
{ "wordlist_wordrecord_description", "DATA", \
	"string", "all", "", "3.2.0b1", "Indexing:How", "**this should not be configured by user**", " \
	Internal data description: *not user configurable* \
" }, \
{0, 0, 0, 0, 0, 0, 0, 0, 0}
};

HtConfiguration	config;
