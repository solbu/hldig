//
// ExternalParser.cc
//
// Implementation of ExternalParser
//
// $Log: ExternalParser.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: ExternalParser.cc,v 1.1.1.1 1997/02/03 17:11:06 turtle Exp $";
#endif

#include "ExternalParser.h"
#include "htdig.h"
#include <String.h>
#include <QuotedStringList.h>
#include <URL.h>
#include <Dictionary.h>
#include <ctype.h>
#include <stdio.h>

static Dictionary	*parsers = 0;
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
	
	QuotedStringList	qsl(config["external_parsers"], " \t");
	int			i;

	for (i = 0; qsl[i]; i += 2)
	{
	    parsers->Add(qsl[i], new String(qsl[i + 1]));
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
    String	path = "/tmp/htdext.";
    path << getpid();

    FILE	*fl = fopen(path, "w");
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
    command << ' ' << path << ' ' << contentType << ' ' << base.get() <<
	' ' << configFile;

    FILE	*input = popen(command, "r");
    if (!input)
    {
	return;
    }

    String	line;
    char	*token1, *token2, *token3;
    URL		url;
    while (readLine(input, line))
    {
	token1 = strtok(line, "\t");
	switch (*token1)
	{
	    case 'w':	// word
		token1 = strtok(0, "\t");
		token2 = strtok(0, "\t");
		token3 = strtok(0, "\t");
		retriever.got_word(token1, atoi(token2), atoi(token3));
		break;
		
	    case 'u':	// href
		token1 = strtok(0, "\t");
		token2 = strtok(0, "\t");
		url.parse(token1);
		retriever.got_href(url, token2);
		break;
		
	    case 't':	// title
		token1 = strtok(0, "\t");
		retriever.got_title(token1);
		break;
		
	    case 'h':	// head
		token1 = strtok(0, "\t");
		retriever.got_head(token1);
		break;
		
	    case 'a':	// anchor
		token1 = strtok(0, "\t");
		retriever.got_anchor(token1);
		break;
		
	    case 'i':	// image url
		token1 = strtok(0, "\t");
		retriever.got_image(token1);
		break;
	}
    }
    pclose(input);
}


