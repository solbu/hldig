//
// defaults.cc
//
// default values for the ht programs
//
//
//
#if RELEASE
static char RCSid[] = "$Id: defaults.cc,v 1.64.2.4 1999/10/27 15:31:53 grdetil Exp $";
#endif

#include "Configuration.h"

ConfigDefaults	defaults[] =
{

{ "add_anchors_to_excerpt", "true", 
	"boolean", "htsearch", "add_anchors_to_excerpt: no", "
	If set to true, the first occurrence of each matched
	word in the excerpt will be linked to the closest
	anchor in the document. This only has effect if the
	<strong>EXCERPT</strong> variable is used in the output
	template and the excerpt is actually going to be displayed.
" },
{ "allow_in_form", "", 
	"string list", "htsearch", "allow_in_form: search_algorithm search_results_header", "
	Allows the specified config file attributes to be specified
	in search forms as separate fields. This could be used to
	allow form writers to design their own headers and footers
	and specify them in the search form. Another example would
	be to offer a menu of search_algorithms in the form.
	<table>
	<tr>
	<td nowrap>
	<tt>
	&nbsp;&nbsp;&lt;SELECT NAME="search_algorithm"&gt;<br>
	&nbsp;&nbsp;&lt;OPTION VALUE="exact:1 prefix:0.6 synonyms:0.5 endings:0.1" SELECTED&gt;fuzzy<br>
	&nbsp;&nbsp;&lt;OPTION VALUE="exact:1"&gt;exact<br>
	&nbsp;&nbsp;&lt;/SELECT&gt;
	</tt></td>
	</tr>
	</table>
" },
{ "allow_numbers", "false", 
	"boolean", "htdig", "allow_numbers: true", "
	If set to true, numbers are considered words. This
	means that searches can be done on number as well as
	regular words. All the same rules apply to numbers as
	to words. See the description of
	<a href=\"#valid_punctuation\">valid_punctuation</a> for the
	rules used to determine what a word is.
" },
{ "allow_virtual_hosts", "true", 
	"boolean", "htdig", "allow_virtual_hosts: false", "
	If set to true, htdig will index virtual web sites as
	expected. If false, all URL host names will be
	normalized into whatever the DNS server claims the IP
	address to map to. If this option is set to false,
	there is no way to index either \"soft\" or \"hard\"
	virtual web sites.
" },
{ "backlink_factor", "1000", 
	"number", "htsearch", "backlink_factor: 501.1", "
	This is a weight of \"how important\" a page is, based on
	the number of URLs pointing to it. It's actually
	multiplied by the ratio of the incoming URLs (backlinks)
	and outgoing URLs, to balance out pages with lots of
	links to pages that link back to them. This factor can
	be changed without changing the database in any way.
	However, setting this value to something other than 0
	incurs a slowdown on search results.
" },
{ "bad_extensions", ".wav .gz .z .sit .au .zip .tar .hqx .exe .com .gif .jpg .jpeg .aiff .class .map .ram .tgz .bin .rpm .mpg .mov .avi", 
	"string list", "htdig", "bad_extensions: .foo .bar .bad", "
	This is a list of extensions on URLs which are
	considered non-parsable. This list is used mainly to
	supplement the MIME-types that the HTTP server provides
	with documents. Some HTTP servers do not have a correct
	list of MIME-types and so can advertise certain
	documents as text while they are some binary format.
" },
{ "bad_querystr", "", 
	"string list", "htdig", "bad_querystr: forum=private section=topsecret&amp;passwd=required", "
	This is a list of CGI query strings to be excluded from
	indexing. This can be used in conjunction with CGI-generated
	portions of a website to control which pages are
	indexed.
" },
{ "bad_word_list", "${common_dir}/bad_words", 
	"string", "htdig htsearch", "bad_word_list: ${common_dir}/badwords.txt", "
	This specifies a file which contains words which should
	be excluded when digging or searching. This list should
	include the most common words or other words that you
	don't want to be able to search on (things like <i>
	sex</i> or <i>smut</i> are examples of these.)<br>
	The file should contain one word per line. A sample
	bad words file is located in the <tt>contrib/examples</tt>
	directory.
" },
{ "bin_dir", BIN_DIR, 
	"string", "htdig htnotify htfuzzy htmerge htsearch", "bin_dir: /usr/local/bin", "
	This is the directory in which the executables
	related to ht://Dig are installed. It is never used
	directly by any of the programs, but other attributes
	can be defined in terms of this one.
	<p>
	The default value of this attribute is determined at
	compile time.
	</p>
" },
{ "case_sensitive", "true", 
	"boolean", "htdig", "case_sensitive: false", "
	This specifies whether ht://Dig should consider URLs
	case-sensitive or not. If your server is case-insensitive,
	you should probably set this to false.
" },
{ "common_dir", COMMON_DIR, 
	"string", "htdig htnotify htfuzzy htmerge htsearch", "common_dir: /tmp", "
	Specifies the directory for files that will or can be
	shared among different search databases. The default
	value for this attribute is defined at compile time.
" },
{ "common_url_parts", "http:// http://www. ftp:// ftp://ftp. /pub/ .html .htm .gif .jpg .jpeg /index.html /index.htm .com/ .com mailto:", 
	"string list", "htdig htnotify htmerge htsearch", "common_url_parts: http://www.htdig.org/ml/ \\<br>
.html \\<br>
http://dev.htdig.org/ \\<br>
http://www.htdig.org/", "
	Sub-strings often found in URLs stored in the
	database.  These are replaced in the database by an
	internal space-saving encoding.  If a string
	specified in <a href=\"#url_part_aliases\">url_part_aliases</a>,
	overlaps any string in common_url_parts, the
	common_url_parts string is ignored.<br>
	Note that when this attribute is changed, the
	database should be rebuilt, unless the effect of
	\"changing\" the affected URLs in the database is
	wanted.<br>
" },
{ "compression_level", "0", 
	"number", "htdig", "compression_level: 6", "
	If specified and the <a
	href=\"http://www.cdrom.com/pub/infozip/zlib/\">zlib</a>
	compression library was available when compiled,
	this attribute controls
	the amount of compression used in the <a
	href=\"#doc_db\">doc_db</a> file. Defaults to zero to
	provide backward compatility with old databases.
" },

{ "config_dir", CONFIG_DIR, 
	"string", "htdig htnotify htfuzzy htmerge htsearch", "config_dir: /var/htdig/conf", "
	This is the directory which contains all configuration
	files related to ht://Dig. It is never used
	directly by any of the programs, but other attributes
	or the <a href=\"#include\">include</a> directive
	can be defined in terms of this one.
	<p>
	The default value of this attribute is determined at
	compile time.
	</p>
" },
{ "create_image_list", "false", 
	"boolean", "htdig", "create_image_list: yes", "
	If set to true, a file with all the image URLs that
	were seen will be created, one URL per line. This list
	will not be in any order and there will be lots of
	duplicates, so after htdig has completed, it should be
	piped through <tt>sort -u</tt> to get a unique list.
" },
{ "create_url_list", "false", 
	"boolean", "htdig", "create_url_list: yes", "
	If set to true, a file with all the URLs that were seen
	will be created, one URL per line. This list will not
	be in any order and there will be lots of duplicates,
	so after htdig has completed, it should be piped
	through <tt>sort -u</tt> to get a unique list.
" },
{ "database_base", "${database_dir}/db", 
	"string", "htdig htnotify htfuzzy htmerge htsearch", "database_base: ${database_dir}/sales", "
	This is the common prefix for files that are specific
	to a search database. Many different attributes use
	this prefix to specify filenames. Several search
	databases can share the same directory by just changing
	this value for each of the databases.
" },
{ "database_dir", DATABASE_DIR, 
	"string", "htdig htnotify htfuzzy htmerge htsearch", "database_dir: /var/htdig", "
	This is the directory which contains all database and
	other files related to ht://Dig. It is never used
	directly by any of the programs, but other attributes
	are defined in terms of this one.
	<p>
	The default value of this attribute is determined at
	compile time.
	</p>
" },
{ "date_factor", "0", 
	"number", "htsearch", "date_factor: 0.35", "
	This factor, like <a
	href=\"#backlink_factor\">backlink_factor</a> can be
	changed without modifing the database. It gives higher
	rankings to newer documents and lower rankings to older
	documents. Before setting this factor, it's advised to
	make sure your servers are returning accurate dates
	(check the dates returned in the long format).
	Additionally, setting this to a nonzero value incurs a
	performance hit on searching.
" },
{ "date_format", "", 
	"string", "htsearch", "date_format: %Y-%m-%d", "
	This format string determines the output format for
	modification dates of documents in the search results.
	It is interpreted by your system's <em>strftime</em>
	function. Please refer to your system's manual page
	for this function, for a description of available
	format codes. If this format string is empty, as it
	is by default, 
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>
	will pick a format itself. In this case, the <a
	href=\"#iso_8601\">iso_8601</a> attribute can be used
	to modify the appearance of the date.
" },
{ "description_factor", "150", 
	"number", "htdig", "description_factor: 350", "
	Plain old \"descriptions\" are the text of a link pointing
	to a document. This factor gives weight to the words of
	these descriptions of the document. Not surprisingly,
	these can be pretty accurate summaries of a document's
	content. See also <a href=\"#title_factor\">title_factor</a> or <a
	href=\"#text_factor\">text_factor</a>. Changing this
	factor will require updating your database.
" },
{ "doc_db", "${database_base}.docdb", 
	"string", "htdig htmerge htsearch", "doc_db: ${database_base}documents.db", "
	This file will contain a Berkeley database of documents
	indexed by document number. It contains all the information
	gathered for each document, so this file can become
	rather large if <a href=\"#max_head_length\"><em>
	max_head_length</em></a> is set to a large value.
" },
{ "doc_excerpt", "${database_base}.excerpts", 
	"", "", "", "

" },
{ "doc_index", "${database_base}.docs.index", 
	"string", "htmerge htdig", "doc_index: documents.index.db", "
	This file will contain a Berkeley database which maps
	document URLs to document numbers. It is basically an
	intermediate database from URLs to the document
	database.
" },
{ "doc_list", "${database_base}.docs", 
	"string", "htdig", "doc_list: /tmp/documents.text", "
	This file is basically a text version of the file
	specified in <em><a href=\"#doc_db\">doc_db</a></em>. Its
	only use is to have a human readable database of all
	documents. The file is easy to parse with tools like
	perl or tcl.
" },
{ "end_ellipses", "<b><tt> ...</tt></b>", 
	"string", "htsearch", "end_ellipses: ...", "
	When excerpts are displayed in the search output, this
	string will be appended to the excerpt if there is text
	following the text displayed. This is just a visual
	reminder to the user that the excerpt is only part of
	the complete document.
" },
{ "endings_affix_file", "${common_dir}/english.aff", 
	"string", "htfuzzy", "endings_affix_file: /var/htdig/affix_rules", "
	Specifies the location of the file which contains the
	affix rules used to create the endings search algorithm
	databases. Consult the documentation on
	<a href=\"htfuzzy.html\">htfuzzy</a> for more information on the
	format of this file.
" },
{ "endings_dictionary", "${common_dir}/english.0", 
	"string", "htfuzzy", "endings_dictionary: /var/htdig/dictionary", "
	Specifies the location of the file which contains the
	dictionary used to create the endings search algorithm
	databases. Consult the documentation on
	<a href=\"htfuzzy.html\">htfuzzy</a> for more information on the
	format of this file.
" },
{ "endings_root2word_db", "${common_dir}/root2word.db", 
	"string", "htfuzzy htsearch", "endings_root2word_db: /var/htdig/r2w.db", "
	This attributes specifies the database filename to be
	used in the 'endings' fuzzy search algorithm. The
	database maps word roots to all legal words with that
	root. For more information about this and other fuzzy
	search algorithms, consult the
	<a href=\"htfuzzy.html\">htfuzzy</a> documentation.<br>
	Note that the default value uses the
	<a href=\"#common_dir\">common_dir</a> attribute instead of the
	<a href=\"#database_dir\">database_dir</a> attribute.
	This is because this database can be shared with
	different search databases.
" },
{ "endings_word2root_db", "${common_dir}/word2root.db", 
	"string", "htfuzzy htsearch", "endings_word2root_db: /var/htdig/w2r.bm", "
	This attributes specifies the database filename to be
	used in the 'endings' fuzzy search algorithm. The
	database maps words to their root. For more information
	about this and other fuzzy search algorithms, consult
	the <a href=\"htfuzzy.html\">htfuzzy</a>
	documentation.<br>
	Note that the default value uses the
	<a href=\"#common_dir\">common_dir</a> attribute instead of the
	<a href=\"#database_dir\">database_dir</a> attribute.
	This is because this database can be shared with
	different search databases.
" },
{ "excerpt_length", "300", 
	"number", "htsearch", "excerpt_length: 500", "
	This is the maximum number of characters the displayed
	excerpt will be limited to. The first matched word will
	be bolded in the middle of the excerpt so that there is
	some surrounding context.<br>
	The <em><a href=\"#start_ellipses\">
	start_ellipses</a></em> and
	<em><a href=\"#end_ellipses\">end_ellipses</a></em> are used to
	indicate that the document contains text before and
	after the displayed excerpt respectively.
" },
{ "excerpt_show_top", "false", 
	"boolean", "htsearch", "excerpt_show_top: yes", "
	If set to true, the excerpt of a match will always show
	the top of the matching document. If it is false (the
	default), the excerpt will attempt to show the part of
	the document that actually contains one of the words.
" },
{ "exclude_urls", "/cgi-bin/ .cgi", 
	"string list", "htdig", "exclude_urls: students.html cgi-bin", "
	If a URL contains any of the space separated patterns,
	it will be rejected. This is used to exclude such
	common things such as an infinite virtual web-tree
	which start with cgi-bin.
" },
{ "external_parsers", "", 
	"quoted string list", "htdig", "external_parsers: text/html /usr/local/bin/htmlparser \\<br>
application/ms-word \"/usr/local/bin/mswordparser -w\"", "
			This attribute is used to specify a list of
			content-type/parsers that are to be used to parse
			documents that cannot by parsed by any of the internal
			parsers. The list of external parsers is examined
			before the builtin parsers are checked, so this can be
			used to override the internal behavior without
			recompiling htdig.<br>
			 The external parsers are specified as pairs of
			strings. The first string of each pair is the
			content-type that the parser can handle while the
			second string of each pair is the path to the external
			parsing program. If quoted, it may contain parameters,
			separated by spaces.<p>
			 The parser program takes four command-line
			parameters, not counting parameters and parameters
			given in the command string:<br>
			<em>infile content-type URL configuration-file</em><br>
			<table border=\"1\">
			  <tr>
				<th>
				  Parameter
				</th>
				<th>
				  Description
				</th>
				<th>
				  Example
				</th>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  infile
				</td>
				<td>
				  A temporary file with the contents to be parsed.
				</td>
				<td>
				  /var/tmp/htdext.14242
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  content-type
				</td>
				<td>
				  The MIME-type of the contents.
				</td>
				<td>
				  text/html
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  URL
				</td>
				<td>
				  The URL of the contents.
				</td>
				<td>
				  http://www.htdig.org/attrs.html
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  configuration-file
				</td>
				<td>
				  The configuration-file in effect.
				</td>
				<td>
				  /etc/htdig/htdig.conf
				</td>
			  </tr>
			</table><p>
			The external parser is to write information for
			htdig on its standard output.<br>
			 The output consists of records, each record terminated
			with a newline. Each record is a series of (unless
			expressively allowed to be empty) non-empty tab-separated
			fields. The first field is a single character
			that specifies the record type. The rest of the fields
			are determined by the record type.
			<table border=\"1\">
			  <tr>
				<th>
				  Record type
				</th>
				<th>
				  Fields
				</th>
				<th>
				  Description
				</th>
			  </tr>
			  <tr>
				<th rowspan=\"3\" valign=\"top\">
				  w
				</th>
				<td valign=\"top\">
				  word
				</td>
				<td>
				  A word that was found in the document.
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  location
				</td>
				<td>
				  A number indicating the normalized location of
				  the word within the document. The number has to
				  fall in the range 0-1000 where 0 means the top of
				  the document.
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  heading level
				</td>
				<td>
				  A heading level that is used to compute the
				  weight of the word depending on its context in
				  the document itself. The level is in the range of
				  0-10 and are defined as follows:
				  <dl compact>
					<dt>
					  0
					</dt>
					<dd>
					  Normal text
					</dd>
					<dt>
					  1
					</dt>
					<dd>
					  Title text
					</dd>
					<dt>
					  2
					</dt>
					<dd>
					  Heading 1 text
					</dd>
					<dt>
					  3
					</dt>
					<dd>
					  Heading 2 text
					</dd>
					<dt>
					  4
					</dt>
					<dd>
					  Heading 3 text
					</dd>
					<dt>
					  5
					</dt>
					<dd>
					  Heading 4 text
					</dd>
					<dt>
					  6
					</dt>
					<dd>
					  Heading 5 text
					</dd>
					<dt>
					  7
					</dt>
					<dd>
					  Heading 6 text
					</dd>
					<dt>
					  8
					</dt>
					<dd>
					  <i>unused</i>
					</dd>
					<dt>
					  9
					</dt>
					<dd>
					  <i>unused</i>
					</dd>
					<dt>
					  10
					</dt>
					<dd>
					  Keywords
					</dd>
				  </dl>
				</td>
			  </tr>
			  <tr>
				<th rowspan=\"2\" valign=\"top\">
				  u
				</th>
				<td valign=\"top\">
				  document URL
				</td>
				<td>
				  A hyperlink to another document that is
				  referenced by the current document.  It must be
				  complete and non-relative, using the URL parameter to
				  resolve any relative references found in the document.
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  hyperlink description
				</td>
				<td>
				  For HTML documents, this would be the text
				  between the &lt;a href...&gt; and &lt;/a&gt;
				  tags.
				</td>
			  </tr>
			  <tr>
				<th valign=\"top\">
				  t
				</th>
				<td valign=\"top\">
				  title
				</td>
				<td>
				  The title of the document
				</td>
			  </tr>
			  <tr>
				<th valign=\"top\">
				  h
				</th>
				<td valign=\"top\">
				  head
				</td>
				<td>
				  The top of the document itself. This is used to
				  build the excerpt. This should only contain
				  normal ASCII text
				</td>
			  </tr>
			  <tr>
				<th valign=\"top\">
				  a
				</th>
				<td valign=\"top\">
				  anchor
				</td>
				<td>
				  The label that identifies an anchor that can be
				  used as a target in an URL. This really only
				  makes sense for HTML documents.
				</td>
			  </tr>
			  <tr>
				<th valign=\"top\">
				  i
				</th>
				<td valign=\"top\">
				  image URL
				</td>
				<td>
				  An URL that points at an image that is part of
				  the document.
				</td>
			  </tr>
			  <tr>
				<th rowspan=\"3\" valign=\"top\">
				  m
				</th>
				<td valign=\"top\">
				  http-equiv
				</td>
				<td>
				  The HTTP-EQUIV attribute of a
				  <a href=\"meta.html\"><i>META</i> tag</a>.
				  May be empty.
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  name
				</td>
				<td>
				  The NAME attribute of this
				  <a href=\"meta.html\"><i>META</i> tag</a>.
				  May be empty.
				</td>
			  </tr>
			  <tr>
				<td valign=\"top\">
				  contents
				</td>
				<td>
				  The CONTENTS attribute of this
				  <a href=\"meta.html\"><i>META</i> tag</a>.
				  May be empty.
				</td>
			  </tr>
			</table>
	<p><em>See also FAQ questions <a
	href=\"FAQ.html#q4.8\">4.8</a> and <a
	href=\"FAQ.html#q4.9\">4.9</a> for more
	examples.</em></p>
" },
{ "extra_word_characters", "", 
	"string", "htdig htsearch", "extra_word_characters: _", "
	These characters are considered part of a word.
	In contrast to the characters in the
	<a href=\"#valid_punctuation\">valid_punctuation</a>
	attribute, they are treated just like letter
	characters.<br>
	Note that the <a href=\"#locale\">locale</a> attribute
	is normally used to configure which characters
	constitute letter characters.
" },

{ "head_before_get", "false", 
	"boolean", "htdig", "head_before_get: true", "
        This option works only if we take advantage of persistent connections (see
        persistent_connections attribute). If set to true an HTTP/1.1 <I>HEAD</I>
        call is made in order to retrieve header information about a document.
        If the status code and the content-type returned let the document be parsable,
        then a following 'GET' call is made.
" },

{ "heading_factor", "5", 
	"", "", "", "

" },

{ "htnotify_sender", "webmaster@www", 
	"string", "htnotify", "htnotify_sender: bigboss@yourcompany.com", "
	This specifies the email address that htnotify email
	messages get sent out from. The address is forged using
	/usr/lib/sendmail. Check htnotify/htnotify.cc for
	detail on how this is done.
" },
{ "http_proxy", "", 
	"string", "htdig", "http_proxy: http://proxy.bigbucks.com:3128", "
	When this attribute is set, all HTTP document
	retrievals will be done using the HTTP-PROXY protocol.
	The URL specified in this attribute points to the host
	and port where the proxy server resides.<br>
	The use of a proxy server greatly improves performance
	of the indexing process.
" },
{ "http_proxy_exclude", "", 
	"string list", "htdig", "http_proxy_exclude: http://intranet.foo.com/", "
	When this is set, URLs matching this will not use the
	proxy. This is useful when you have a mixture of sites
	near to the digging server and far away.
" },
{ "image_list", "${database_base}.images", 
	"string", "htdig", "image_list: allimages", "
	This is the file that a list of image URLs gets written
	to by <a href=\"htdig.html\">htdig</a> when the
	<a href=\"#create_image_list\">create_image_list</a> is set to
	true. As image URLs are seen, they are just appended to
	this file, so after htdig finishes it is probably a
	good idea to run <tt>sort -u</tt> on the file to
	eliminate duplicates from the file.
" },
{ "image_url_prefix", IMAGE_URL_PREFIX, 
	"string", "htsearch", "image_url_prefix: /images/htdig", "
	This specifies the directory portion of the URL used
	to display star images. This attribute isn't directly
	used by htsearch, but is used in the default URL for
	the <a href=\"#star_image\">star_image</a> and
	<a href=\"#star_blank\">star_blank</a> attributes, and
	other attributes may be defined in terms of this one.
	<p>
	The default value of this attribute is determined at
	compile time.
	</p>
" },
{ "img_alt_factor", "3", 
	"", "", "", "

" },
{ "include", "",
        "string", "htdig htnotify htfuzzy htmerge htsearch", "include: ${config_dir}/htdig.conf", "
			This is not quite a configuration attribute, but
			rather a directive. It can be used within one
			configuration file to include the definitions of
			another file. The last definition of an attribute
			is the one that applies, so after including a file,
			any of its definitions can be overridden with
			subsequent definitions. This can be useful when
			setting up many configurations that are mostly the
			same, so all the common attributes can be maintained
			in a single configuration file. The include directives
			can be nested, but watch out for nesting loops.
" },
{ "iso_8601", "false", 
	"boolean", "htsearch htnotify", "iso_8601: true", "
	This sets whether dates should be output in ISO 8601
	format. For example, this was written on: 1998-10-31 11:28:13 EST.
	See also the <a
	href=\"#date_format\">date_format</a> attribute, which
	can override any date format that
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>
	picks by default.<br>
	This attribute also affects the format of the date
	<a href=\"htnotify.html\">htnotify</a> expects to find
	in a <strong>htdig-notification-date</strong> field.
" },
{ "keywords_factor", "100", 
	"number", "htdig", "keywords_factor: 12", "
	This is a factor which will be used to multiply the
	weight of words in the list of keywords of a document.
	The number may be a floating point number. See also the
	<a href=\"#title_factor\">title_factor</a> and
	<a href=\"#text_factor\">text_factor</a>attributes.
" },
{ "keywords_meta_tag_names", "keywords htdig-keywords", 
	"string list", "htdig", "keywords_meta_tag_names: keywords description", "
	The words in this list are used to search for keywords
	in HTML <i>META</i> tags. This list can contain any
	number of strings that each will be seen as the name
	for whatever keyword convention is used.<br>
	The <i>META</i> tags have the following format:<br>
<tt>
&nbsp;&nbsp;&lt;META name=\"<i>somename</i>\" content=\"<i>somevalue</i>\"&gt;
</tt>
" },
{ "limit_normalized", "", 
	"string list", "htdig", "limit_normalized: http://www.mydomain.com", "
	This specifies a set of patterns that all URLs have to
	match against in order for them to be included in the
	search. Unlike the limit_urls_to directive, this is done
	<b>after</b> the URL is normalized and the server_aliases
	directive is applied. This allows filtering after any
	hostnames and DNS aliases are resolved. Otherwise, this
	directive is the same as the <a
	href=\"#limit_urls_to\">limit_urls_to</a> directive.
" },
{ "limit_urls_to", "${start_url}", 
	"string list", "htdig", "limit_urls_to: .sdsu.edu kpbs", "
	This specifies a set of patterns that all URLs have to
	match against in order for them to be included in the
	search. Any number of strings can be specified,
	separated by spaces. If multiple patterns are given, at
	least one of the patterns has to match the URL.<br>
	Matching is a case-insensitive string match on the URL
	to be used. The match will be performed <em>after</em>
	the relative references have been converted to a valid
	URL. This means that the URL will <em>always</em> start
	with <tt>http://</tt>.<br>
	Granted, this is not the perfect way of doing this,
	but it is simple enough and it covers most cases.
" },
{ "local_default_doc", "index.html", 
	"string", "htdig", "local_default_doc: default.html", "
	Set this to the default document in a directory used by the
	server. This is used for local filesystem access to
	translate URLs like http://foo.com/ into something like
	/home/foo.com/index.html
" },
{ "local_urls", "", 
	"string list", "htdig", "local_urls: http://www.foo.com/=/usr/www/htdocs/", "
	Set this to tell ht://Dig to access certain URLs through
	local filesystems. At first ht://Dig will try to access
	pages with URLs matching the patterns through the
	filesystems specified. If it cannot find the file, it will
	try the URL through HTTP instead. Note the example--the
	equal sign and the final slashes in both the URL and the
	directory path are critical.
" },
{ "local_user_urls", "", 
	"string list", "htdig", "local_user_urls: http://www.my.org/=/home/,/www/", "
	Set this to access user directory URLs through the local
	filesystem. If you leave the \"path\" portion out, it will
	look up the user's home directory in /etc/password (or NIS
	or whatever). As with local_urls, if the files are not
	found, ht://Dig will try with HTTP. Again, note the
	example's format. To map http://www.my.org/~joe/foo/bar.html
	to /home/joe/www/foo/bar.html, try the example below.
" },
{ "locale", "C", 
	"string", "htdig", "locale: en_US", "
	Set this to whatever locale you want your search
	database cover. It affects the way international
	characters are dealt with. On most systems a list of
	legal locales can be found in /usr/lib/locale. Also
	check the <strong>setlocale(3C)</strong> man page.
" },
{ "logging", "false", 
	"boolean", "htsearch", "logging: true", "
			This sets whether htsearch should use the syslog() to log
			search requests. If set, this will log requests with a
			default level of LOG_INFO and a facility of LOG_LOCAL5. For
			details on redirecting the log into a separate file or other
			actions, see the <strong>syslog.conf(5)</strong> man
			page. To set the level and facility used in logging, change
			LOG_LEVEL and LOG_FACILITY in the include/htconfig.h file
			before compiling.
			<dl>
			  <dt>
			    Each line logged by htsearch contains the following:
			  </dt>
			  <dd>
			    REMOTE_ADDR [config] (match_method) [words]
			    [logicalWords] (matches/matches_per_page) -
			    page, HTTP_REFERER
			  </dd>
			</dl>
			where any of the above are null or empty, it
			either puts in '-' or 'default' (for config).
" },
{ "maintainer", "bogus@unconfigured.htdig.user", 
	"string", "htdig", "maintainer: ben.dover@uptight.com", "
	This should be the email address of the person in
	charge of the digging operation. This string is added
	to the user-agent: field when the digger sends a
	request to a server.
" },
{ "match_method", "and", 
	"string", "htsearch", "match_method: boolean", "
	This is the default method for matching that htsearch
	uses. The valid choices are:
	<ul>
	<li>
	or
	</li>
	<li>
	and
	</li>
	<li>
	boolean
	</li>
	</ul>
	This attribute will only be used if the HTML form that
	calls htsearch didn't have the <strong>method</strong>
	value set.
" },
{ "matches_per_page", "10", 
	"number", "htsearch", "matches_per_page: 999", "
	If this is set to a relatively small number, the
	matches will be shown in pages instead of all at once.
" },
{ "max_description_length", "60", 
	"number", "htdig", "max_description_length: 40", "
	While gathering descriptions of URLs,
	<a href=\"htdig.html\">htdig</a> will only record those
	descriptions which are shorter than this length. This
	is used mostly to deal with broken HTML. (If a
	hyperlink is not terminated with a &lt;/a&gt; the
	description will go on until the end of the document.)
" },
{ "max_descriptions", "5", 
	"", "", "", "

" },
{ "max_doc_size", "100000", 
	"number", "htdig", "max_doc_size: 5000000", "
	This is the upper limit to the amount of data retrieved
	for documents. This is mainly used to prevent
	unreasonable memory consumption since each document
	will be read into memory by <a href=\"htdig.html\">
	htdig</a>.
" },
{ "max_head_length", "512", 
	"number", "htdig", "max_head_length: 50000", "
	For each document retrieved, the top of the document is
	stored. This attribute determines the size of this
	block. The text that will be stored is only the text;
	no markup is stored.<br>
	We found that storing 50,000 bytes will store about
	95% of all the documents completely. This really
	depends on how much storage is available and how much
	you want to show.
" },
{ "max_hop_count", "999999", 
	"number", "htdig", "max_hop_count: 4", "
	Instead of limiting the indexing process by URL
	pattern, it can also be limited by the number of hops
	or clicks a document is removed from the starting URL.
	Unfortunately, this only works reliably when a complete
	index is created, not an update.<br>
	The starting page will have hop count 0.
" },

{ "max_meta_description_length", "512", 
	"number", "htdig", "max_meta_description_length: 1000", "
	While gathering descriptions from meta description tags,
	<a href=\"htdig.html\">htdig</a> will truncate
	descriptions which are longer than this length.
" },
{ "max_prefix_matches", "1000", 
	"integer", "htsearch", "max_prefix_matches: 100", "
	The Prefix fuzzy algorithm could potentially match a
	very large number of words. This value limits the
	number of words each prefix can match. Note
	that this does not limit the number of documents that
	are matched in any way.
" },

{ "max_retries", "3", 
	"number", "htdig", "max_retries: 6", "
         This option set the maximum number of retries when retrieving a document
         fails (mainly for reasons of connection).
" },

{ "max_stars", "4", 
	"number", "htsearch", "max_stars: 6", "
	When stars are used to display the score of a match,
	this value determines the maximum number of stars that
	can be displayed.
" },
{ "maximum_pages", "10", 
	"integer", "htsearch", "maximum_pages: 20", "
	This value limits the number of page links that will be
	included in the page list at the bottom of the search
	results page. Note that this does not limit the number
	of documents that are matched in any way.
" },
{ "maximum_word_length", "32", 
	"number", "htdig htsearch", "maximum_word_length: 15", "
	This sets the maximum length of words that will be
	indexed. Words longer than this value will be silently
	truncated when put into the index, or searched in the
	index.
" },
{ "meta_description_factor", "50", 
	"number", "htdig", "meta_description_factor: 20", "
	This is a factor which will be used to multiply the
	weight of words in any META description tags in a document.
	The number may be a floating point number. See also the
	<a href=\"#title_factor\">title_factor</a> and
	<a href=\"#text_factor\">text_factor</a> attributes.
" },
{ "metaphone_db", "${database_base}.metaphone.db", 
	"string", "htfuzzy htsearch", "metaphone_db: ${database_base}.mp.db", "
	The database file used for the fuzzy \"metaphone\" search
	algorithm. This database is created by
	<a href=\"htfuzzy.html\">htfuzzy</a> and used by
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>.
" },
{ "method_names", "and All or Any boolean Boolean", 
	"quoted string list", "htsearch", "method_names: or Or and And", "
	These values are used to create the <strong>
	method</strong> menu. It consists of pairs. The first
	element of each pair is one of the known methods, the
	second element is the text that will be shown in the
	menu for that method. This text needs to be quoted if
	it contains spaces.
" },
{ "minimum_prefix_length", "1", 
	"number", "htsearch", "minimum_prefix_length: 2", "
	This sets the minimum length of prefix matches used by the
	\"prefix\" fuzzy matching algorithm. Words shorter than this
	will not be used in prefix matching.
" },
{ "minimum_speling_length", "5", 
	"", "", "", "

" },
{ "minimum_word_length", "3", 
	"number", "htdig htsearch", "minimum_word_length: 2", "
	This sets the minimum length of words that will be
	indexed. Words shorter than this value will be silently
	ignored but still put into the excerpt.<br>
	Note that by making this value less than 3, a lot more
	words that are very frequent will be indexed. It might
	be advisable to add some of these to the bad_words
	list.
" },
{ "modification_time_is_now", "true", 
	"boolean", "htdig", "modification_time_is_now: true", "
	This sets ht://Dig's response to a server that does not
	return a modification date. By default, it stores
	nothing. By setting modification_time_is_now, it will store
	the current time if the server does not return a
	date. Though this will return incorrect dates in search
	results, it will cut down on reindexing from such servers
	when doing updates.
" },
{ "next_page_text", "[next]", 
	"string", "htsearch", "next_page_text: &lt;img src=\"/htdig/buttonr.gif\"&gt;", "
	The text displayed in the hyperlink to go to the next
	page of matches.
" },
{ "no_excerpt_show_top", "false", 
	"boolean", "htsearch", "no_excerpt_show_top: yes", "
	If no excerpt is available, this option will act the
	same as <a
	href=\"#excerpt_show_top\">excerpt_show_top</a>, that is,
	it will show the top of the document.
" },
{ "no_excerpt_text", "<em>(None of the search words were found in the top of this document.)</em>", 
	"string", "htsearch", "no_excerpt_text:", "
	This text will be displayed in place of the excerpt if
	there is no excerpt available. If this attribute is set
	to nothing (blank), the excerpt label will not be
	displayed in this case.
" },
{ "no_next_page_text", "[next]", 
	"string", "htsearch", "no_next_page_text:", "
	The text displayed where there would normally be a
	hyperlink to go to the next page of matches.
" },
{ "no_page_list_header", "", 
	"string", "htsearch", "no_page_list_header: &lt;hr noshade size=2&gt;All results on this page.&lt;br&gt;", "
	This text will be used as the value of the PAGEHEADER
	variable, for use in templates or the
	<a href=\"#search_results_footer\">search_results_footer</a>
	file, when all search results fit on a single page.
" },
{ "no_page_number_text", "", 
	"quoted string list", "htsearch", "no_page_number_text:
				  &lt;strong&gt;1&lt;/strong&gt; &lt;strong&gt;2&lt;/strong&gt; \\<br>
				  &lt;strong&gt;3&lt;/strong&gt; &lt;strong&gt;4&lt;/strong&gt; \\<br>
				  &lt;strong&gt;5&lt;/strong&gt; &lt;strong&gt;6&lt;/strong&gt; \\<br>
				  &lt;strong&gt;7&lt;/strong&gt; &lt;strong&gt;8&lt;/strong&gt; \\<br>
				  &lt;strong&gt;9&lt;/strong&gt; &lt;strong&gt;10&lt;/strong&gt;
", "
	The text strings in this list will be used when putting
	together the PAGELIST variable, for use in templates or
	the <a href=\"#search_results_footer\">search_results_footer</a>
	file, when search results fit on more than page. The PAGELIST
	is the list of links at the bottom of the search results page.
	There should be as many strings in the list as there are
	pages allowed by the <a href=\"#maximum_pages\">maximum_pages</a>
	attribute. If there are not enough, or the list is empty,
	the page numbers alone will be used as the text for the links.
	An entry from this list is used for the current page, as the
	current page is shown in the page list without a hypertext link,
	while entries from the <a href=\"#page_number_text\">
	page_number_text</a> list are used for the links to other pages.
	The text strings can contain HTML tags to highlight page numbers
	or embed images. The strings need to be quoted if they contain
	spaces.
" },
{ "no_prev_page_text", "[prev]", 
	"string", "htsearch", "no_prev_page_text:", "
	The text displayed where there would normally be a
	hyperlink to go to the previous page of matches.
" },
{ "no_title_text", "filename", 
	"string", "htsearch", "no_title_text: \"No Title Found\"", "
	This specifies the text to use in search results when no
	title is found in the document itself. If it is set to
	filename, htsearch will use the name of the file itself,
	enclosed in brackets (e.g. [index.html]).
" },
{ "noindex_end", "<!--/htdig_noindex-->", 
	"string", "htdig", "noindex_end: &lt;/SCRIPT&gt;", "
	This string marks the end of a section of an HTML file that should be
	completely ignored when indexing. It works together with noindex_start
	described below. As in the defaults, this can be SGML comment 
	declarations that can be inserted anywhere in the documents to exclude 
	different sections from being indexed. However, existing tags can also be 
	used; this is especially useful to exclude some sections from being indexed 
	where the files to be indexed can not be edited. The example shows how
	SCRIPT sections in 'uneditable' documents can be skipped.
	Note that the match for this string is case insensitive.
" },
{ "noindex_start", "<!--htdig_noindex-->", 
	"string", "htdig", "noindex_start: &lt;SCRIPT", "
	This string marks the start of a section of an HTML file that should be
	completely ignored when indexing. It works together with noindex_end
	described above. As in the defaults, this can be SGML comment
	declarations that can be inserted anywhere in the documents to exclude
	different sections from being indexed. However, existing tags can also be
	used; this is especially useful to exclude some sections from being indexed
	where the files to be indexed can not be edited. The example shows how
	SCRIPT sections in 'uneditable' documents can be skipped; note how
	noindex_start does not contain an ending &gt;: this allows for all SCRIPT
	tags to be matched regardless of attributes defined (different types or
	languages). Note that the match for this string is case insensitive.
" },
{ "nothing_found_file", "${common_dir}/nomatch.html", 
	"string", "htsearch", "nothing_found_file: /www/searching/nothing.html", "
	This specifies the file which contains the <tt>
	HTML</tt> text to display when no matches were found.
	The file should contain a complete <tt>HTML</tt>
	document.<br>
	Note that this attribute could also be defined in
	terms of <a href=\"#database_base\">database_base</a> to
	make is specific to the current search database.
" },
{ "page_list_header", "<hr noshade size=2>Pages:<br>", 
	"string", "htsearch", "page_list_header:", "
	This text will be used as the value of the PAGEHEADER
	variable, for use in templates or the
	<a href=\"#search_results_footer\">search_results_footer</a>
	file, when all search results fit on more than one page.
" },
{ "page_number_text", "", 
	"quoted string list", "htsearch", "page_number_text:
				  &lt;em&gt;1&lt;/em&gt; &lt;em&gt;2&lt;/em&gt; \\<br>
				  &lt;em&gt;3&lt;/em&gt; &lt;em&gt;4&lt;/em&gt; \\<br>
				  &lt;em&gt;5&lt;/em&gt; &lt;em&gt;6&lt;/em&gt; \\<br>
				  &lt;em&gt;7&lt;/em&gt; &lt;em&gt;8&lt;/em&gt; \\<br>
				  &lt;em&gt;9&lt;/em&gt; &lt;em&gt;10&lt;/em&gt;
", "
	The text strings in this list will be used when putting
	together the PAGELIST variable, for use in templates or
	the <a href=\"#search_results_footer\">search_results_footer</a>
	file, when search results fit on more than page. The PAGELIST
	is the list of links at the bottom of the search results page.
	There should be as many strings in the list as there are
	pages allowed by the <a href=\"#maximum_pages\">maximum_pages</a>
	attribute. If there are not enough, or the list is empty,
	the page numbers alone will be used as the text for the links.
	Entries from this list are used for the links to other pages,
	while an entry from the <a href=\"#no_page_number_text\">
	no_page_number_text</a> list is used for the current page, as the
	current page is shown in the page list without a hypertext link.
	The text strings can contain HTML tags to highlight page numbers
	or embed images. The strings need to be quoted if they contain
	spaces.
" },
{ "pdf_parser", PDF_PARSER " -toPostScript -pairs", 
	"string", "htdig", "pdf_parser: /usr/local/Acrobat3/bin/acroread -toPostScript -pairs", "
	Set this to the path of the program used to parse PDF
	files, including all command-line options. The
	program will be called with the parameters:<br>
	<em>infile outfile</em>,<br>
	where <em>infile</em> is a file to parse and
	<em>outfile</em> is the PostScript output of the
	parser. In the case where acroread is the parser, and
	the -pairs option is not given, the second parameter
	will be the output directory rather than the output
	file name. The program is supposed to convert to a
	variant of PostScript, which is then parsed
	internally. Currently, only Adobe's <a
	href=\"http://www.adobe.com/prodindex/acrobat/readstep.html\">
	acroread</a> program has been tested as a pdf_parser.
	There is a bug in Acrobat 4's acroread command, which
	causes it to fail when -pairs is used, hence the special
	case above.<br>
	The pdftops program that is part of the <a
	href=\"http://www.foolabs.com/xpdf/\">xpdf</a>
	package is not suitable as a pdf_parser,
	because its variant of PostScript is slightly
	different.  However, an alternative is to
	use xpdf's pdftotext program as a component
	of an <a href=\"#external_parsers\">external
	parser</a> with the xpdf 0.90 package installed
	on your system, as described in FAQ question <a
	href=\"FAQ.html#q4.9\">4.9</a>.<br>
	In either case, to successfully index PDF files,
	be sure to set the <a
	href=\"#max_doc_size\">max_doc_size</a> attribute
	to a value larger than the size of your largest
	PDF file. PDF documents can not be parsed if they
	are truncated.
	<p>
	The default value of this attribute is determined at
	compile time, to include the path to the acroread
	executable.
	</p>
" },

{ "persistent_connections", "true", 
	"boolean", "htdig", "persistent_connections: false", "
	If set to true, when servers make it possible, htdig can take advantage
        of persistent connections, as defined by HTTP/1.1 (<I>RFC2616</I>). This permits
        to reduce the number of open/close operations of connections, when retrieving
        a document with HTTP.
" },

{ "prefix_match_character", "*", 
	"string", "htsearch", "prefix_match_character: ing", "
	A null prefix character means that prefix matching should be
	applied to every search word. Otherwise a match is
	returned only if the word does not end in the characters specified.
" },
{ "prev_page_text", "[prev]", 
	"string", "htsearch", "prev_page_text: &lt;img src=\"/htdig/buttonl.gif\"&gt;", "
	The text displayed in the hyperlink to go to the
	previous page of matches.
" },
{ "regex_max_words", "25", 
	"", "", "", "

" },
{ "remove_bad_urls", "true", 
	"boolean", "htmerge", "remove_bad_urls: true", "
	If TRUE, htmerge will remove any URLs which were marked
	as unreachable by htdig from the database. If FALSE, it
	will not do this. When htdig is run in initial mode,
	documents which were referred to but could not be
	accessed should probably be removed, and hence this
	option should then be set to TRUE, however, if htdig is
	run to update the database, this may cause documents on
	a server which is temporarily unavailable to be
	removed. This is probably NOT what was intended, so
	hence this option should be set to FALSE in that case.
" },
{ "remove_default_doc", "index.html", 
	"string list", "htdig", "remove_default_doc: default.html default.htm index.html index.htm", "
	Set this to the default documents in a directory used by the
	servers you are indexing. These document names will be stripped
	off of URLs when they are normalized, if one of these names appears
	after the final slash, to translate URLs like
	http://foo.com/index.html into http://foo.com/<br>
	Note that you can disable stripping of these names during
	normalization by setting the list to an empty string.
	The list should only contain names that all servers you index
	recognize as default documents for directory URLs, as defined
	by the DirectoryIndex setting in Apache's srm.conf, for example.
" },
{ "robotstxt_name", "htdig", 
	"string", "htdig", "robotstxt_name: myhtdig", "
	Sets the name that htdig will look for when parsing
	robots.txt files. This can be used to make htdig appear
	as a different spider than ht://Dig. Useful to
	distinguish between a private and a global index.
" },
{ "script_name", "", 
	"string", "htsearch", "script_name: /search/results.shtml", "
	Overrides the value of the SCRIPT_NAME
	environment attribute. This is useful if
	htsearch is not being called directly as a CGI
	program, but indirectly from within a dynamic
	.shtml page using SSI directives. Previously,
	you needed a wrapper script to do this, but
	this configuration attribute makes wrapper
	scripts obsolete for SSI and possibly for
	other server scripting languages, as
	well. (You still need a wrapper script when
	using PHP, though.)<br>
	Check out the <tt>contrib/scriptname</tt>
	directory for a small example. Note that this
	attribute also affects the value of the <a
	href=\"hts_templates.html#CGI\">CGI</a> variable
	used in htsearch templates.
" },
{ "search_algorithm", "exact:1", 
	"string list", "htsearch", "search_algorithm: exact:1 soundex:0.3", "
			Specifies the search algorithms and their weight to use
			when searching. Each entry in the list consists of the
			algorithm name, followed by a colon (:) followed by a
			weight multiplier. The multiplier is a floating point
			number between 0 and 1. Current algorithms supported
			are:
			<dl>
			  <dt>
				exact
			  </dt>
			  <dd>
				The default exact word matching algorithm. This
				will find only exactly matched words.
			  </dd>
			  <dt>
				soundex
			  </dt>
			  <dd>
				Uses a slightly modified soundex algorithm to match
				words. This requires that the soundex database be
				present. It is generated with the
				<a href=\"htfuzzy.html\">htfuzzy</a> program.
			  </dd>
			  <dt>
				metaphone
			  </dt>
			  <dd>
				Uses the metaphone algorithm for matching words.
				This algorithm is more specific to the english
				language than soundex. It is generated with the <a
				href=\"htfuzzy.html\">htfuzzy</a> program.
			  </dd>
			  <dt>
				endings
			  </dt>
			  <dd>
				This algorithm uses language specific word endings
				to find matches. Each word is first reduced to its
				word root and then all known legal endings are used
				for the matching. This algorithm uses two databases
				which are generated with <a href=\"htfuzzy.html\">
				htfuzzy</a>.
			  </dd>
			  <dt>
				synonyms
			  </dt>
			  <dd>
				Performs a dictionary lookup on all the words. This
				algorithm uses a database generated with the <a
				href=\"htfuzzy.html\">htfuzzy</a> program.
			  </dd>
			<dt>
			substring
			</dt>
			<dd>
			  Matches all words containing the queries as
			  substrings. Since this requires checking every word in
			  the database, this can really slow down searches
			  considerably.
			<dd>
			<dt>
			  prefix
			</dt>
			<dd>
			  Matches all words beginning with the query
			  strings. Uses the option <a
			  href=\"#prefix_match_character\">prefix_match_character</a>
			  to decide whether a query requires prefix
			  matching. For example \"abc*\" would perform prefix
			  matching on \"abc\" since * is the default
			  prefix_match_character.
			</dd>
			</dl>
" },
{ "search_results_footer", "${common_dir}/footer.html", 
	"string", "htsearch", "search_results_footer: /usr/local/etc/ht/end-stuff.html", "
			This specifies a filename to be output at the end of
			search results. While outputting the footer, some
			variables will be expanded. Variables use the same
			syntax as the Bourne shell. If there is a variable VAR,
			the following will all be recognized:
			<ul>
			  <li>
				$VAR
			  </li>
			  <li>
				$(VAR)
			  </li>
			  <li>
				${VAR}
			  </li>
			</ul>
			The following variables are available:
			<dl>
			  <dt>
				MATCHES
			  </dt>
			  <dd>
				The number of documents that were matched.
			  </dd>
			  <dt>
				PLURAL_MATCHES
			  </dt>
			  <dd>
				If MATCHES is not 1, this will be the string \"s\",
				else it is an empty string. This can be used to say
				something like \"$(MATCHES)
				document$(PLURAL_MATCHES) were found\"
			  </dd>
			  <dt>
				MAX_MATCHES
			  </dt>
			  <dd>
				The number of maximum displayed matches.
			  </dd>
			  <dt>
				MAX_STARS
			  </dt>
			  <dd>
				The value of the <a href=\"#max_stars\">max_stars</a>
				attribute.
			  </dd>
			  <dt>
				LOGICAL_WORDS
			  </dt>
			  <dd>
				A string of the search words with either \"and\" or
				\"or\" between the words, depending on the type of
				search.
			  </dd>
			  <dt>
				WORDS
			  </dt>
			  <dd>
				A string of the search words with spaces in
				between.
			  </dd>
			  <dt>
				PAGEHEADER
			  </dt>
			  <dd>
				This expands to either the value of the
				<a href=\"#page_list_header\">page_list_header</a> or
				<a href=\"#no_page_list_header\">no_page_list_header</a>
				attribute depending on how many pages there are.
			  </dd>
			</dl>
			Note that this file will <strong>NOT</strong> be output
			if no matches were found. In this case the
			<a href=\"#nothing_found_file\">nothing_found_file</a>
			attribute is used instead.
			Also, this file will not be output if it is
			overridden by defining the
			<a href=\"#search_results_wrapper\">search_results_wrapper</a>
			attribute.
" },
{ "search_results_header", "${common_dir}/header.html", 
	"string", "htsearch", "search_results_header: /usr/local/etc/ht/start-stuff.html", "
			This specifies a filename to be output at the start of
			search results. While outputting the header, some
			variables will be expanded. Variables use the same
			syntax as the Bourne shell. If there is a variable VAR,
			the following will all be recognized:
			<ul>
			  <li>
				$VAR
			  </li>
			  <li>
				$(VAR)
			  </li>
			  <li>
				${VAR}
			  </li>
			</ul>
			The following variables are available:
			<dl>
			  <dt>
				MATCHES
			  </dt>
			  <dd>
				The number of documents that were matched.
			  </dd>
			  <dt>
				PLURAL_MATCHES
			  </dt>
			  <dd>
				If MATCHES is not 1, this will be the string \"s\",
				else it is an empty string. This can be used to say
				something like \"$(MATCHES)
				document$(PLURAL_MATCHES) were found\"
			  </dd>
			  <dt>
				MAX_MATCHES
			  </dt>
			  <dd>
				The number of maximum displayed matches.
			  </dd>
			  <dt>
				MAX_STARS
			  </dt>
			  <dd>
				The value of the <a href=\"#max_stars\">max_stars</a>
				attribute.
			  </dd>
			  <dt>
				LOGICAL_WORDS
			  </dt>
			  <dd>
				A string of the search words with either \"and\" or
				\"or\" between the words, depending on the type of
				search.
			  </dd>
			  <dt>
				WORDS
			  </dt>
			  <dd>
				A string of the search words with spaces in
				between.
			  </dd>
			</dl>
			Note that this file will <strong>NOT</strong> be output
			if no matches were found. In this case the
			<a href=\"#nothing_found_file\">nothing_found_file</a>
			attribute is used instead.
			Also, this file will not be output if it is
			overridden by defining the
			<a href=\"#search_results_wrapper\">search_results_wrapper</a>
			attribute.
" },
{ "search_results_wrapper", "", 
	"string", "htsearch", "search_results_wrapper: ${common_dir}/wrapper.html", "
	This specifies a filename to be output at the start and
	end of search results. This file replaces the
	<a href=\"#search_results_header\">search_results_header</a> and
	<a href=\"#search_results_footer\">search_results_footer</a>
	files, with the contents of both in one file, and uses the
	pseudo-variable <strong>$(HTSEARCH_RESULTS)</strong> as a
	separator for the header and footer sections.
	If the filename is not specified, the file is unreadable,
	or the pseudo-variable above is not found, htsearch reverts
	to the separate header and footer files instead.
	While outputting the wrapper,
	some variables will be expanded, just as for the
	<a href=\"#search_results_header\">search_results_header</a> and
	<a href=\"#search_results_footer\">search_results_footer</a>
	files.<br>
	Note that this file will <strong>NOT</strong> be output
	if no matches were found. In this case the
	<a href=\"#nothing_found_file\">nothing_found_file</a>
	attribute is used instead.
" },
{ "server_aliases", "", 
	"string list", "htdig", "server_aliases:
				  foo.mydomain.com:80=www.mydomain.com:80 \\<br>
				  bar.mydomain.com:80=www.mydomain.com:80
", "
	This directive tells the indexer that servers have several
	DNS aliases, which all point to the same machine and are NOT
	virtual hosts. This allows you to ensure pages are indexed
	only once on a given machine, despite the alias used in a URL.
" },
{ "server_max_docs", "-1", 
	"integer", "htdig", "server_max_docs: 50", "
	This directive tells htdig to limit the dig to retrieve a maximum
	number of documents from each server. This can cause
	unusual behavior on update digs since the old URLs are
	stored alphabetically. Therefore, update digs will add
	additional URLs in pseudo-alphabetical order, up to the
	limit of the directive. However, it is most useful to
	partially index a server as the URLs of additional
	documents are entered into the database, marked as never
	retrieved.<br>
        A value of -1 specifies no limit.
" },
{ "server_wait_time", "0", 
	"integer", "htdig", "server_wait_time: 20", "
	This directive tells htdig to ensure a server has had a
	delay (in seconds) from the beginning of the last
	connection. This can be used to prevent \"server abuse\"
	by digging without delay. It's recommended to set this
	to 10-30 (seconds) when indexing servers that you don't
	monitor yourself. Additionally, this directive can slow
	down local indexing if set, which may or may not be what
	you intended.
" },
{ "sort", "score", 
	"string", "htsearch", "sort: revtime", "
	This is the default sorting method that htsearch
	uses to determine the order in which matches are displayed.
	The valid choices are:
	<table border=\"0\">
	<tr>
	<td>
	<ul>
	<li>
	score
	</li>
	<li>
	time
	</li>
	<li>
	title
	</li>
	</ul>
	</td>
	<td>
	<ul>
	<li>
	revscore
	</li>
	<li>
	revtime
	</li>
	<li>
	revtitle
	</li>
	</ul>
	</td>
	</tr>
	</table>
	This attribute will only be used if the HTML form that
	calls htsearch didn't have the <strong>sort</strong>
	value set. The words date and revdate can be used instead
	of time and revtime, as both will sort by the time that
	the document was last modified, if this information is
	given by the server. The default is to sort by the score,
	which ranks documents by best match. The sort methods that
	begin with \"rev\" simply reverse the order of the
	sort. Note that setting this to something other than
	\"score\" will incur a slowdown in searches.
" },
{ "sort_names", "score Score time Time title Title revscore 'Reverse Score' revtime 'Reverse Time' revtitle 'Reverse Title'", 
	"quoted string list", "htsearch", "sort_names:
				  score 'Best Match' time Newest title A-Z \\<br>
				  revscore 'Worst Match' revtime Oldest revtitle Z-A
", "
	These values are used to create the <strong>
	sort</strong> menu. It consists of pairs. The first
	element of each pair is one of the known sort methods, the
	second element is the text that will be shown in the
	menu for that sort method. This text needs to be quoted if
	it contains spaces.
" },
{ "soundex_db", "${database_base}.soundex.db", 
	"string", "htfuzzy htsearch", "soundex_db: ${database_base}.snd.db", "
	The database file used for the fuzzy \"soundex\" search
	algorithm. This database is created by
	<a href=\"htfuzzy.html\">htfuzzy</a> and used by
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>.
" },
{ "star_blank", "${image_url_prefix}/star_blank.gif", 
	"string", "htsearch", "star_blank: http://www.somewhere.org/icons/elephant.gif", "
	This specifies the URL to use to display a blank of the
	same size as the star defined in the
	<a href=\"#image_star\">image_star</a> attribute or in the
	<a href=\"#star_patterns\">star_patterns</a> attribute.
" },
{ "star_image", "${image_url_prefix}/star.gif", 
	"string", "htsearch", "star_image: http://www.somewhere.org/icons/elephant.gif", "
	This specifies the URL to use to display a star. This
	allows you to use some other icon instead of a star.
	(We like the star...)<br>
	The display of stars can be turned on or off with the
	<em><a href=\"#use_star_image\">use_star_image</a></em>
	attribute and the maximum number of stars that can be
	displayed is determined by the
	<em><a href=\"#max_stars\">max_stars</a></em> attribute.<br>
	Even though the image can be changed, the ALT value
	for the image will always be a '*'.
" },
{ "star_patterns", "", 
	"string list", "htsearch", "star_patterns:
				  http://www.sdsu.edu /sdsu.gif \\<br>
				  http://www.ucsd.edu /ucsd.gif
", "
	This attribute allows the star image to be changed
	depending on the URL or the match it is used for. This
	is mainly to make a visual distinction between matches
	on different web sites. The star image could be
	replaced with the logo of the company the match refers
	to.<br>
	It is advisable to keep all the images the same size
	in order to line things up properly in a short result
	listing.<br>
	The format is simple. It is a list of pairs. The first
	element of each pair is a pattern, the second element
	is a URL to the image for that pattern.
" },
{ "start_ellipses", "<b><tt>... </tt></b>", 
	"string", "htsearch", "start_ellipses: ...", "
	When excerpts are displayed in the search output, this
	string will be prepended to the excerpt if there is
	text before the text displayed. This is just a visual
	reminder to the user that the excerpt is only part of
	the complete document.
" },
{ "start_url", "http://www.htdig.org/", 
	"string list", "htdig", "start_url: http://www.somewhere.org/alldata/index.html", "
	This is the list of URLs that will be used to start a
	dig when there was no existing database. Note that
	multiple URLs can be given here.
" },
{ "substring_max_words", "25", 
	"integer", "htsearch", "substring_max_words: 100", "
	The Substring fuzzy algorithm could potentially match a
	very large number of words. This value limits the
	number of words each substring pattern can match. Note
	that this does not limit the number of documents that
	are matched in any way.
" },
{ "synonym_db", "${common_dir}/synonyms.db", 
	"string", "htsearch htfuzzy", "synonym_db: ${database_base}.syn.db", "
	Points to the database that <a href=\"htfuzzy.html\">
	htfuzzy</a> creates when the <strong>synonyms</strong>
	algorithm is used.<br>
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>
	uses this to perform synonym dictionary lookups.
" },
{ "synonym_dictionary", "${common_dir}/synonyms", 
	"string", "htfuzzy", "synonym_dictionary: /usr/dict/synonyms", "
	This points to a text file containing the synonym
	dictionary used for the synonyms search algorithm.<br>
	Each line of this file has at least two words. The
	first word is the word to replace, the rest of the
	words are synonyms for that word.
" },
{ "syntax_error_file", "${common_dir}/syntax.html", 
	"string", "htsearch", "syntax_error_file: ${common_dir}/synerror.html", "
	This points to the file which will be displayed if a
	boolean expression syntax error was found.
" },
{ "template_map", "Long builtin-long builtin-long Short builtin-short builtin-short", 
	"string list", "htsearch", "template_map:
				  Short short ${common_dir}/short.html \\<br>
				  Normal normal builtin-long \\<br>
				  Detailed detail ${common_dir}/detail.html
", "
	This maps match template names to internal names and
	template file names. It is a list of triplets. The
	first element in each triplet is the name that will be
	displayed in the FORMAT menu. The second element is the
	name used internally and the third element is a
	filename of the template to use.<br>
	There are two predefined templates, namely <strong>
	builtin-long</strong> and <strong>
	builtin-short</strong>. If the filename is one of
	those, they will be used instead.<br>
	More information about templates can be found in the
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>
	documentation.
" },
{ "template_name", "builtin-long", 
	"string", "htsearch", "template_name: long", "
	Specifies the default template if none is given by the
	search form. This needs to map to the
	<a href=\"#template_map\">template_map</a>.
" },
{ "template_patterns", "", 
	"string list", "htsearch", "template_patterns:
				  http://www.sdsu.edu ${common_dir}/sdsu.html \\<br>
				  http://www.ucsd.edu ${common_dir}/ucsd.html
", "
	This attribute allows the results template to be changed
	depending on the URL or the match it is used for. This
	is mainly to make a visual distinction between matches
	on different web sites. The results for each site could
	thus be shown in a style matching that site.<br>
	to.<br>
	The format is simply a list of pairs. The first
	element of each pair is a pattern, the second element
	is the name of the template file for that pattern.<br>
	More information about templates can be found in the
	<a href=\"htsearch.html\" target=\"_top\">htsearch</a>
	documentation.
" },
{ "text_factor", "1", 
	"number", "htdig", "text_factor: 0", "
	This is a factor which will be used to multiply the
	weight of words that are not in any special part of a
	document. Setting a factor to 0 will cause normal words
	to be ignored. The number may be a floating point
	number. See also the <a href=\"#heading_factor\">
	heading_factor_[1-6]</a>, <a href=\"#title_factor\">
	title_factor</a>, and <a href=\"#keywords_factor\">
	keywords_factor</a> attributes.
" },
{ "timeout", "30", 
	"number", "htdig", "timeout: 42", "
	Specifies the time the digger will wait to complete a
	network read. This is just a safeguard against
	unforeseen things like the all too common
	transformation from a network to a notwork.<br>
	The timeout is specified in seconds.
" },
{ "title_factor", "100", 
	"number", "htdig", "title_factor: 12", "
	This is a factor which will be used to multiply the
	weight of words in the title of a document. Setting a
	factor to 0 will cause words in the title to be
	ignored. The number may be a floating point number. See
	also the <a href=\"#heading_factor\">
	heading_factor_[1-6]</a> attribute.
" },
{ "translate_amp", "false", 
	"boolean", "htdig", "translate_amp: true", "
	If set to false, the entity &amp;amp; (or &amp;#38;) will not be
	translated into its ASCII equivalent &amp;. If translation were taking
	place, an excerpt containing a &amp; might be misinterpreted by the browser
	and look unrecognizable to the user. For this reason, not translating this
	entity is the default behavior.
" },
{ "translate_lt_gt", "false", 
	"boolean", "htdig", "translate_lt_gt: true", "
	If set to false, the entities &amp;lt; (or &amp;#60;) and &amp;gt;
	(or &amp;#62;) will not be translated into their ASCII equivalents &lt; and
	&gt;. If translation were taking place, an excerpt containing &lt; and
	&gt; might be misinterpreted by the browser and look unrecognizable to the
	user. For this reason, not translating these entities is the default
	behavior.
" },
{ "translate_quot", "false", 
	"boolean", "htdig", "translate_quot: true", "
	If set to false, the entity &amp;quot; (or &amp;#34;) will not be
	translated into its ASCII equivalent &quot;. If translation were taking
	place, an excerpt containing a &quot; might be misinterpreted by the browser
	and look unrecognizable to the user. For this reason, not translating this
	entity is the default behavior.
" },
{ "uncoded_db_compatible", "true", 
	"boolean", "htdig htnotify htmerge htsearch", "uncoded_db_compatible: false<p>", "
	At the cost of time for extra database accesses and
	not getting the full effect of the options
	<a href=\"#common_url_parts\">common_url_parts</a> and
	<a href=\"#url_part_aliases\">url_part_aliases</a>,
	read databases where some or all URLs are not
	encoded <b>at all</b> through these options.<br>
	Note that the database still needs to be rebuilt
	if either or both of common_url_parts and
	url_part_aliases were non-empty when it was built or
	modified, or if they were set to anything else than
	the current values.<br>
	If a <em>to</em>-string in url_part_aliases can
	occur in normal URLs, this option should be set to
	<b>false</b> to eliminate surprises.<br>
" },
{ "url_factor", "2", 
	"", "", "", "

" },
{ "url_list", "${database_base}.urls", 
	"string", "htdig", "url_list: /tmp/urls", "
	This file is only created if
	<em><a href=\"#create_url_list\">create_url_list</a></em> is set to
	true. It will contain a list of all URLs that were
	seen.
" },
{ "url_log", "${database_base}.log", 
	"string", "htdig", "url_log: /tmp/htdig.progress", "
	If <a href=\"htdig.html\">htdig</a> is run with the -l option
	and interrupted, it will write out its progress to this
	file. Note that if it has a large number of URLs to write,
	it may take some time to exit. This can especially happen
	when running update digs and the run is interrupted soon
	after beginning.
" },
{ "url_part_aliases", "", 
	"string list", "htdig htnotify htmerge htsearch", "url_part_aliases:
				   http://search.example.com/~htdig *site \\<br>
				   http://www.htdig.org/this/ *1 \\<br>
				   .html *2
url_part_aliases:
				   http://www.htdig.org/ *site \\<br>
-                                  http://www.htdig.org/that/ *1 \\<br>
-                                  .htm *2
", "
	A list of translations pairs <em>from</em> and
	<em>to</em>, used when accessing the database.
	If a part of an URL matches with the
	<em>from</em>-string of each pair, it will be
	translated into the <em>to</em>-string just before
	writing the URL to the database, and translated
	back just after reading it from the database.<br>
	This is primarily used to provide an easy way to
	rename parts of URLs for e.g. changing
	www.example.com/~htdig to www.htdig.org.  Two
	different configuration files for digging and
	searching are then used, with url_part_aliases
	having different <em>from</em> strings, but
	identical <em>to</em>-strings.<br>
	See also <a
	href=\"#common_url_parts\">common_url_parts</a>.<br>
	Strings that are normally incorrect in URLs or
	very seldom used, should be used as
	<em>to</em>-strings, since extra storage will be
	used each time one is found as normal part of a
	URL.  Translations will be performed with priority
	for the leftmost longest match.	 Each
	<em>to</em>-string must be unique and not be a
	part of any other <em>to</em>-string.<br>
	Note that when this attribute is changed, the
	database should be rebuilt, unless the effect of
	\"moving\" the affected URLs in the database is
	wanted, as described above.
" },
{ "use_doc_date", "false", 
	"", "", "", "

" },
{ "use_meta_description", "false", 
	"boolean", "htsearch", "use_meta_description: true", "
	If set to true, any META description tags will be used as
	excerpts by htsearch. Any documents that do not have META
	descriptions will retain their normal excerpts.
" },
{ "use_star_image", "true", 
	"boolean", "htsearch", "use_star_image: no", "
	If set to true, the <em><a href=\"#star_image\">
	star_image</a></em> attribute is used to display upto
	<em><a href=\"#max_stars\">max_stars</a></em> images for
	each match.
" },
{ "user_agent", "htdig", 
	"string", "htdig", "user_agent: htdig-digger", "
	This allows customization of the user_agent: field sent when
	the digger requests a file from a server.
" },
{ "valid_extensions", "", 
	"", "", "", "

" },
{ "valid_punctuation", ".-_/!#$%^&'", 
	"string", "htdig htsearch", "valid_punctuation: -'", "
	This is the set of characters which will be deleted
	from the document before determining what a word is.
	This means that if a document contains something like
	<tt>Andrew's</tt> the digger will see this as <tt>
	Andrews</tt>.<br>
	The same transformation is performed on the keywords
	the search engine gets.<br>
	See also the <a
	href=\"#extra_word_characters\">extra_word_characters</a>
	attribute.
" },
{ "version", VERSION, 
	"string", "htsearch", "version: 3.2.0", "
	This specifies the value of the VERSION
	variable which can be used in search templates.
	The default value of this attribute is determined
	at compile time, and will not normally be set
	in configuration files.
" },
{ "word_db", "${database_base}.words.db", 
	"string", "htdig htmerge htsearch", "word_db: ${database_base}.allwords.db", "
	This is the main word database. It is an index of all
	the words to a list of documents that contain the
	words. This database can grow large pretty quickly.
" },
{ "word_dump", "${database_base}.worddump", 
	"string", "htdig", "word_dump: /tmp/words.txt", "
	This file is basically a text version of the file
	specified in <em><a href=\"#word_db\">word_db</a></em>. Its
	only use is to have a human readable database of all
	words. The file is easy to parse with tools like
	perl or tcl.
" },
{ "word_list", "${database_base}.wordlist", 
	"string", "htdig htmerge", "word_list: ${database_base}.allwords.text", "
	This is the input file that <a href=\"htmerge.html\">
	htmerge</a> uses to create the main words database
	specified by <em><a href=\"#word_db\">word_db</a></em>.
	This file gets about as large as the main words
	database. If this file exists when htdig is running, it
	will append data to this file. htmerge will then use
	the existing data and the appended data to create a
	completely new main word database.
" },
{0, 0, 0, 0, 0},
};

Configuration	config;
