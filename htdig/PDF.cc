//
// PDF.cc : Implementation of class PDF
//
// Written by Sylvain Wallez, wallez@mail.dotcom.fr
//
#if RELEASE
static char RCSid[] = "$Id: PDF.cc,v 1.9.2.6 2000/02/15 20:57:04 grdetil Exp $";
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include "PDF.h"
#include "URL.h"
#include "htdig.h"
#include "htString.h"
#include "StringList.h"
#include <stdlib.h>
#include <ctype.h>
#include "HtWordType.h"


//*****************************************************************************
// PDF::PDF()
//
PDF::PDF()
{
    _data = 0;
    _dataLength = 0;
    _bigSpacing = 0;

    initParser();
}


//*****************************************************************************
// PDF::~PDF()
//
PDF::~PDF()
{
}


//*****************************************************************************
// void PDF::initParser()
//
void PDF::initParser()
{
    _inTextBlock = 0;
    _parsedString = 0;
    _pages = 1;
    _curPage = 0;
    _retriever = 0;
    _head = 0;
}

//*****************************************************************************
// void PDF::setContents(char* data, int length)
//
void
PDF::setContents(char* data, int length)
{
    if (debug > 2)
	printf("PDF::setContents(%d bytes)\n", length);

    if (data == 0 || length == 0)
    {
	_data = 0;
	_dataLength = 0;
    }
    else
    {
	_data = data;
	_dataLength = length;
    }

    initParser();
}

//*****************************************************************************
// void PDF::parse(Retriever &retriever, URL &)
//
void
PDF::parse(Retriever &retriever, URL &url)
{
    if (debug > 2)
    	printf("PDF::parse(%s)\n", url.get());

    if (_data == 0)
    {
	if (debug > 2)
	    printf("PDF::parse: empty contents\n");
	return;
    }

    initParser();
    _retriever = &retriever;

    String acroread;
    char* configValue = config["pdf_parser"];
    if (configValue && strlen(configValue))
	acroread = configValue;
    else
	// Assume it's in the path
        acroread = "acroread";

    // Check for existance of acroread program! (if not, return)
    struct stat stat_buf;
    static int notfound = 0;
    if (notfound)	// we only need to complain once
	return;
    String arg0 = acroread;
    char *endarg = strchr(arg0.get(), ' ');
    if (endarg)
	*endarg = '\0';
    // If first arg is a path, check that it exists, and is a regular file. 
    if (strchr(arg0.get(), '/') &&
	((stat(arg0.get(), &stat_buf) == -1) || !S_ISREG(stat_buf.st_mode)))
    {
	printf("PDF::parse: cannot find pdf parser %s\n", arg0.get());
	notfound = 1;
	return;
    }

    // Write the pdf contents in a temp file to give it to acroread

    // First build the base name. If pdf has no title, acroread will use this.
    _tempFileBase = "htdig";
    _tempFileBase << getpid();


    String tmpdir = getenv("TMPDIR");
    if (tmpdir.length() == 0)
      tmpdir = "/tmp";
    String pdfName = tmpdir;
    pdfName << "/" << _tempFileBase;
    String psName = pdfName;
    pdfName << ".pdf";
    psName << ".ps";

    FILE *file = fopen(pdfName, "w");
    if (!file)
    {
	printf("PDF::parse: cannot open %s\n", pdfName.get());
	return;
    }

    fwrite(_data, 1, _dataLength, file);
    fclose(file);


    // Use acroread as a filter to convert to PostScript.
    // Now generalized to allow xpdf as a parser, or other compatible parsers
    // (It was claimed it works with most recent xpdf, but it doesn't!)
    //    acroread << " -toPostScript " << pdfName << " " << tmpdir << " 2>&1";
    String dest = psName;
    if (strstr(acroread.get(), "acroread"))
    {
	// special-case tests only for acroread (what else you gonna use?)
	if (!strstr(acroread.get(), "-toPostScript"))
	    acroread << " -toPostScript ";	// add missing option
	if (!strstr(acroread.get(), "-pairs"))	// don't use -pairs with 4.0
	    dest = tmpdir;
    }
    acroread << " " << pdfName << " " << dest << " 2>&1";

    if (system(acroread))
    {
	printf("PDF::parse: error running pdf_parser on %s\n", url.get());
	unlink(pdfName);
	return;
    }
    FILE* psFile = fopen(psName, "r");
    if (!psFile)
    {
	printf("PDF::parse: cannot open acroread output from %s\n", url.get());
	unlink(pdfName);
	return;
    }

    // Parse outpout from acroread
    String line;
    int lines = 0;
    while(readLine(psFile, line))
    {
	// parse line

	if (debug > 4)
	    printf("PDF::parse read line %d<%s>\n", line.length(), line.get());

	lines++;

	if (_inTextBlock)
	    parseTextLine(line);
	else
	    parseNonTextLine(line);
    }
    // Handle incomplete last line
    parseString();

    if (debug > 3)
	printf("PDF::parse: head = \"%s\"\n", _head.get());

    _retriever->got_head(_head);

    fclose(psFile);
    unlink(pdfName);
    unlink(psName);

    if (lines < 2) // No output or one error line
	printf("PDF::parse: no data from acroread on url %s\n", url.get());
    else if (debug > 2)
	printf("PDF::parse: %d lines parsed\n", lines);


    if (debug > 2)
    	printf("PDF::parse ends normally\n");
}

