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
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ExternalParser.cc,v 1.19.2.19 2000/12/12 19:35:42 grdetil Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "ExternalParser.h"
#include "HTML.h"
#include "Plaintext.h"
#include "htdig.h"
#include "htString.h"
#include "QuotedStringList.h"
#include "URL.h"
#include "Dictionary.h"
#include "good_strtok.h"

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

static Dictionary	*parsers = 0;
static Dictionary	*toTypes = 0;
extern String		configFile;

//*****************************************************************************
// ExternalParser::ExternalParser(char *contentType)
//
ExternalParser::ExternalParser(char *contentType)
{
  String mime;
  int sep;

    if (canParse(contentType))
    {
        String mime = contentType;
	mime.lowercase();
	sep = mime.indexOf(';');
	if (sep != -1)
	  mime = mime.sub(0, sep).get();
	
	currentParser = ((String *)parsers->Find(mime))->get();
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
    
    line = 0; // read(in, buffer, sizeof(buffer)
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
  int			sep;

    if (!parsers)
    {
	parsers = new Dictionary();
	toTypes = new Dictionary();
	
	QuotedStringList	qsl(config["external_parsers"], " \t");
	String			from, to;
	int			i;

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
	    from.lowercase();
	    sep = from.indexOf(';');
	    if (sep != -1)
	      from = from.sub(0, sep).get();

	    parsers->Add(from, new String(qsl[i + 1]));
	    toTypes->Add(from, new String(to));
	}
    }

    String mime = contentType;
    mime.lowercase();
    sep = mime.indexOf(';');
    if (sep != -1)
      mime = mime.sub(0, sep).get();
    return parsers->Exists(mime);
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
    int		fd;
    if (path.length() == 0)
      path = "/tmp";
#ifndef HAVE_MKSTEMP
    path << "/htdext." << getpid(); // This is unfortunately predictable

#ifdef O_BINARY
    fd = open((char*)path, O_WRONLY|O_CREAT|O_EXCL|O_BINARY);
#else
    fd = open((char*)path, O_WRONLY|O_CREAT|O_EXCL);
#endif
#else
    path << "/htdex.XXXXXX";
    fd = mkstemp((char*)path);
    // can we force binary mode somehow under Cygwin, if it has mkstemp?
#endif
    if (fd < 0)
    {
      if (debug)
	cout << "External parser error: Can't create temp file "
	     << (char *)path << endl;
      return;
    }
    
    write(fd, contents->get(), contents->length());
    close(fd);

    unsigned int minimum_word_length = config.Value("minimum_word_length", 3);
    String	line;
    char	*token1, *token2, *token3;
    int		loc = 0, hd = 0;
    URL		url;
    String mime = contentType;
    mime.lowercase();
    int	sep = mime.indexOf(';');
    if (sep != -1)
      mime = mime.sub(0, sep).get();
    String	convertToType = ((String *)toTypes->Find(mime))->get();
    int		get_hdr = (convertToType.nocase_compare("user-defined") == 0);
    int		get_file = (convertToType.length() != 0);
    String	newcontent;

    StringList	cpargs(currentParser);
    const char   **parsargs = new const char * [cpargs.Count() + 5];
    int    argi;
    for (argi = 0; argi < cpargs.Count(); argi++)
	parsargs[argi] = cpargs[argi].get();
    parsargs[argi++] = path.get();
    parsargs[argi++] = contentType.get();
    parsargs[argi++] = base.get().get();
    parsargs[argi++] = configFile.get();
    parsargs[argi++] = 0;

    int    stdout_pipe[2];
    int	   fork_result = -1;
    int	   fork_try;

    if (pipe(stdout_pipe) == -1)
    {
      if (debug)
	cout << "External parser error: Can't create pipe!" << endl;
      unlink((char*)path);
      return;
    }

    for (fork_try = 4; --fork_try >= 0;)
    {
      fork_result = fork(); // Fork so we can execute in the child process
      if (fork_result != -1)
	break;
      if (fork_try)
	sleep(3);
    }
    if (fork_result == -1)
    {
      if (debug)
	cout << "Fork Failure in ExternalParser" << endl;
      unlink((char*)path);
      return;
    }

    if (fork_result == 0) // Child process
    {
	close(STDOUT_FILENO); // Close handle STDOUT to replace with pipe
	dup(stdout_pipe[1]);
	close(stdout_pipe[0]);
	close(stdout_pipe[1]);
	close(STDIN_FILENO); // Close STDIN to replace with file
	open((char*)path, O_RDONLY);

	// Call External Parser
	execv(currentParser.get(), parsargs);

	exit(EXIT_FAILURE);
    }

    // Parent Process
    delete [] parsargs;
    close(stdout_pipe[1]); // Close STDOUT for writing
#ifdef O_BINARY
    FILE *input = fdopen(stdout_pipe[0], "rb");
#else
    FILE *input = fdopen(stdout_pipe[0], "r");
#endif
    if (input == NULL)
    {
      if (debug)
	cout << "Fdopen Failure in ExternalParser" << endl;
      unlink((char*)path);
      return;
    }

    while ((!get_file || get_hdr) && readLine(input, line))
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
#ifdef O_BINARY
	line.chop('\r');
#endif
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
    } // while(readLine)
    if (get_file)
    {
	if (!canParse(convertToType) &&
	    mystrncasecmp((char*)convertToType, "text/", 5) != 0)
	{
	    if (mystrcasecmp((char*)convertToType, "user-defined") == 0)
		cerr << "External parser error: no Content-Type given\n";
	    else
		cerr << "External parser error: can't parse Content-Type \""
		     << convertToType << "\"\n";
	    cerr << " URL: " << base.get() << "\n";
	}
	else
	{
	    char	buffer[2048];
	    int		length;
	    while ((length = fread(buffer, 1, sizeof(buffer), input)) > 0)
		newcontent.append(buffer, length);
	}
    }
    fclose(input);
    // close(stdout_pipe[0]); // This is closed for us by the fclose()
    int rpid, status;
    while ((rpid = wait(&status)) != fork_result && rpid != -1)
	;
    unlink((char*)path);

    if (newcontent.length() > 0)
    {
	static HTML			*html = 0;
	static Plaintext		*plaintext = 0;
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
	else 
	{
	    if (!plaintext)
		plaintext = new Plaintext();
	    parsable = plaintext;
	    if (debug)
		cout << "External parser error: \"" << contentType <<
			"\" not a recognized type.  Assuming text/plain\n";
	}
	parsable->setContents(newcontent.get(), newcontent.length());
	parsable->parse(retriever, base);
    }
}


