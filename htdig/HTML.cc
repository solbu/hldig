//
// HTML.cc
//
// Implementation of HTML
//
// $Log: HTML.cc,v $
// Revision 1.1  1997/02/03 17:11:06  turtle
// Initial revision
//
//
#if RELEASE
static char RCSid[] = "$Id: HTML.cc,v 1.1 1997/02/03 17:11:06 turtle Exp $";
#endif

#include "htdig.h"
#include "HTML.h"
#include "SGMLEntities.h"
#include <Configuration.h>
#include <ctype.h>
#include <StringMatch.h>
#include <URL.h>

static StringMatch	tags;
static StringMatch	attrs;
static StringMatch	srcMatch;
static StringMatch	hrefMatch;
static StringMatch	keywordsMatch;


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
    tags.Pattern("title|/title|a|/a|h1|h2|h3|h4|h5|h6|/h1|/h2|/h3|/h4|/h5|/h6|noindex|/noindex|img|li|meta|frame|area|base");

    attrs.IgnoreCase();
    attrs.Pattern("src|href|name");

    srcMatch.IgnoreCase();
    srcMatch.Pattern("src");

    hrefMatch.IgnoreCase();
    hrefMatch.Pattern("href");

    String	keywordNames = config["keywords_meta_tag_names"];
    keywordNames.replace(' ', '|');
    keywordNames.remove(",\t\r\n");
    keywordsMatch.IgnoreCase();
    keywordsMatch.Pattern(keywordNames);
    
    word = 0;
    href = 0;
    title = 0;
    description = 0;
    head = 0;
    tag = 0;
    in_title = 0;
    in_ref = 0;
    in_heading = 0;
    base = 0;
    doindex = 1;
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
    unsigned char	*q, *start;
    unsigned char	*position = (unsigned char *) contents->get();

    start = position;
    title = 0;
    head = 0;
    doindex = 1;
    in_heading = 0;
    in_title = 0;
    in_ref = 0;
	
    while (*position)
    {
	offset = position - start;

	if (strncmp((char *)position, "<!--", 4) == 0)
	{
	    //
	    // Stupid comment.  This can contain other '<' and '>'
	    // stuff which we have to ignore
	    //
	    q = (unsigned char*)strstr((char *)position, "-->");
	    if (!q)
		return;			// Rest of document is a comment...
	    position = q + 3;
	    continue;
	}
	if (*position == '<')
	{
	    //
	    // Start of a tag.  Since tags cannot be nested, we can simply
	    // search for the closing '>'
	    //
	    q = (unsigned char*)strchr((char *)position, '>');
	    if (!q)
		return; // Syntax error in the doc.  Tag never ends.
	    tag = 0;
	    tag.append((char*)position, q - position + 1);
	    do_tag(retriever, tag);
	    position = q;
	}
#if 0
	else if (*position == '&')
	{
	    //
	    // HTML uses "&<string>;" as a way to put special characters in a
	    // document.  We'll just skip these...
	    //
	    unsigned char	*orig = position++;
	    if (!isalnum(*position))
	    {
		//
		// This is an illegal escape.  We need to assume the
		// author just wants to use a '&'.
		//
		position = orig;
	    }
	    else
	    {
		while (isalnum(*position))
		    position++;
		if (*position != ';')
		{
		    //
		    // Broken escape.  Didn't end with a ';'.  Assume literal
		    //
		    position = orig;
		}
		else
		{
		    position++;
		    continue;
		}
	    }
	}
#endif
	
	word = 0;
	if (*position > 0 && (isalnum(*position) || *position >= 160 ||
			      *position == '&'))
	{
	    //
	    // Start of a word.  Try to find the whole thing
	    //
	    in_space = 0;
	    while (*position &&
		   (isalnum(*position) ||
		    strchr(valid_punctuation, *position) ||
		    *position >= 160 ||
		    *position == '&'))
	    {
		if (*position == '&')
		{
		    unsigned char	ch;
		    ch = SGMLEntities::translateAndUpdate(position);
		    word << (char) ch;
		}
		else
		{
		    word << (char)*position;
		    position++;
		}
	    }

	    if (in_title && doindex)
	    {
		title << word;
	    }

	    if (in_ref)
	    {
		description << word;
		if (description.length() > max_description_length)
		{
		    description << " ...";
		    retriever.got_href(*href, description);
		    in_ref = 0;
		    description = 0;
		}
	    }

	    if (head.length() < max_head_length && doindex && !in_title)
	    {
		//
		// Capitalize H1 and H2 blocks
		//
		if (in_heading > 1 && in_heading < 4)
		{
		    word.uppercase();
		}

		//
		// Append the word to the head (excerpt)
		//
		head << word;
	    }

	    if (word.length() >= minimumWordLength && doindex)
	    {
		word.lowercase();
		word.remove(valid_punctuation);
		if (word.length() >= minimumWordLength)
		{
		    retriever.got_word(word,
				       int(offset * 1000 / contents->length()),
				       in_heading);
		}
	    }
	}
	else
	{
	    //
	    // Characters that are not part of a word
	    //
	    if (*position != '>' && doindex)
	    {
		if (isspace(*position))
		{
		    //
		    // Reduce all multiple whitespace to a single space
		    //
		    if (!in_space)
		    {
			if (head.length() < max_head_length)
			{
			    head << ' ';
			}
			if (in_ref)
			{
			    description << ' ';
			}
			if (in_title)
			{
			    title << ' ';
			}
		    }
		    in_space = 1;
		}
		else
		{
		    //
		    // Not whitespace
		    //
		    if (head.length() < max_head_length)
		    {
			head << *position;
		    }
		    if (in_ref)
		    {
			description << *position;
		    }
		    if (in_title)
		    {
			title << *position;
		    }
		    in_space = 0;
		}
	    }
	    position++;
	}
    }
    retriever.got_head(head);
}