//*****************************************************************************
// PDF::readLine
// (inspired from ExternalParser.cc)

int PDF::readLine(FILE* in, String &line)
{
    char	buffer[2048];
    int		length;

    line = 0;
    while (fgets(buffer, sizeof(buffer), in))
    {
	length = strlen(buffer);
	if (buffer[length - 1] == '\n')
	{
	    // A full line was read : strip linefeed
	    buffer[length - 1] = '\0';

	    if (buffer[length - 2] == '\\')
	    {
		// Long line is split in PS code
		// strip backslash and append to the line
		buffer[length - 2] = '\0';
		line << buffer;
	    }
	    else
	    {
		line << buffer;
		return 1;
	    }
	}
	else
	{
	    // Only a partial line was read.  Append it to the line
	    // and read some more.
	    line << buffer;
	}
    }
    return line.length() > 0;
}

//*****************************************************************************
// PDF::parseNonTextLine

void PDF::parseNonTextLine(String &line)
{
   // To do :
   //   Handle %%Encoding for non iso-latin1 encoding
   //   Add an anchor on each page
   //   (useful only if acrobat plugin can be page-driven by the URL)

   char	*position = line.get();

   // Check for PS comment
   if (!strncmp(position, "%%", 2))
   {
	int val;
	// EPS comment : get title or page
	if (sscanf(position, "%%%%Page: %d", &val))
	{
	    // Current page number
	    if (debug > 3)
		printf("PDF::parseNonTextLine: start page %d\n", val);

	    _curPage = (val > _pages) ? _pages : val;
	}
	if (sscanf(position, "%%%%Pages: %d", &val))
	{
	    // Total number of pages
	    if (debug > 3)
		printf("PDF::parseNonTextLine: total pages is %d\n", val);

	    _pages = val;
	}
	if (!strncmp(position, "%%Title: (", 10))
	{
	    // Title of the PDF document
	    // Decode the title in _parsedString
	    position += 10; // Skip "%%Title: "

	    // Parse pending text
	    parseString();

	    addToString(position);

	    if (strncmp(_tempFileBase, _parsedString, _tempFileBase.length()))
	    {
		// PDF file really has a title
		if (debug > 3)
		    printf("PDF::parseNonTextLine: title is \"%s\"\n",
			_parsedString.get());

		_retriever->got_title(_parsedString);
	    }
	    _parsedString = 0;
	}
	
   }
   else if ( mystrncasecmp( line.get(), "BT", 2 ) == 0 )
   {
	// Beginning of text block
	if (debug > 3)
	    printf("PDF::parseNonTextLine: begin text block\n");

        _inTextBlock = 1;
   }
   // All other are ignored
}

//*****************************************************************************
// PDF::parseTextLine

void PDF::parseTextLine(String &line)
{
    char *position = line.get();
    int   length   = line.length();

    if (length == 0) return;

    // Find the command :
    // find first non alphanumeric char from the end of the line

    char *cmd = position + length -1; // last char of line

    if (*cmd == '*') cmd--; // Special case for the "T*" command

    while (cmd >= position && isalpha(*cmd))
	cmd--;

    // Go back to first char of command
    cmd++;

    if (debug > 3)
	printf("PDF::parseTextLine(\"%s\") cmd=%s\n", position, cmd);

    // Analyze command
    if (!strcmp(cmd, "TJ"))
    {
	// Parameter is an array of strings and integers
	// Parse all strings found in the line
	while ((position = strchr(position, '(')))
	{
	    position++;
	    position = addToString(position);
	    // Do not parse now
	}
    }
    else if (!strcmp(cmd, "Tj"))
    {
	// Parameter is a single string
	position = strchr(position, '(');
	if (position)
	{
	    position++;
	    addToString(position);
	    // Do not parse now
	}
    }
    else if (!strcmp(cmd, "ET"))
    {
	// End of text block
	_inTextBlock = 0;
	parseString();
    }
    else if (!strcmp(cmd, "Td") || !strcmp(cmd, "TD") ||
             !strcmp(cmd, "Tm") || !strcmp(cmd, "T*"))
    {
	// Text positioning commands Td, TD, Tm and T* are considered
	// as a word break (see PDF 1.2 spec, chapter 8.7.3)
	parseString();
    }
    else if (!strcmp(cmd, "Tc"))
    {
	// Text positioning command Tc, with operand of 3 or more, seems
	// sometimes to act as a word break between or after characters in
	// the following Tj command.  (E.g. PDFs generated from .cdr files.)
	_bigSpacing = (atof(position) >= 3.0);
    }
    else
    {
	// Other commands are not considered as a word break
    }
}

