//
// HTML.cc
//
// HTML: Class to parse HTML documents and return useful information
//       to the Retriever
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2001 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
#if RELEASE
static char RCSid[] = "$Id: HTML.cc,v 1.30.2.23 2001/11/02 18:29:55 grdetil Exp $";
#endif

#include "htdig.h"
#include "HTML.h"
#include "SGMLEntities.h"
#include "Configuration.h"
#include "StringMatch.h"
#include "StringList.h"
#include "URL.h"
#include "HtWordType.h"

// Flags for noindex & nofollow, indicating who turned indexing off/on...
#define TAGnoindex		0x0001
#define TAGstyle		0x0002
#define TAGscript		0x0004
#define TAGmeta_htdig_noindex	0x0008
#define TAGmeta_robots		0x0010

static StringMatch	tags;
static StringMatch	nobreaktags;
static StringMatch	spacebeforetags;
static StringMatch	spaceaftertags;
static StringMatch	metadatetags;
static StringMatch	keywordsMatch;
static int		keywordsCount;
static int		max_keywords;
static int		offset;
static int		totlength;


//*****************************************************************************
// ADDSPACE() macro, to insert space where needed in various strings
// 		Reduces all multiple whitespace to a single space

#define ADDSPACE(in_space)	\
    if (!in_space)							\
    {									\
	if (in_title && !noindex)					\
	{								\
	    title << ' ';						\
	}								\
	if (in_ref && description.length() < max_description_length)	\
	{								\
	    description << ' ';						\
	}								\
	if (head.length() < max_head_length && !noindex && !in_title)	\
	{								\
	    head << ' ';						\
	}								\
	in_space = 1;							\
    }


//*****************************************************************************
// HTML::HTML()
//
HTML::HTML()
{
    //
    // Initialize the patterns that we will try to match.
    // The tags Match object is used to match tag commands while
    //
    tags.IgnoreCase();
    tags.Pattern("title|/title|a|/a|h1|h2|h3|h4|h5|h6|/h1|/h2|/h3|/h4|/h5|/h6|noindex|/noindex|img|li|meta|frame|area|base|embed|object|link|style|/style|script|/script");

    // These tags don't cause a word break.  They may also be in "tags" above,
    // except for the "a" tag, which must be handled as a special case.
    // Note that <sup> & <sub> should cause a word break.
    nobreaktags.IgnoreCase();
    nobreaktags.Pattern("font|/font|em|/em|strong|/strong|i|/i|b|/b|u|/u|tt|/tt|abbr|/abbr|code|/code|q|/q|samp|/samp|kbd|/kbd|var|/var|dfn|/dfn|cite|/cite|blink|/blink|big|/big|small|/small|s|/s");

    // These tags, which may also be in "tags" above, cause word breaks and
    // therefore cause space to be inserted before (or after) do_tag() is done.
    spacebeforetags.IgnoreCase();
    spacebeforetags.Pattern("title|h1|h2|h3|h4|h5|h6|address|blockquote|noindex|img|li|th|td|dt|dd|p|br|hr|center|spacer");
    spaceaftertags.IgnoreCase();
    spaceaftertags.Pattern("/title|/h1|/h2|/h3|/h4|/h5|/h6|/address|/blockquote");

    // These are the name values of meta tags that carry date information.
    metadatetags.IgnoreCase();
    metadatetags.Pattern("date|dc.date|dc.date.created|dc.data.modified");

    //String	keywordNames = config["keywords_meta_tag_names"];
    //keywordNames.replace(' ', '|');
    //keywordNames.remove(",\t\r\n");
    //keywordsMatch.IgnoreCase();
    //keywordsMatch.Pattern(keywordNames);
    StringList keywordNames(config["keywords_meta_tag_names"], " \t");
    keywordsMatch.IgnoreCase();
    keywordsMatch.Pattern(keywordNames.Join('|'));
    keywordNames.Release();
    max_keywords = config.Value("max_keywords", -1);
    if (max_keywords < 0)
	max_keywords = (int) ((unsigned int) ~1 >> 1);
    
    word = 0;
    href = 0;
    title = 0;
    description = 0;
    head = 0;
    meta_dsc = 0;
    tag = 0;
    in_title = 0;
    in_ref = 0;
    in_heading = 0;
    base = 0;
    noindex = 0;
    nofollow = 0;
    minimumWordLength = config.Value("minimum_word_length", 3);
}