//*****************************************************************************
// void HTML::do_tag(Retriever &retriever, String &tag)
//
void
HTML::do_tag(Retriever &retriever, String &tag)
{
    char	*position = tag.get() + 1;		// Skip the '<'
    char	*q;
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
	    in_title = 1;
	    in_heading = 1;
	    break;
			
	case 1:		// "/title"
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
			if (*position == '"')
			{
			    position++;
			    q = strchr(position, '"');
			    if (!q)
				break;
			}
			else
			{
			    q = position;
			    while (*q &&
				   *q != '>' &&
				   !isspace(*q) &&
				   *q != '?' &&
				   *q != '#')
				q++;
			}
			*q = '\0';
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
			if (*position == '"')
			{
			    position++;
			    q = strchr(position, '"');
			    if (!q)
				break;
			}
			else
			{
			    q = position;
			    while (*q && *q != '>' && !isspace(*q))
				q++;
			}
			*q = '\0';
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
	    if (doindex && in_ref)
	    {
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
	    break;

	case 17:	// "/noindex"
	    doindex = 1;
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
	    if (*position == '"')
	    {
		position++;
		q = strchr(position, '"');
		if (!q)
		    break;
	    }
	    else
	    {
		q = position;
		while (*q && *q != '>' && !isspace(*q))
		    q++;
	    }
	    *q = '\0';
	    retriever.got_image(position);
	    break;
	}

	case 19:	// "li"
	    if (doindex && head.length() < max_head_length)
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
		doindex = 0;
	    if (conf["htdig-index"])
		doindex = 1;
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
		char	*w = strtok(keywords, " \t\r\n");
		while (w)
		{
		    if (strlen(w) >= minimumWordLength)
			retriever.got_word(w, 1, 10);
		    w = strtok(0, " \t\r\n");
		}
	    }

	    //
	    // Now check for <meta name=...  content=...> tags that
	    // fly with any reasonable DTD out there
	    //
	    if (conf["name"] && conf["content"])
	    {
		char	*cache = conf["name"];

		which = -1;
		if (keywordsMatch.CompareWord(cache))
		{
		    char	*w = strtok(conf["content"], " \t\r\n");
		    while (w)
		    {
			if (strlen(w) >= minimumWordLength)
			    retriever.got_word(w, 1, 10);
			w = strtok(0, " \t\r\n");
		    }
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
		    doindex = 0;
		}
	    }
	    else if (conf["name"] &&
		     mystrcasecmp(conf["name"], "htdig-noindex") == 0)
	    {
		doindex = 0;
	    }
	    break;
	}

	case 21:	// frame
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
		    if (*position == '"')
		    {
			position++;
			q = strchr(position, '"');
			if (!q)
			    break;
		    }
		    else
		    {
			q = position;
			while (*q &&
			       *q != '>' &&
			       !isspace(*q) &&
			       *q != '?' &&
			       *q != '#')
			    q++;
		    }
		    *q = '\0';
		    delete href;
		    href = new URL(position, *base);
		    if (doindex)
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
	{
	    which = -1;
	    int pos = hrefMatch.FindFirstWord(position, which, length);
	    position += pos + length;
	    switch (which)
	    {
		case 0:		// "href"
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
		    if (*position == '"')
		    {
			position++;
			q = strchr(position, '"');
			if (!q)
			    break;
		    }
		    else
		    {
			q = position;
			while (*q &&
			       *q != '>' &&
			       !isspace(*q) &&
			       *q != '?' &&
			       *q != '#')
			    q++;
		    }
		    *q = '\0';
		    delete href;
		    href = new URL(position, *base);
		    if (doindex)
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
		    if (*position == '"')
		    {
			position++;
			q = strchr(position, '"');
			if (!q)
			    break;
		    }
		    else
		    {
			q = position;
			while (*q &&
			       *q != '>' &&
			       !isspace(*q) &&
			       *q != '?' &&
			       *q != '#')
			    q++;
		    }
		    *q = '\0';
		    URL tempBase(position, *base);
		    *base = tempBase;
		}
	    }
	}
	
	default:
	    return;						// Nothing...
    }
}
