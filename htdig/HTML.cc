//
// HTML.cc
//
// Implementation of HTML
// Class to parse HTML documents and return useful information to the Retriever
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
//
#if RELEASE
static char RCSid[] = "$Id: HTML.cc,v 1.52 1999/08/23 15:18:48 grdetil Exp $";
#endif

#include "htdig.h"
#include "HTML.h"
#include "HtSGMLCodec.h"
#include "Configuration.h"
#include "StringMatch.h"
#include "StringList.h"
#include "URL.h"
#include "HtWordType.h"
#include <ctype.h>


static StringMatch	tags;
static StringMatch	nobreaktags;
static StringMatch	spacebeforetags;
static StringMatch	spaceaftertags;
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
    String		scratch, textified;
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
	    q = (unsigned char*)strchr((char *)position, ';');
	    if (q && q <= position+10)
	      {	// got ending, looks like valid SGML entity
		scratch = 0;
		scratch.append((char*)position, q+1 - position);
		textified = HtSGMLCodec::instance()->encode(scratch);
		if (textified[0] != '&')	// it was decoded, copy it
		  {
		    position = (unsigned char *)textified.get();
		    while (*position)
		      {
			if (*position == '<')
			  { // got a decoded &lt;, make a fake tag for it
			    // to avoid confusing it with real tag start
			    *ptext++ = '<';
			    *ptext++ = '~';
			    *ptext++ = '>';
			    position++;
			  }
			else
			    *ptext++ = *position++;
		      }
		    position = q+1;
		  }
		else	// it wasn't decoded, copy '&', and rest will follow
		    *ptext++ = *position++;
	      }
	    else	// not SGML entity, copy bare '&'
		*ptext++ = *position++;
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
		    position += 2;	// skip over fake tag for decoded '<'
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
		// handle normal case where decoded '<' is punctuation...
		if (strncmp((char *)position, "<~>", 3) == 0)
		    position += 2;	// skip over fake tag for decoded '<'
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
    char		*position = tag.get();
    int			which, length;

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
	cout << "Tag: " << tag << ", matched " << which << endl;
    
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
		  if (dofollow)
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

	case 19:	// "li"
	    if (doindex && !in_title && head.length() < max_head_length)
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
		doindex = 0;
		dofollow = 0;
	      }
	    if (attrs["htdig-index"])
	      {
		doindex = 1;
		dofollow = 1;
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
		char	*w = strtok(transSGML(keywords), " ,\t\r\n");
		while (w)
		{
		    if (strlen(w) >= minimumWordLength)
		      retriever.got_word(w, 1, 9);
		    w = strtok(0, " ,\t\r\n");
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
		    char *q = mystrcasestr(content, "url=");
		    if (q && *q)
		      {
			q += 4; // skiping "URL="
			char *qq = q;
			while (*qq && (*qq != ';') && (*qq != '"') &&
			       !isspace(*qq))qq++;
			*qq = 0;
			if (href)
			  delete href;
			href = new URL(transSGML(q), *base);
			// I don't know why anyone would do this, but hey...
			if (dofollow)
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

		which = -1; // What does it do?

		  // First of all, check for META description

		  if (mystrcasecmp(cache, "description") == 0 
			 && strlen(attrs["content"]) != 0)
		  {
		    //
		    // We need to do two things. First grab the description
		    //
		    meta_dsc = transSGML(attrs["content"]);
		   if (meta_dsc.length() > max_meta_description_length)
		     meta_dsc = meta_dsc.sub(0, max_meta_description_length).get();
		   if (debug > 1)
		     cout << "META Description: " << attrs["content"] << endl;
		   retriever.got_meta_dsc(meta_dsc);


		   //
		   // Now add the words to the word list
		   // Slot 10 is the current slot for this
		   //

		   char        *w = strtok(transSGML(attrs["content"]), " \t\r\n");
                   while (w)
		     {
			if (strlen(w) >= minimumWordLength)
			  retriever.got_word(w, 1,10);
			w = strtok(0, " \t\r\n");
		     }
		 w = '\0';
		}

		if (keywordsMatch.CompareWord(cache))
		{
		    char	*w = strtok(transSGML(attrs["content"]), " ,\t\r\n");
		    while (w)
		    {
			if (strlen(w) >= minimumWordLength)
			  retriever.got_word(w, 1, 9);
			w = strtok(0, " ,\t\r\n");
		    }
		    w = '\0';
		}
		else if (mystrcasecmp(cache, "htdig-email") == 0)
		{
		    retriever.got_meta_email(transSGML(attrs["content"]));
		}
		else if (mystrcasecmp(cache, "date") == 0 && 
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
		    doindex = 0;
		    dofollow = 0;
		  }
		else if (mystrcasecmp(cache, "robots") == 0
			 && strlen(attrs["content"]) !=0)
		  {
		    String   content_cache = attrs["content"];

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
	    else if (attrs["name"] &&
		     mystrcasecmp(attrs["name"], "htdig-noindex") == 0)
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
	  if (attrs["src"])
	    {
	      //
	      // src seen
	      //
	      if (dofollow)
		{
		  if (href)
		    delete href;
		  href = new URL(transSGML(attrs["src"]), *base);
		  // Frames have the same hopcount as the parent.
		  retriever.got_href(*href, 0, 0);
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
	      if (dofollow)
		{
		  if (href)
		    delete href;
		  href = new URL(transSGML(attrs["href"]), *base);
		  // area & link are like anchor tags -- one hopcount!
		  retriever.got_href(*href, "", 1);
		  in_ref = 0;
		}
	    }
	  break;
	}
	  
	case 23:	// base
	{
	  if (attrs["href"])
	    {
	      URL tempBase(transSGML(attrs["href"]));
	      *base = tempBase;
	    }
	  break;
	}
	
         case 18: // img
	   {
	     if (attrs["src"])
	       {
		 retriever.got_image(transSGML(attrs["src"]));
		 if (attrs["alt"])
		   {
		     char *w = strtok(transSGML(attrs["alt"]), " ,\t\r\n");
		     while (w)
		       {
			 if (strlen(w) >= minimumWordLength)
			   retriever.got_word(w, 1, 8); // slot for img_alt
			 w = strtok(0, " ,\t\r\n");
		       }
		     w = '\0';
		   }
	       }
	     break;
	   }

         default:
	   return;	// Nothing...
    }
}


//*****************************************************************************
// char * HTML::transSGML(char *text)
//
char *
HTML::transSGML(char *str)
{
    static String	convert;
    String		scratch = str;
    convert = HtSGMLCodec::instance()->encode(scratch);
    return convert.get();
}
