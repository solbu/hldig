//
// defaults.cc
//
// default values for the ht programs
//
// $Log: defaults.cc,v $
// Revision 1.1  1997/02/03 17:11:07  turtle
// Initial revision
//
//
#if RELEASE
static char RCSid[] = "$Id: defaults.cc,v 1.1 1997/02/03 17:11:07 turtle Exp $";
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

    //
    // General defaults
    //
    {"add_anchors_to_excerpt",		"true"},
    {"allow_numbers",			"false"},
    {"bad_extensions",			".wav .gz .z .sit .au .zip .tar .hqx .exe .com .gif .jpg .jpeg .aiff .pdf .class .map .ram"},
    {"bad_word_list",			"${common_dir}/bad_words"},
    {"create_image_list",		"false"},
    {"create_url_list",			"false"},
    {"database_base",			"${database_dir}/db"},
    {"doc_db",				"${database_base}.docdb"},
    {"doc_index",			"${database_base}.docs.index"},
    {"doc_list",			"${database_base}.docs"},
    {"end_elipses",			"<b><tt> ...</tt></b>"},
    {"endings_affix_file",		"${common_dir}/english.aff"},
    {"endings_dictionary",		"${common_dir}/english.0"},
    {"endings_root2word_db",		"${common_dir}/root2word.gdbm"},
    {"endings_word2root_db",		"${common_dir}/word2root.gdbm"},
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
    {"keywords_factor",			"100"},
    {"keywords_meta_tag_names",		"keywords htdig-keywords"},
    {"limit_urls_to",			"${start_url}"},
    {"locale",				"iso_8859_1"},
    {"maintainer",			"andrew@contigo.com"},
    {"match_method",			"or"},
    {"matches_per_page",		"10"},
    {"max_description_length",		"60"},
    {"max_doc_size",			"100000"},
    {"max_head_length",			"512"},
    {"max_hop_count",			"999999"},
    {"max_stars",			"4"},
    {"maximum_pages",			"10"},
    {"metaphone_db",			"${database_base}.metaphone.gdbm"},
    {"method_names",			"and All or Any boolean Boolean"},
    {"minimum_word_length",		"3"},
    {"next_page_text",			"[next]"},
    {"no_excerpt_text",			"<em>(None of the search words were found in the top of this document.)</em>"},
    {"no_next_page_text",		"[next]"},
    {"page_list_header",		""},
    {"no_prev_page_text",		"[prev]"},
    {"nothing_found_file",		"${common_dir}/nomatch.html"},
    {"page_list_header",		"<hr noshade size=2>Pages:<br>"},
    {"prev_page_text",			"[prev]"},
    {"remove_bad_urls",			"false"},
    {"robotstxt_name",			"htdig"},
    {"search_algorithm",		"exact:1"},
    {"search_results_footer",		"${common_dir}/footer.html"},
    {"search_results_header",		"${common_dir}/header.html"},
    {"soundex_db",			"${database_base}.soundex.gdbm"},
    {"star_image",			"${image_url_prefix}/star.gif"},
    {"star_blank",			"${image_url_prefix}/star_blank.gif"},
    {"star_patterns",			""},
    {"start_elipses",			"<b><tt>... </tt></b>"},
    {"start_url",			"http://htdig.sdsu.edu/"},
    {"substring_max_words",		"25"},
    {"synonym_dictionary",		"${common_dir}/synonyms"},
    {"synonym_db",			"${common_dir}/synonyms.gdbm"},
    {"syntax_error_file",		"${common_dir}/syntax.html"},
    {"template_map",			"Long builtin-long builtin-long Short builtin-short builtin-short"},
    {"template_name",			"builtin-long"},
    {"text_factor",			"1"},
    {"timeout",				"30"},
    {"title_factor",			"100"},
    {"url_list",			"${database_base}.urls"},
    {"use_star_image",			"true"},
    {"valid_punctuation",		".-_/!#$%^&*'"},
    {"version",				HTDIG_VERSION},
    {"word_db",				"${database_base}.words.gdbm"},
    {"word_list",			"${database_base}.wordlist"},
    {0,					0},
};

Configuration	config;