//*****************************************************************************
// HTML::~HTML()
//
HTML::~HTML()
{
}


//*****************************************************************************
// void HTML::parse(Retriever &retriever, URL &baseURL)
//   Parse the HTML document using the Retriever object for all the callbacks.
//   The HTML document contents are contained in the contents String.
//
void
HTML::parse(Retriever &retriever, URL &baseURL)
{
    if (contents == 0 || contents->length() == 0)
	return;

    base = &baseURL;
    
    //
    // We have some variables which will contain the various items we
    // are looking for
    //
    int			in_space;
    int			in_punct;
    unsigned char	*q, *start;
    unsigned char	*position = (unsigned char *) contents->get();
    unsigned char       *text = (unsigned char *) new char[contents->length()+1];
    unsigned char       *ptext = text;
    static char         *skip_start = config["noindex_start"];
    static char         *skip_end = config["noindex_end"];
    int			skip_start_len = strlen(skip_start);
    int			skip_end_len = strlen(skip_end);

    keywordsCount = 0;
    offset = 0;
    title = 0;
    head = 0;
    meta_dsc = 0;
    noindex = 0;
    nofollow = 0;
    in_heading = 0;
    in_title = 0;
    in_ref = 0;
    in_space = 0;
    in_punct = 0;
	
    while (*position)
    {

      //
      // Filter out section marked to be ignored for indexing. 
      // This can contain any HTML. 
      //
      if (*skip_start &&
	  mystrncasecmp((char *)position, skip_start, skip_start_len) == 0)
	{
	  q = (unsigned char*)mystrcasestr((char *)position, skip_end);
	  if (!q)
	    *position = '\0';       // Rest of document will be skipped...
	  else
	    position = q + skip_end_len;
	  continue;
	}

      if (strncmp((char *)position, "<!", 2) == 0)
	{
	  //
	  // Possible comment declaration (but could be DTD declaration!)
	  // A comment can contain other '<' and '>':
	  // we have to ignore complete comment declarations
	  // but of course also DTD declarations.
	  //
	  position += 2;	// Get past declaration start
	  if (strncmp((char *)position, "--", 2) == 0)
	    {
	      // Found start of comment - now find the end
	      position += 2;
	      do
		{
		  q = (unsigned char*)strstr((char *)position, "--");
		  if (!q)
		    {
		      *position = '\0';
		      break;	// Rest of document seems to be a comment...
		    }
		  else
		    {
		      position = q + 2;
		      // Skip extra dashes after a badly formed comment
		      while (*position == '-')
			  position++;
		      // Skip whitespace after an individual comment
		      while (isspace(*position))
			  position++;
		    }
		  // if comment declaration hasn't ended, skip another comment
		}
	      while (*position && *position != '>');
	      if (*position == '>')
		{
		  position++;	// End of comment declaration
		}
	    }
	  else
	    {
	      // Not a comment declaration after all
	      // but possibly DTD: get to the end
	      q = (unsigned char*)strchr((char *)position, '>');
	      if (q)
		{
		  position = q + 1;
		  // End of (whatever) declaration
		}
	      else
		{
		  *position = '\0'; // Rest of document is DTD?
		}
	    }
	  continue;
	}

	if (*position == '<')
	{
	    //
	    // Start of a tag.  Since tags cannot be nested, we can simply
	    // search for the closing '>'
	    //
	    q = (unsigned char*)strchr((char *)position, '>');
	    if (q)
	      { // copy tag
		while (position <= q)
		  *ptext++ = *position++;
	      }
	    else
	      { // copy rest of text, as tag does not end
		while (*position)
		  *ptext++ = *position++;
	      }
	}
	else if (*position == '&')
	{
           *ptext = SGMLEntities::translateAndUpdate(position);
           if (*ptext == '<') 
           {
              // got a decoded &lt;, make a fake tag for it
              // to avoid confusing it with real tag start
              *ptext++ = '<';
              *ptext++ = '~';
              *ptext = '>';
           }
           ptext++;
        }
        else
        {
           *ptext++ = *position++;
        }
      }
      *ptext++ = '\0';
      totlength = ptext - text;

      position = text;
      start = position;

      while (*position)
      {
	offset = position - start;
	// String = 0 is expensive
	// word = 0;
	if (*position == '<' && (position[1] != '~' || position[2] != '>'))
	  {
	    //
	    // Start of a tag.  Since tags cannot be nested, we can simply
	    // search for the closing '>'
	    //
	    q = (unsigned char*)strchr((char *)position, '>');
	    if (!q)
	      break; // Syntax error in the doc.  Tag never ends.
	    position++;
	    tag = 0;
	    tag.append((char*)position, q - position);
	    while (isspace(*position))
		position++;
	    if (!in_space && spacebeforetags.CompareWord((char *)position)
		|| !in_space && !in_punct && *position != '/')
	    {
		// These opening tags cause a space to be inserted
		// before anything they insert.
		// Tags processed here (i.e. not in nobreaktags), like <a ...>
		// tag, are a special case: they don't actually add space in
		// formatted text, but because in our processing it causes
		// a word break, we avoid word concatenation in "head" string.
		ADDSPACE(in_space);
		in_punct = 0;
	    }
	    do_tag(retriever, tag);
	    if (!in_space && spaceaftertags.CompareWord((char *)position))
	    {
		// These closing tags cause a space to be inserted
		// after anything they insert.
		ADDSPACE(in_space);
		in_punct = 0;
	    }
	    position = q+1;
	  }
	else if (*position > 0 && HtIsStrictWordChar(*position))
	{
	    //
	    // Start of a word.  Try to find the whole thing
	    //
	    word = 0;
	    in_space = 0;
	    in_punct = 0;
	    while (*position && HtIsWordChar(*position))
	    {
		word << (char)*position;
		// handle case where '<' is in extra_word_characters...
		if (strncmp((char *)position, "<~>", 3) == 0)
		    position += 2;      // skip over fake tag for decoded '<'
		position++;
		if (*position == '<')
		{
		    q = position+1;
		    while (isspace(*q))
			q++;
		    // Does this tag cause a word break?
		    if (nobreaktags.CompareWord((char *)q))
		    {
			// These tags just change character formatting and
			// don't break words.
			q = (unsigned char*)strchr((char *)position, '>');
			if (q)
			{
			    position++;
			    tag = 0;
			    tag.append((char*)position, q - position);
			    do_tag(retriever, tag);
			    position = q+1;
			}
		    }
		}
	    }

	    if (in_title && !noindex)
	    {
		title << word;
	    }

	    if (in_ref)
	    {
		if (description.length() < max_description_length)
		{
		    description << word;
		}
		else
		{
		    description << " ...";
		    if (!nofollow)
		      retriever.got_href(*href, description);
		    in_ref = 0;
		    description = 0;
		}
	    }

	    if (head.length() < max_head_length && !noindex && !in_title)
	    {
		//
		// Capitalize H1 and H2 blocks
		// (This is currently disabled until we can captialize
	        // non-ASCII characters -GRH
	        // if (in_heading > 1 && in_heading < 4)
	        // {
	        //   word.uppercase();
	        // }

		//
		// Append the word to the head (excerpt)
		//
		  head << word;
	    }

	    if (word.length() >= minimumWordLength && !noindex)
	    {
	      retriever.got_word(word,
				 int(offset * 1000 / totlength),
				 in_heading);
	    }
	}
	else
	{
	    //
	    // Characters that are not part of a word
	    //
	    if (isspace(*position))
	    {
		ADDSPACE(in_space);
		in_punct = 0;
	    }
	    else
	    {
		//
		// Not whitespace
		//
		if (head.length() < max_head_length && !noindex && !in_title)
		{
		    // We don't want to add random chars to the 
		    // excerpt if we're in the title.
		    head << *position;
		}
		if (in_ref && description.length() < max_description_length)
		{
		    description << *position;
		}
		if (in_title && !noindex)
		{
		    title << *position;
		}
		in_space = 0;
		in_punct = 1;
		// handle normal case where decoded '<' is punctuation...
		if (strncmp((char *)position, "<~>", 3) == 0)
		    position += 2;      // skip over fake tag for decoded '<'
	    }
	    position++;
	}
    }
    retriever.got_head(head);

    delete [] text;
}


