//
// defaults.cc
//
// default values for the ht programs
//
// $Log: defaults.cc,v $
// Revision 1.21  1998/11/27 18:31:45  ghutchis
//
// Changed backlink_factor to 1000, description_factor to 150, match_method to
// and, and meta_description factor to 50. Should produce more accurate search
// results.
//
// Revision 1.20  1998/11/22 19:13:38  ghutchis
//
// New config options "description_factor" and "no_excerpt_show_top"
//
// Revision 1.19  1998/11/17 04:06:14  ghutchis
//
// Add new ranking factors backlink_factor and date_factor
//
// Revision 1.18  1998/10/21 17:33:03  ghutchis
//
// Added defaults for server_aliases and limit_normalized
//
// Revision 1.17  1998/10/17 14:06:08  ghutchis
//
// Changed htdig.sdsu.edu to www.htdig.org in start_urls
//
// Revision 1.16  1998/10/12 02:09:28  ghutchis
//
// Added htsearch logging patch from Alexander Bergolth.
//
// Revision 1.13  1998/09/23 14:58:21  ghutchis
//
// Many, many bug fixes
//
// Revision 1.12  1998/09/08 03:29:09  ghutchis
//
// Clean up for 3.1.0b1.
//
// Revision 1.11  1998/09/07 04:27:39  ghutchis
//
// Bug fixes.
//
// Revision 1.10  1998/08/11 08:58:26  ghutchis
// Second patch for META description tags. New field in DocDB for the
// desc., space in word DB w/ proper factor.
//
// Revision 1.9  1998/08/06 14:18:30  ghutchis
// Added config option "local_default_doc" for default filename in a local
// directory. Fixed spelling mistake in "elipses" attributes.
//
// Revision 1.8  1998/07/23 16:18:51  ghutchis
//
// Added files (and patch) from Sylvain Wallez for PDF
// parsing. Incorporates fix for non-Adobe PDFs.
//
// Revision 1.7  1998/07/09 09:38:56  ghutchis
//
//
// Added support for local file digging using patches by Pasi. Patches
// include support for local user (~username) digging.
//
// Revision 1.6  1998/07/09 09:32:02  ghutchis
//
// Added support for META name=description tags. Uses new config-file
// option "use_meta_description" which is off by default.
//
// Revision 1.5  1998/06/22 04:35:43  turtle
// Got rid of my email address as the default maintainer
//
// Revision 1.4  1998/06/21 23:20:01  turtle
// patches by Esa and Jesse to add BerkeleyDB and Prefix searching
//
// Revision 1.3  1997/07/03 17:44:38  turtle
// Added support for virtual hosts
//
// Revision 1.2  1997/03/14 17:15:32  turtle
// Changed default value for remove_bad_urls to true
//
// Revision 1.1.1.1  1997/02/03 17:11:07  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: defaults.cc,v 1.21 1998/11/27 18:31:45 ghutchis Exp $";
#endif

#include <Configuration.h>

