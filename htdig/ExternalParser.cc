//
// ExternalParser.cc
//
// ExternalParser: Implementation of ExternalParser
//                 Allows external programs to parse unknown document formats.
//                 The parser is expected to return the document in a 
//                 specific format. The format is documented 
//                 in http://www.htdig.org/attrs.html#external_parser
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ExternalParser.cc,v 1.20 2000/02/19 05:28:51 ghutchis Exp $
//

#include "ExternalParser.h"
#include "HTML.h"
#include "Plaintext.h"
#include "PDF.h"
#include "htdig.h"
#include "htString.h"
#include "QuotedStringList.h"
#include "URL.h"
#include "Dictionary.h"

#include <ctype.h>
#include <stdio.h>
#include "good_strtok.h"

static Dictionary	*parsers = 0;
static Dictionary	*toTypes = 0;
extern String		configFile;

//*****************************************************************************
// ExternalParser::ExternalParser(char *contentType)
//
ExternalParser::ExternalParser(char *contentType)
{
    if (canParse(contentType))
    {
	currentParser = ((String *)parsers->Find(contentType))->get();
    }
    ExternalParser::contentType = contentType;
}


//*****************************************************************************
// ExternalParser::~ExternalParser()
//
ExternalParser::~ExternalParser()
{
}


//*****************************************************************************
// int ExternalParser::readLine(FILE *in, String &line)
//
int
ExternalParser::readLine(FILE *in, String &line)
{
    char	buffer[2048];
    int		length;
    
    line = 0;
    while (fgets(buffer, sizeof(buffer), in))
    {
	length = strlen(buffer);
	if (buffer[length - 1] == '\n')
	{
	    //
	    // A full line has been read.  Return it.
	    //
	    line << buffer;
	    line.chop('\n');
	    return 1;
	}
	else
	{
	    //
	    // Only a partial line was read.  Append it to the line
	    // and read some more.
	    //
	    line << buffer;
	}
    }
    return line.length() > 0;
}


//*****************************************************************************
// int ExternalParser::canParse(char *contentType)
//
int
ExternalParser::canParse(char *contentType)
{
    if (!parsers)
    {
	parsers = new Dictionary();
	toTypes = new Dictionary();
	
	QuotedStringList	qsl(config["external_parsers"], " \t");
	String			from, to;
	int			i;
	int			sep;

	for (i = 0; qsl[i]; i += 2)
	{
	    from = qsl[i];
	    to = "";
	    sep = from.indexOf("->");
	    if (sep != -1)
	    {
		to = from.sub(sep+2).get();
		from = from.sub(0, sep).get();
	    }
	    parsers->Add(from, new String(qsl[i + 1]));
	    toTypes->Add(from, new String(to));
	}
    }
    return parsers->Exists(contentType);
}