//*****************************************************************************
// void HTML::do_tag(Retriever &retriever, String &tag)
//
void
HTML::do_tag(Retriever &retriever, String &tag)
{
    static int	ignore_alt_text = config.Boolean("ignore_alt_text", 0);
    char	*position = tag.get();
    int		which, length;

    while (isspace(*position))
	position++;

    which = -1;
    if (tags.CompareWord(position, which, length) < 0)
	return; // Nothing matched.

    // Use the configuration code to match attributes as key-value pairs
    Configuration	attrs;
    attrs.NameValueSeparators("=");
    attrs.Add(position);

    if (debug > 3)
	cout << "Tag: <" << tag << ">, matched " << which << endl;
    
    switch (which)
    {
	case 0:		// "title"
	    if (title.length())
	    {
		if (debug)
		    cout << "More than one <title> tag in document!"
			 << " (possible search engine spamming)" << endl;
		break;
	    }
	    in_title = 1;
	    in_heading = 1;
	    break;
			
	case 1:		// "/title"
	    if (!in_title)
		break;
	    in_title = 0;
	    in_heading = 0;
	    retriever.got_title(title);
	    break;
			
	case 2:		// "a"
	{
	  if (attrs["href"])
	    {
	      //
	      // a href seen
	      //
	      if (in_ref)
		{
		  if (debug > 1)
		    cout << "Terminating previous <a href=...> tag,"
			 << " which didn't have a closing </a> tag."
			 << endl;
		  if (!nofollow)
		      retriever.got_href(*href, description);
		  in_ref = 0;
		}
	      if (href)
		delete href;
	      href = new URL(transSGML(attrs["href"]), *base);
	      in_ref = 1;
	      description = 0;
	      break;
	    }
	  
	  if (attrs["title"] && attrs["href"])
	    {
	      //
	      // a title seen for href
	      //
	      retriever.got_href(*href, transSGML(attrs["title"]));
	    }

	  if (attrs["name"])
	    {
	      //
	      // a name seen
	      //
	      retriever.got_anchor(transSGML(attrs["name"]));
	    }
	  break;
	}
				   
	case 3:		// "/a"
	    if (in_ref)
	    {
	      if (!nofollow)
		retriever.got_href(*href, description);
	      in_ref = 0;
	    }
	    break;

	case 4:		// "h1"
	    in_heading = 2;
	    break;

	case 5:		// "h2"
	    in_heading = 3;
	    break;

	case 6:		// "h3"
	    in_heading = 4;
	    break;

	case 7:		// "h4"
	    in_heading = 5;
	    break;

	case 8:		// "h5"
	    in_heading = 6;
	    break;

	case 9:		// "h6"
	    in_heading = 7;
	    break;

	case 10:	// "/h1"
	case 11:	// "/h2"
	case 12:	// "/h3"
	case 13:	// "/h4"
	case 14:	// "/h5"
	case 15:	// "/h6"
	    in_heading = 0;
	    break;

	case 16:	// "noindex"
	    noindex |= TAGnoindex;
	    nofollow |= TAGnoindex;
	    if (attrs["follow"])
		nofollow &= ~TAGnoindex;
	    break;

	case 27:	// "style"
	    noindex |= TAGstyle;
	    nofollow |= TAGstyle;
	    break;

        case 29:        // "script"
	    noindex |= TAGscript;
	    nofollow |= TAGscript;
	    break;

	case 17:	// "/noindex"
	    noindex &= ~TAGnoindex;
	    nofollow &= ~TAGnoindex;
	    break;

	case 28:	// "/style"
	    noindex &= ~TAGstyle;
	    nofollow &= ~TAGstyle;
	    break;

        case 30:	// "/script"
	    noindex &= ~TAGscript;
	    nofollow &= ~TAGscript;
	    break;

	case 18:	// "img"
	  {
	    if (!ignore_alt_text && attrs["alt"])
	      {
		char	*alttxt = transSGML(attrs["alt"]);
		if (!noindex && in_title)
		    title << alttxt << " ";
		if (in_ref && description.length() < max_description_length)
		    description << alttxt << " ";
		if (!noindex && !in_title && head.length() < max_head_length)
		    head << alttxt << " ";
		char *w = HtWordToken(alttxt);
		while (w && !noindex)
		  {
		    if (strlen(w) >= minimumWordLength)
		      retriever.got_word(w,
				 int((offset+(w-alttxt)) * 1000
					/ totlength),
				 in_heading);
		    w = HtWordToken(0);
		  }
		w = '\0';
	      }
	    if (attrs["src"])
	      {
		retriever.got_image(transSGML(attrs["src"]));
	      }
	    break;
	  }

	case 19:	// "li"
	    if (!noindex && !in_title && head.length() < max_head_length)
		head << "* ";
	    break;

	case 20:	// "meta"
	{
	    //
	    // First test for old-style meta tags (these break any
	    // reasonable DTD...)
	    //
	    if (attrs["htdig-noindex"])
	      {
		retriever.got_noindex();
		noindex |= TAGmeta_htdig_noindex;
		nofollow |= TAGmeta_htdig_noindex;
	      }
	    if (attrs["htdig-index"])
	      {
		noindex &= ~TAGmeta_htdig_noindex;
		nofollow &= ~TAGmeta_htdig_noindex;
	      }
	    if (attrs["htdig-email"])
	      retriever.got_meta_email(transSGML(attrs["htdig-email"]));

	    if (attrs["htdig-notification-date"])
	      retriever.got_meta_notification(transSGML(attrs["htdig-notification-date"]));

	    if (attrs["htdig-email-subject"])
	      retriever.got_meta_subject(transSGML(attrs["htdig-email-subject"]));

	    if (attrs["htdig-keywords"] || attrs["keywords"])
	    {
		//
		// Keywords are added as being at the very top of the
		// document and have a weight factor of
		// keywords-factor which is assigned to slot 10 in the
		// factor table.
		//
		char	*keywords = attrs["htdig-keywords"];
		if (!keywords)
		    keywords = attrs["keywords"];
		char	*w = HtWordToken(transSGML(keywords));
		while (w && !noindex)
		{
		    if (strlen(w) >= minimumWordLength
				&& ++keywordsCount <= max_keywords)
		      retriever.got_word(w, 1, 10);
		    w = HtWordToken(0);
		}
		w = '\0';
	    }
	
	    if (attrs["http-equiv"])
	      {

		// <META HTTP-EQUIV=REFRESH case
		if (mystrcasecmp(attrs["http-equiv"], "refresh") == 0
		    && attrs["content"])
		  {
		    char *content = attrs["content"];
		    char *q = mystrcasestr(content, "url");
		    if (q && *q)
		      {
			q += 3; // skiping "URL"
			while (*q && ((*q == '=') || isspace(*q))) q++;
			char *qq = q;
			while (*qq && (*qq != ';') && (*qq != '"') &&
			       !isspace(*qq))qq++;
			*qq = 0;
			if (href)
			  delete href;
			href = new URL(transSGML(q), *base);
			// I don't know why anyone would do this, but hey...
			if (!nofollow)
			  retriever.got_href(*href, "");
		      }
		  }
	      }

	    //
	    // Now check for <meta name=...  content=...> tags that
	    // fly with any reasonable DTD out there
	    //

	    if (attrs["name"] && attrs["content"])
	    {
		char	*cache = attrs["name"];

		  // First of all, check for META description

		  if (mystrcasecmp(cache, "description") == 0 
			 && strlen(attrs["content"]) != 0)
		  {
		    //
		    // We need to do two things. First grab the description
		    // and clean it up
		    //
		    meta_dsc = transSGML(attrs["content"]);
		    meta_dsc.replace('\n', ' ');
		    meta_dsc.replace('\r', ' ');
		    meta_dsc.replace('\t', ' ');
		    if (meta_dsc.length() > max_meta_description_length)
		     meta_dsc = meta_dsc.sub(0, max_meta_description_length).get();
		   if (debug > 1)
		     cout << "META Description: " << attrs["content"] << endl;
		   retriever.got_meta_dsc(meta_dsc);


		   //
		   // Now add the words to the word list
		   // (slot 11 is the new slot for this)
		   //

		   char        *words = HtWordToken(transSGML(attrs["content"]));
		   char        *w = words;
                   while (w && !noindex)
		     {
			if (strlen(w) >= minimumWordLength)
			  retriever.got_word(w,
				 int((offset+(w-words)) * 1000
					/ totlength),
				 11);
			w = HtWordToken(0);
		     }
		   w = '\0';
		}

		if (keywordsMatch.CompareWord(cache) && !noindex)
		{
		    char	*w = HtWordToken(transSGML(attrs["content"]));
		    while (w)
		    {
			if (strlen(w) >= minimumWordLength
				&& ++keywordsCount <= max_keywords)
			  retriever.got_word(w, 1, 10);
			w = HtWordToken(0);
		    }
		    w = '\0';
		}
		else if (mystrcasecmp(cache, "htdig-email") == 0)
		{
		    retriever.got_meta_email(transSGML(attrs["content"]));
		}
		else if (metadatetags.CompareWord(cache) &&
				config.Boolean("use_doc_date",0))
		{
		    retriever.got_time(transSGML(attrs["content"]));
		}
		else if (mystrcasecmp(cache, "htdig-notification-date") == 0)
		{
		    retriever.got_meta_notification(transSGML(attrs["content"]));
		}
		else if (mystrcasecmp(cache, "htdig-email-subject") == 0)
		{
		    retriever.got_meta_subject(transSGML(attrs["content"]));
		}
		else if (mystrcasecmp(cache, "htdig-noindex") == 0)
		  {
		    retriever.got_noindex();
		    noindex |= TAGmeta_htdig_noindex;
		    nofollow |= TAGmeta_htdig_noindex;
		  }
		else if (mystrcasecmp(cache, "robots") == 0
			 && strlen(attrs["content"]) !=0)
		  {
		    String   content_cache = attrs["content"];
		    content_cache.lowercase();
		    if (content_cache.indexOf("noindex") != -1)
		      {
			noindex |= TAGmeta_robots;
			retriever.got_noindex();
		      }
		    if (content_cache.indexOf("nofollow") != -1)
			nofollow |= TAGmeta_robots;
		    if (content_cache.indexOf("none") != -1)
		      {
			noindex |= TAGmeta_robots;
			nofollow |= TAGmeta_robots;
			retriever.got_noindex();
		      }
		  }
	    }
	    else if (attrs["name"] &&
		     mystrcasecmp(attrs["name"], "htdig-noindex") == 0)
	    {
	        retriever.got_noindex();
		noindex |= TAGmeta_htdig_noindex;
		nofollow |= TAGmeta_htdig_noindex;
	    }
	    break;
	}

	case 21:	// frame
        case 24:	// embed
	{
	  if (attrs["src"])
	    {
	      //
	      // src seen
	      //
	      if (!nofollow)
		{
		  if (href)
		    delete href;
		  href = new URL(transSGML(attrs["src"]), *base);
		  retriever.got_href(*href, transSGML(attrs["title"]));
		  in_ref = 0;
		}
	    }
	  break;
	}
	  
        case 25:	// object
	{
	  if (attrs["data"])
	    {
	      //
	      // data seen
	      //
	      if (!nofollow)
		{
		  if (href)
		    delete href;
		  href = new URL(transSGML(attrs["data"]), *base);
		  retriever.got_href(*href, transSGML(attrs["title"]));
		  in_ref = 0;
		}
	    }
	  break;
	}
	  
	case 22:	// area
        case 26:	// link
	{
	  if (attrs["href"])
	    {
	      // href seen
	      if (!nofollow)
		{
		  if (href)
		    delete href;
		  href = new URL(transSGML(attrs["href"]), *base);
		  retriever.got_href(*href, transSGML(attrs["title"]));
		  in_ref = 0;
		}
	    }
	  break;
	}
	  
	case 23:	// base
	{
	  if (attrs["href"])
	    {
	      //URL tempBase(transSGML(attrs["href"]), *base);
	      URL tempBase(transSGML(attrs["href"]));
	      *base = tempBase;
	    }
	  break;
	}
	
	default:
	    return;						// Nothing...
    }
}


//*****************************************************************************
// char * HTML::transSGML(char *str)
//
char *
HTML::transSGML(char *str)
{
    static String	convert;
    unsigned char	*text = (unsigned char *)str;

    convert = 0;
    while (text && *text)
    {
	if (*text == '&')
	{
	    if (strncmp((char *)text, "&amp;", 5) == 0) 
	    {
		// We MUST convert these in URLs, regardless of translate_amp.
		convert << '&';
		text += 5;
	    } else
		convert << SGMLEntities::translateAndUpdate(text);
	}
	else
	    convert << *text++;
    }
    return convert.get();
}
