//
// HTML.cc
//
// Implementation of HTML
// Class to parse HTML documents and return useful information to the Retriever
//
//
#if RELEASE
static char RCSid[] = "$Id: HTML.cc,v 1.30.2.9 1999/09/01 20:25:27 grdetil Exp $";
#endif

#include "htdig.h"
#include "HTML.h"
#include "SGMLEntities.h"
#include "Configuration.h"
#include <ctype.h>
#include "StringMatch.h"
#include "StringList.h"
#include "URL.h"
#include "HtWordType.h"

static StringMatch	tags;
static StringMatch	nobreaktags;
static StringMatch	spacebeforetags;
static StringMatch	spaceaftertags;
static StringMatch	attrs;
static StringMatch	srcMatch;
static StringMatch	hrefMatch;
static StringMatch	keywordsMatch;


//*****************************************************************************
// ADDSPACE() macro, to insert space where needed in various strings
// 		Reduces all multiple whitespace to a single space

#define ADDSPACE(in_space)	\
    if (!in_space)							\
    {									\
	if (in_title && doindex)					\
	{								\
	    title << ' ';						\
	}								\
	if (in_ref && description.length() < max_description_length)	\
	{								\
	    description << ' ';						\
	}								\
	if (head.length() < max_head_length && doindex && !in_title)	\
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
    // the attrs Match object is used to match names of tag parameters.
    //
    tags.IgnoreCase();
    tags.Pattern("title|/title|a|/a|h1|h2|h3|h4|h5|h6|/h1|/h2|/h3|/h4|/h5|/h6|noindex|/noindex|img|li|meta|frame|area|base|embed|object|link");

    attrs.IgnoreCase();
    attrs.Pattern("src|href|name");

    srcMatch.IgnoreCase();
    srcMatch.Pattern("src");

    hrefMatch.IgnoreCase();
    hrefMatch.Pattern("href");

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

    //String	keywordNames = config["keywords_meta_tag_names"];
    //keywordNames.replace(' ', '|');
    //keywordNames.remove(",\t\r\n");
    //keywordsMatch.IgnoreCase();
    //keywordsMatch.Pattern(keywordNames);
    StringList keywordNames(config["keywords_meta_tag_names"], " \t");
    keywordsMatch.IgnoreCase();
    keywordsMatch.Pattern(keywordNames.Join('|'));
    keywordNames.Release();
    
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
    doindex = 1;
    dofollow = 1;
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
    int			offset = 0;
    int			in_space;
    int			in_punct;
    unsigned char	*q, *start;
    unsigned char	*position = (unsigned char *) contents->get();
    unsigned char       *text = (unsigned char *) new char[contents->length()+1];
    unsigned char       *ptext = text;
    static char         *skip_start = config["noindex_start"];
    static char         *skip_end = config["noindex_end"];

    title = 0;
    head = 0;
    meta_dsc = 0;
    doindex = 1;
    dofollow = 1;
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
	  mystrncasecmp((char *)position, skip_start, strlen(skip_start)) == 0)
	{
	  q = (unsigned char*)mystrcasestr((char *)position, skip_end);
	  if (!q)
	    *position = '\0';       // Rest of document will be skipped...
	  else
	    position = q + strlen(skip_end);
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
	      q = (unsigned char*)strstr((char *)position, ">");
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
              *ptext = ' ';
           }
           ptext++;
        }
        else
        {
           *ptext++ = *position++;
        }
      }
      *ptext++ = '\0';

      position = text;
      start = position;

      while (*position)
      {
	offset = position - start;
	// String = 0 is expensive
	// word = 0;
	if (*position == '<')
	  {
	    //
	    // Start of a tag.  Since tags cannot be nested, we can simply
	    // search for the closing '>'
	    //
	    q = (unsigned char*)strchr((char *)position, '>');
	    if (!q)
	      break; // Syntax error in the doc.  Tag never ends.
	    tag = 0;
	    tag.append((char*)position, q - position + 1);
	    position++;
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
			    tag = 0;
			    tag.append((char*)position, q - position + 1);
			    do_tag(retriever, tag);
			    position = q+1;
			}
		    }
		}
	    }

	    if (in_title && doindex)
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
		    if (dofollow)
		      retriever.got_href(*href, description);
		    in_ref = 0;
		    description = 0;
		}
	    }

	    if (head.length() < max_head_length && doindex && !in_title)
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

	    if (word.length() >= minimumWordLength && doindex)
	    {
	      retriever.got_word(word,
				 int(offset * 1000 / contents->length()),
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
		if (head.length() < max_head_length && doindex && !in_title)
		{
		    // We don't want to add random chars to the 
		    // excerpt if we're in the title.
		    head << *position;
		}
		if (in_ref && description.length() < max_description_length)
		{
		    description << *position;
		}
		if (in_title && doindex)
		{
		    title << *position;
		}
		in_space = 0;
		in_punct = 1;
	    }
	    position++;
	}
    }
    retriever.got_head(head);

    delete text;
}