//*****************************************************************************
// void ExternalParser::parse(Retriever &retriever, URL &base)
//
void
ExternalParser::parse(Retriever &retriever, URL &base)
{
    if (contents == 0 || contents->length() == 0 ||
	currentParser.length() == 0)
    {
	return;
    }

    //
    // Write the contents to a temporary file.
    //
    String      path = getenv("TMPDIR");
    if (path.length() == 0)
      path = "/tmp";
    path << "/htdext." << getpid();

    FILE	*fl = fopen((char*)path, "w");
    if (!fl)
    {
	return;
    }
    
    fwrite(contents->get(), 1, contents->length(), fl);
    fclose(fl);
    
    //
    // Now start the external parser.
    //
    String	command = currentParser;
    command << ' ' << path << ' ' << contentType << " \"" << base.get() <<
	"\" " << configFile;

    FILE	*input = popen((char*)command, "r");
    if (!input)
    {
	unlink((char*)path);
	return;
    }

    unsigned int minimum_word_length
      = config.Value("minimum_word_length", 3);

    String	line;
    char	*token1, *token2, *token3;
    int		loc = 0, hd = 0;
    URL		url;
    String	convertToType = ((String *)toTypes->Find(contentType))->get();
    int		get_hdr = (convertToType.nocase_compare("user-defined") == 0);
    int		get_file = (convertToType.length() != 0);
    String	newcontent;

    while (readLine(input, line))
    {
	if (get_hdr)
	{
	    line.chop('\r');
	    if (line.length() == 0)
		get_hdr = FALSE;
	    else if (mystrncasecmp((char*)line, "content-type:", 13) == 0)
	    {
		token1 = line.get() + 13;
		while (*token1 && isspace(*token1))
		    token1++;
		token1 = strtok(token1, "\n\t");
		convertToType = token1;
	    }
	    continue;
	}
	if (get_file)
	{
	    if (newcontent.length() == 0 &&
		!canParse(convertToType) &&
		mystrncasecmp((char*)convertToType, "text/", 5) != 0 &&
		mystrncasecmp((char*)convertToType, "application/pdf", 15) != 0)
	    {
		if (mystrcasecmp((char*)convertToType, "user-defined") == 0)
		    cerr << "External parser error: no Content-Type given\n";
		else
		    cerr << "External parser error: can't parse Content-Type \""
			 << convertToType << "\"\n";
		cerr << " URL: " << base.get() << "\n";
		break;
	    }
	    newcontent << line << '\n';
	    continue;
	}
	token1 = strtok(line, "\t");
	if (token1 == NULL)
	    token1 = "";
	token2 = NULL;
	token3 = NULL;
	switch (*token1)
	{
	    case 'w':	// word
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  token2 = strtok(0, "\t");
		if (token2 != NULL)
		  token3 = strtok(0, "\t");
		if (token1 != NULL && token2 != NULL && token3 != NULL &&
			(loc = atoi(token2)) >= 0 &&
			(hd = atoi(token3)) >= 0 && hd < 12)
		  retriever.got_word(token1, loc, hd);
		else
		  cerr<< "External parser error: expected word in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;
		
	    case 'u':	// href
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  token2 = strtok(0, "\t");
		if (token1 != NULL && token2 != NULL)
		{
		  url.parse(token1);
		  url.hopcount(base.hopcount() + 1);
		  retriever.got_href(url, token2);
		}
		else
		  cerr<< "External parser error: expected URL in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;
		
	    case 't':	// title
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  retriever.got_title(token1);
		else
		  cerr<< "External parser error: expected title in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;
		
	    case 'h':	// head
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  retriever.got_head(token1);
		else
		  cerr<< "External parser error: expected text in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;
		
	    case 'a':	// anchor
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  retriever.got_anchor(token1);
		else
		  cerr<< "External parser error: expected anchor in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;
		
	    case 'i':	// image url
		token1 = strtok(0, "\t");
		if (token1 != NULL)
		  retriever.got_image(token1);
		else
		  cerr<< "External parser error: expected image URL in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;

	    case 'm':	// meta
	      {
		// Using good_strtok means we can accept empty
		// fields.
		char *httpEquiv = good_strtok(token1+2, '\t');
		char *name = good_strtok(0, '\t');
		char *content = good_strtok(0, '\t');

		if (httpEquiv != NULL && name != NULL && content != NULL)
		{
		  // It would be preferable if we could share
		  // this part with HTML.cc, but it has other
		  // chores too, and I do not se a point where to
		  // split it up to get a common shared function
		  // (or class).  Which should not stop anybody from
		  // finding a better solution.
		  // For now, there is duplicated code.
		  static StringMatch *keywordsMatch = 0;
		  if (!keywordsMatch)
		  {
			StringList kn(config["keywords_meta_tag_names"], " \t");
			keywordsMatch = new StringMatch();
			keywordsMatch->IgnoreCase();
			keywordsMatch->Pattern(kn.Join('|'));
			kn.Release();
		  }
    
		  // <URL:http://www.w3.org/MarkUp/html-spec/html-spec_5.html#SEC5.2.5> 
		  // says that the "name" attribute defaults to
		  // the http-equiv attribute if empty.
		  if (*name == '\0')
		    name = httpEquiv;

		  if (*httpEquiv != '\0')
		  {
		    // <META HTTP-EQUIV=REFRESH case
		    if (mystrcasecmp(httpEquiv, "refresh") == 0
			&& *content != '\0')
		    {
		      char *q = (char*)mystrcasestr(content, "url=");
		      if (q && *q)
		      {
			q += 4; // skiping "URL="
			char *qq = q;
			while (*qq && (*qq != ';') && (*qq != '"') &&
			       !isspace(*qq))qq++;
			*qq = 0;
			URL href(q, base);
			// I don't know why anyone would do this, but hey...
			retriever.got_href(href, "");
		      }
		    }
		  }

		  //
		  // Now check for <meta name=...  content=...> tags that
		  // fly with any reasonable DTD out there
		  //
		  if (*name != '\0' && *content != '\0')
		  {
		    if (keywordsMatch->CompareWord(name))
		    {
		      char	*w = strtok(content, " ,\t\r");
		      while (w)
		      {
			if (strlen(w) >= minimum_word_length)
			  retriever.got_word(w, 1, 10);
			w = strtok(0, " ,\t\r");
		      }
		    }
		    else if (mystrcasecmp(name, "htdig-email") == 0)
		    {
		      retriever.got_meta_email(content);
		    }
		    else if (mystrcasecmp(name, "htdig-notification-date") == 0)
		    {
		      retriever.got_meta_notification(content);
		    }
		    else if (mystrcasecmp(name, "htdig-email-subject") == 0)
		    {
		      retriever.got_meta_subject(content);
		    }
		    else if (mystrcasecmp(name, "description") == 0 
			     && strlen(content) != 0)
		    {
		      //
		      // We need to do two things. First grab the description
		      //
		      String meta_dsc = content;

		      if (meta_dsc.length() > max_meta_description_length)
			meta_dsc = meta_dsc.sub(0, max_meta_description_length).get();
		      if (debug > 1)
			cout << "META Description: " << content << endl;
		      retriever.got_meta_dsc((char*)meta_dsc);

		      //
		      // Now add the words to the word list
		      // (slot 11 is the new slot for this)
		      //
		      char	  *w = strtok(content, " \t\r");
		      while (w)
		      {
			if (strlen(w) >= minimum_word_length)
			  retriever.got_word(w, 1, 11);
			w = strtok(0, " \t\r");
		      }
		    }
		  }
		}
		else
		  cerr<< "External parser error: expected metadata in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;
	      }

	    default:
		  cerr<< "External parser error: unknown field in line "<<line<<"\n" << " URL: " << base.get() << "\n";
		break;
	}
    }
    pclose(input);
    unlink((char*)path);

    if (newcontent.length() > 0)
    {
	static HTML			*html = 0;
	static Plaintext		*plaintext = 0;
	static PDF			*pdf = 0;
	Parsable			*parsable = 0;

	contentType = convertToType;
	if (canParse(contentType))
	{
	    currentParser = ((String *)parsers->Find(contentType))->get();
	    parsable = this;
	}
	else if (mystrncasecmp((char*)contentType, "text/html", 9) == 0)
	{
	    if (!html)
		html = new HTML();
	    parsable = html;
	}
	else if (mystrncasecmp((char*)contentType, "text/plain", 10) == 0)
	{
	    if (!plaintext)
		plaintext = new Plaintext();
	    parsable = plaintext;
	}
	else if (mystrncasecmp((char*)contentType, "application/pdf", 15) == 0)
	{
	    if (!pdf)
		pdf = new PDF();
	    parsable = pdf;
	}
	else
	{
	    if (!plaintext)
		plaintext = new Plaintext();
	    parsable = plaintext;
	    if (debug)
		cout << "External parser error: \"" << contentType <<
			"\" not a recognized type.  Assuming text\n";
	}
	parsable->setContents(newcontent.get(), newcontent.length());
	parsable->parse(retriever, base);
    }
}