//*****************************************************************************
// PDF::addToString

// Check octal digit
#ifndef isodigit
#define isodigit(x) ((x)>='0' && (x)<='7')
#endif

char * PDF::addToString(char *position)
{
    char *pos;

    pos = position;
    while (*pos && *pos != ')')
    {
	if (*pos == '\\')
	{
	    // Escaped char : analyze next one
	    pos++;

	    if (isodigit(*pos))
	    {
		// 1 to 3 octal digits encoding
		unsigned char val = 0;
		for (int i = 0; i < 3 && *pos && isodigit(*pos); i++, pos++)
		{
		    val = val * 8 + (*pos - '0');
		}
		if (val)
		{
		    switch(val)
		    {
			case 0222:
			    _parsedString << '\'';
			    break;
			case 0267:
			    _parsedString << '*';
			    break;
			case 0336:
			    _parsedString << "fi";
			    break;
			default:
			    _parsedString << (char)val;
		    }
		    if (_bigSpacing)
			_parsedString << ' ';

		    // To do : handle more special characters
		}
	    }
	    else
	    {
		switch(*pos)
		{
		    case 'n' : // linefeed
		    case 'r' : // carriage return
		    case 't' : // horizontal tab
		    case 'b' : // backspace
		    case 'f' : // formfeed
		        // They all are considered as a word separator
		        _parsedString << ' ';
			pos++;
		        break;

		    default :
			// Add the escaped character
			_parsedString << *pos;
			if (_bigSpacing)
			    _parsedString << ' ';
			pos++;
		}
	    }
	}
	else
	{
	    // Add character to the string
	    _parsedString << *pos;
	    if (_bigSpacing)
		_parsedString << ' ';
	    pos++;
	}
    }
    
    return pos;
}

//*****************************************************************************
// PDF::parseString : Taken from Plaintext.cc

void PDF::parseString()
{
    if (_parsedString.length() == 0)
	return;

    static int	minimumWordLength = config.Value("minimum_word_length", 3);
    char	*position = _parsedString.get();
    /* Currently unused    char	*start = position; */
    int		in_space = 0;
    String	word;

    int		head_length = _head.length();

    while (*position)
    {
	word = 0;

	if (HtIsStrictWordChar(*position))
	{
	    //
	    // Start of a word.  Try to find the whole thing
	    //
	    in_space = 0;
	    while (*position && HtIsWordChar(*position))
	    {
		word << *position;
		position++;
	    }

	    if (_head.length() < max_head_length)
	    {
		_head << word;
	    }

	    if (word.length() >= minimumWordLength)
	    {
		_retriever->got_word(word,
				     int(_curPage * 1000 / _pages),
				     0);
		if (debug > 3)
		    printf("PDF::parseString: got word %s\n", word.get());
	    }
	}
		
	if (_head.length() < max_head_length)
	{
	    //
	    // Characters that are not part of a word
	    //
	    if (*position && isspace(*position))
	    {
		//
		// Reduce all multiple whitespace to a single space
		//
		if (!in_space)
		{
		    _head << ' ';
		}
		in_space = 1;
	    }
	    else
	    {
		//
		// Non whitespace
		//
		switch (*position)
		{
		    case '<':
			_head << "&lt;";
			break;
		    case '>':
			_head << "&gt;";
			break;
		    case '&':
			_head << "&amp;";
			break;
		    case '\0':
			break;
		    default:
			_head << *position;
			break;
		}
		in_space = 0;
	    }
	}
	if (*position)
	    position++;
    }

    if (_head.length() != head_length)
    {
	// Some words were added to _head : append a space for next call
	_head << ' ';
    }

    // Flush parsed string
    _parsedString = 0;
    _bigSpacing = 0;
}