ConfigDefaults	defaults[] =
{
    //
    // These variables are set to whatever the system was configured for.
    //
    {"common_dir",			COMMON_DIR},
    {"config_dir",			CONFIG_DIR},
    {"database_dir",			DATABASE_DIR},
    {"bin_dir",				BIN_DIR},
    {"image_url_prefix",		IMAGE_URL_PREFIX},
    {"pdf_parser",                      PDF_PARSER},

    //
    // General defaults
    //
    {"add_anchors_to_excerpt",		"true"},
    {"allow_numbers",			"false"},
    {"allow_virtual_hosts",		"true"},
    {"backlink_factor",                 "1000"},
    {"bad_extensions",			".wav .gz .z .sit .au .zip .tar .hqx .exe .com .gif .jpg .jpeg .aiff .class .map .ram"},
    {"bad_word_list",			"${common_dir}/bad_words"},
    {"case_sensitive"                   "true"},
    {"create_image_list",		"false"},
    {"create_url_list",			"false"},
    {"date_factor",                     "0"},
    {"database_base",			"${database_dir}/db"},
    {"description_factor",              "150"},
    {"doc_db",				"${database_base}.docdb"},
    {"doc_index",			"${database_base}.docs.index"},
    {"doc_list",			"${database_base}.docs"},
    {"end_ellipses",			"<b><tt> ...</tt></b>"},
    {"endings_affix_file",		"${common_dir}/english.aff"},
    {"endings_dictionary",		"${common_dir}/english.0"},
    {"endings_root2word_db",		"${common_dir}/root2word.db"},
    {"endings_word2root_db",		"${common_dir}/word2root.db"},
    {"excerpt_length",			"300"},
    {"excerpt_show_top",		"false"},
    {"exclude_urls",			"/cgi-bin/ .cgi"},
    {"external_parsers",		""},
    {"heading_factor_1",		"5"},
    {"heading_factor_2",		"4"},
    {"heading_factor_3",		"3"},
    {"heading_factor_4",		"1"},
    {"heading_factor_5",		"1"},
    {"heading_factor_6",		"0"},
    {"htnotify_sender",			"webmaster@www"},
    {"http_proxy",			""},
    {"image_list",			"${database_base}.images"},
    {"iso_8601",                        "false"},
    {"keywords_factor",			"100"},
    {"keywords_meta_tag_names",		"keywords htdig-keywords"},
    {"limit_urls_to",			"${start_url}"},
    {"limit_normalized",                ""},
    {"locale",				"iso_8859_1"},
    {"local_default_doc",               "index.html"},
    {"local_urls",			""},
    {"local_user_urls",			""},
    {"logging",                         "false"},
    {"maintainer",			"bogus@unconfigured.htdig.user"},
    {"match_method",			"and"},
    {"matches_per_page",		"10"},
    {"max_description_length",		"60"},
    {"max_doc_size",			"100000"},
    {"max_head_length",			"512"},
    {"max_hop_count",			"999999"},
    {"max_meta_description_length",     "512"},
    {"max_prefix_matches",		"1000"},
    {"max_stars",			"4"},
    {"maximum_pages",			"10"},
    {"metaphone_db",			"${database_base}.metaphone.db"},
    {"meta_description_factor",		"50"},
    {"method_names",			"and All or Any boolean Boolean"},
    {"minimum_word_length",		"3"},
    {"minimum_prefix_length",		"1"},
    {"modification_time_is_now",        "false"},
    {"next_page_text",			"[next]"},
    {"no_excerpt_text",			"<em>(None of the search words were found in the top of this document.)</em>"},
    {"no_excerpt_show_top",             "false"},
    {"no_next_page_text",		"[next]"},
    {"no_prev_page_text",		"[prev]"},
    {"nothing_found_file",		"${common_dir}/nomatch.html"},
    {"page_list_header",		"<hr noshade size=2>Pages:<br>"},
    {"prefix_match_character",		"*"},
    {"prev_page_text",			"[prev]"},
    {"remove_bad_urls",			"true"},
    {"robotstxt_name",			"htdig"},
    {"search_algorithm",		"exact:1"},
    {"search_results_footer",		"${common_dir}/footer.html"},
    {"search_results_header",		"${common_dir}/header.html"},
    {"server_aliases",                  ""},
    {"soundex_db",			"${database_base}.soundex.db"},
    {"star_blank",			"${image_url_prefix}/star_blank.gif"},
    {"star_image",			"${image_url_prefix}/star.gif"},
    {"star_patterns",			""},
    {"start_ellipses",			"<b><tt>... </tt></b>"},
    {"start_url",			"http://www.htdig.org/"},
    {"substring_max_words",		"25"},
    {"synonym_db",			"${common_dir}/synonyms.db"},
    {"synonym_dictionary",		"${common_dir}/synonyms"},
    {"syntax_error_file",		"${common_dir}/syntax.html"},
    {"template_map",			"Long builtin-long builtin-long Short builtin-short builtin-short"},
    {"template_name",			"builtin-long"},
    {"text_factor",			"1"},
    {"timeout",				"30"},
    {"title_factor",			"100"},
    {"url_list",			"${database_base}.urls"},
    {"use_star_image",			"true"},
    {"use_meta_description",            "false"},
    {"user_agent",			"htdig"},
    {"valid_punctuation",		".-_/!#$%^&'"},
    {"version",				VERSION},
    {"word_db",				"${database_base}.words.db"},
    {"word_list",			"${database_base}.wordlist"},
    {0,					0},
};

Configuration	config;