//*****************************************************************************
// void HTML::do_tag(Retriever &retriever, String &tag)
//
void
HTML::do_tag(Retriever &retriever, String &tag)
{
    char	*position = tag.get() + 1;		// Skip the '<'
    char	*q, *t;
    int		which, length;

    while (isspace(*position))
	position++;

    which = -1;
    if (tags.CompareWord(position, which, length) < 0)
	return; // Nothing matched.

    if (debug > 3)
	cout << "Tag: " << position << ", matched " << which << endl;
    
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
	    which = -1;
	    int pos;
	    while ((pos = attrs.FindFirstWord(position, which, length)) >= 0)
	    {
		position += pos + length;
		if (debug > 1)
		    cout << "A tag: pos = " << pos << ", position = " << position << endl;
		switch (which)
		{
		    case 1:		// "href"
		    {
			//
			// a href seen
			//
			while (*position && *position != '=')
			    position++;
			if (!*position)
			    return;
			position++;
			while (isspace(*position))
			    position++;
                       //
		       // Allow either single quotes or double quotes
                       // around the URL itself
                       //
                       if (*position == '"'||*position == '\'')
			{
			    position++;
			    q = strchr(position, position[-1]);
			    if (!q)
				break;
                           //
                           // We seem to have matched the opening quote char
                           // Mark the end of the quotes as our endpoint, so
                           // that we can continue parsing after the current
                           // text
                           //
                           *q = '\0';
                           //
                           // If a '#' is present in a quoted URL,
                           //  treat that as the end of the URL, but we skip
                           //  past the quote to parse the rest of the anchor.
                           //
                           if ((t = strchr(position, '#')) != NULL)
                               *t = '\0';
			}
			else
			{
			    q = position;
			    while (*q &&
				   *q != '>' &&
				   !isspace(*q) && // *q != '?'  ???? -grh
				   *q != '#')
				q++;
			    *q = '\0';
			}
			if (in_ref)
			{
			    if (debug > 1)
				cout << "Terminating previous <a href=...> tag,"
				     << " which didn't have a closing </a> tag."
				     << endl;
			    if (dofollow)
				retriever.got_href(*href, description);
			    in_ref = 0;
			}
			delete href;
			href = new URL(position, *base);
			in_ref = 1;
			description = 0;
			position = q + 1;
			break;
		    }

		    case 2:		// "name"
		    {
			//
			// a name seen
			//
			while (*position && *position != '=')
			    position++;
			if (!*position)
			    return;
			position++;
			while (isspace(*position))
			    position++;
                       //
                       // Allow either single quotes or double quotes
                       // around the URL itself
                       //
                       if (*position == '"'||*position == '\'')
			{
			    position++;
			    q = strchr(position, position[-1]);
			    if (!q)
				break;
                           //
                           // We seem to have matched the opening quote char
                           // Mark the end of the quotes as our endpoint, so
                           // that we can continue parsing after the current
                           // text
                           //
                           *q = '\0';
                           //
                           // If a '#' is present in a quoted URL,
                           //  treat that as the end of the URL, but we skip
                           //  past the quote to parse the rest of the anchor.
                           //
                           if ((t = strchr(position, '#')) != NULL)
                               *t = '\0';
			}
			else
			{
			    q = position;
			    while (*q && *q != '>' && !isspace(*q))
				q++;
			*q = '\0';
			}
			retriever.got_anchor(position);
			position = q + 1;
			break;
		    }
		    default:
			break;
		}
	    }
	    break;
	}

	case 3:		// "/a"
	    if (in_ref)
	    {
	      if (dofollow)
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
	    doindex = 0;
	    dofollow = 0;
	    break;

	case 17:	// "/noindex"
	    doindex = 1;
	    dofollow = 1;
	    break;

	case 18:	// "img"
	{
	    which = -1;
	    int pos = attrs.FindFirstWord(position, which, length);
	    if (pos < 0 || which != 0)
		break;
	    position += pos + length;
	    while (*position && *position != '=')
		position++;
	    if (!*position)
		break;
	    position++;
	    while (isspace(*position))
		position++;
           //
           // Allow either single quotes or double quotes
           // around the URL itself
           //
           if (*position == '"'||*position == '\'')
	    {
		position++;
		q = strchr(position, position[-1]);
		if (!q)
		    break;
               //
               // We seem to have matched the opening quote char
               // Mark the end of the quotes as our endpoint, so
               // that we can continue parsing after the current
               // text
               //
               *q = '\0';
               //
               // If a '#' is present in a quoted URL,
               //  treat that as the end of the URL, but we skip
               //  past the quote to parse the rest of the anchor.
               //
               if ((t = strchr(position, '#')) != NULL)
                   *t = '\0';
	    }
	    else
	    {
		q = position;
		while (*q && *q != '>' && !isspace(*q))
		    q++;
	    *q = '\0';
	    }
	    retriever.got_image(position);
	    break;
	}

	case 19:	// "li"
	    if (doindex && !in_title && head.length() < max_head_length)
		head << "* ";
	    break;

	case 20:	// "meta"
	{	    position += length;
	    Configuration	conf;
	    conf.NameValueSeparators("=");
	    conf.Add(position);

	    //
	    // First test for old-style meta tags (these break any
	    // reasonable DTD...)
	    //
	    if (conf["htdig-noindex"])
	      {
		retriever.got_noindex();
		doindex = 0;
		dofollow = 0;
	      }
	    if (conf["htdig-index"])
	      {
		doindex = 1;
		dofollow = 1;
	      }
	    if (conf["htdig-email"])
	    {
		retriever.got_meta_email(conf["htdig-email"]);
	    }
	    if (conf["htdig-notification-date"])
	    {
		retriever.got_meta_notification(conf["htdig-notification-date"]);
	    }
	    if (conf["htdig-email-subject"])
	    {
		retriever.got_meta_subject(conf["htdig-email-subject"]);
	    }
	    if (conf["htdig-keywords"] || conf["keywords"])
	    {
		//
		// Keywords are added as being at the very top of the
		// document and have a weight factor of
		// keywords-factor which is assigned to slot 10 in the
		// factor table.
		//
		char	*keywords = conf["htdig-keywords"];
		if (!keywords)
		    keywords = conf["keywords"];
		char	*w = strtok(keywords, " ,\t\r\n");
		while (w)
		{
		    if (strlen(w) >= minimumWordLength)
		      retriever.got_word(w, 1, 10);
		    w = strtok(0, " ,\t\r\n");
		}
		w = '\0';
	    }
	
	    if (conf["http-equiv"])
	      {

		// <META HTTP-EQUIV=REFRESH case
		if (mystrcasecmp(conf["http-equiv"], "refresh") == 0
		    && conf["content"])
		  {
		    char *content = conf["content"];
		    char *q = mystrcasestr(content, "url=");
		    if (q && *q)
		      {
			q += 4; // skiping "URL="
			char *qq = q;
			while (*qq && (*qq != ';') && (*qq != '"') &&
			       !isspace(*qq))qq++;
			*qq = 0;
			URL *href = new URL(q, *base);
			// I don't know why anyone would do this, but hey...
			if (dofollow)
			  retriever.got_href(*href, "");
			delete href;
		      }
		  }
	      }

	    //
	    // Now check for <meta name=...  content=...> tags that
	    // fly with any reasonable DTD out there
	    //

	    if (conf["name"] && conf["content"])
	    {
		char	*cache = conf["name"];

		which = -1; // What does it do?

		  // First of all, check for META description

		  if (mystrcasecmp(cache, "description") == 0 
			 && strlen(conf["content"]) != 0)
		  {
		    //
		    // We need to do two things. First grab the description
		    //
		    meta_dsc = conf["content"];
		   if (meta_dsc.length() > max_meta_description_length)
		     meta_dsc = meta_dsc.sub(0, max_meta_description_length).get();
		   if (debug > 1)
		     cout << "META Description: " << conf["content"] << endl;
		   retriever.got_meta_dsc(meta_dsc);


		   //
		   // Now add the words to the word list
		   // (slot 11 is the new slot for this)
		   //

		   char        *w = strtok(conf["content"], " \t\r\n");
                   while (w)
		     {
			if (strlen(w) >= minimumWordLength)
			  retriever.got_word(w, 1, 11);
			w = strtok(0, " \t\r\n");
		     }
		 w = '\0';
		}

		if (keywordsMatch.CompareWord(cache))
		{
		    char	*w = strtok(conf["content"], " ,\t\r\n");
		    while (w)
		    {
			if (strlen(w) >= minimumWordLength)
			  retriever.got_word(w, 1, 10);
			w = strtok(0, " ,\t\r\n");
		    }
		    w = '\0';
		}
		else if (mystrcasecmp(cache, "htdig-email") == 0)
		{
		    retriever.got_meta_email(conf["content"]);
		}
		else if (mystrcasecmp(cache, "htdig-notification-date") == 0)
		{
		    retriever.got_meta_notification(conf["content"]);
		}
		else if (mystrcasecmp(cache, "htdig-email-subject") == 0)
		{
		    retriever.got_meta_subject(conf["content"]);
		}
		else if (mystrcasecmp(cache, "htdig-noindex") == 0)
		  {
		    retriever.got_noindex();
		    doindex = 0;
		    dofollow = 0;
		  }
		else if (mystrcasecmp(cache, "robots") == 0
			 && strlen(conf["content"]) !=0)
		  {
		    String   content_cache = conf["content"];

		    if (content_cache.indexOf("noindex") != -1)
		      {
			doindex = 0;
			retriever.got_noindex();
		      }
		    if (content_cache.indexOf("nofollow") != -1)
		      dofollow = 0;
		    if (content_cache.indexOf("none") != -1)
		      {
			doindex = 0;
			dofollow = 0;
			retriever.got_noindex();
		      }
		  }
	    }
	    else if (conf["name"] &&
		     mystrcasecmp(conf["name"], "htdig-noindex") == 0)
	    {
	        retriever.got_noindex();
	        doindex = 0;
		dofollow = 0;
	    }
	    break;
	}

	case 21:	// frame
	case 24:	// embed
	case 25:	// object
	{
	    which = -1;
	    int pos = srcMatch.FindFirstWord(position, which, length);
	    position += pos + length;
	    switch (which)
	    {
		case 0:		// "src"
		{
		    //
		    // src seen
		    //
		    while (*position && *position != '=')
			position++;
		    if (!*position)
			return;
		    position++;
		    while (isspace(*position))
			position++;
                   //
                   // Allow either single quotes or double quotes
                   // around the URL itself
                   //
                   if (*position == '"'||*position == '\'')
		    {
			position++;
			q = strchr(position, position[-1]);
			if (!q)
			    break;
                       //
                       // We seem to have matched the opening quote char
                       // Mark the end of the quotes as our endpoint, so
                       // that we can continue parsing after the current
                       // text
                       //
                       *q = '\0';
                       //
                       // If a '#' is present in a quoted URL,
                       //  treat that as the end of the URL, but we skip
                       //  past the quote to parse the rest of the anchor.
                       //
                       if ((t = strchr(position, '#')) != NULL)
                           *t = '\0';
		    }
		    else
		    {
			q = position;
			while (*q &&
			       *q != '>' &&
			       !isspace(*q) && //  *q != '?'   ??? -grh
			       *q != '#')
			    q++;
			*q = '\0';
		    }
		    delete href;
		    href = new URL(position, *base);
		    if (dofollow)
		    {
			description = 0;
			retriever.got_href(*href, description);
			in_ref = 0;
		    }
		    break;
		}
		break;
	    }
	    break;
	}
	
	case 22:	// area
	case 26:	// link
	{
	    which = -1;
	    int pos = hrefMatch.FindFirstWord(position, which, length);
	    position += pos + length;
	    switch (which)
	    {
		case 0:		// "href"
		{
		    //
		    // href seen
		    //
		    while (*position && *position != '=')
			position++;
		    if (!*position)
			return;
		    position++;
		    while (isspace(*position))
			position++;
                   //
                   // Allow either single quotes or double quotes
                   // around the URL itself
                   //
                   if (*position == '"'||*position == '\'')
		    {
			position++;
			q = strchr(position, position[-1]);
			if (!q)
			    break;
                       //
                       // We seem to have matched the opening quote char
                       // Mark the end of the quotes as our endpoint, so
                       // that we can continue parsing after the current
                       // text
                       //
                       *q = '\0';
                       //
                       // If a '#' is present in a quoted URL,
                       //  treat that as the end of the URL, but we skip
                       //  past the quote to parse the rest of the anchor.
                       if ((t = strchr(position, '#')) != NULL)
                           *t = '\0';
		    }
		    else
		    {
			q = position;
			while (*q &&
			       *q != '>' &&
			       !isspace(*q) && //  *q != '?'   ???? --grh
			       *q != '#')
			    q++;
			*q = '\0';
		    }
		    delete href;
		    href = new URL(position, *base);
		    if (dofollow)
		    {
			description = 0;
			retriever.got_href(*href, description);
			in_ref = 0;
		    }
		    break;
		}

		default:
		    break;
	    }
	    break;
	}

	case 23:	// base
	{
	    which = -1;
	    int pos = hrefMatch.FindFirstWord(position, which, length);
	    position += pos + length;
	    switch (which)
	    {
		case 0:		// "href"
		{
		    while (*position && *position != '=')
			position++;
		    if (!*position)
			return;
		    position++;
		    while (isspace(*position))
			position++;
                   //
                   // Allow either single quotes or double quotes
                   // around the URL itself
                   //
                   if (*position == '"'||*position == '\'')
		    {
			position++;
			q = strchr(position, position[-1]);
			if (!q)
			    break;
                       //
                       // We seem to have matched the opening quote char
                       // Mark the end of the quotes as our endpoint, so
                       // that we can continue parsing after the current
                       // text
                       //
                       *q = '\0';
                       //
                       // If a '#' is present in a quoted URL,
                       //  treat that as the end of the URL, but we skip
                       //  past the quote to parse the rest of the anchor.
                       //
                       // Is there a better way of looking for these?
                       //
                       if ((t = strchr(position, '#')) != NULL)
                           *t = '\0';
		    }
		    else
		    {
			q = position;
			while (*q &&
			       *q != '>' &&
			       !isspace(*q) && // *q != '?'   ??? -grh
			       *q != '#')
			    q++;
		    *q = '\0';
		    }
		    URL tempBase(position, *base);
		    *base = tempBase;
		}
	    }
	    break;
	}
	
	default:
	    return;						// Nothing...
    }
}
