//
// PDF.h
//
// Written by Sylvain Wallez, wallez@mail.dotcom.fr
//
// This class parses PDF (acrobat) files.
// Parsing is done on PostScript translation of the PDF file by Acrobat Reader
// (acroread). It is freely available for most platform at www.adobe.com
//
// Using acroread as a translator avoids writing a complicated PDF
// parser that can handle the various compression mechanisms available in PDF.
// It allows also to keep this parser up to date when Adobe issues a new
// release of the PDF specification (PDF spec is available at www.adobe.com)
//
// However, there are still 2 problems :
// - you need acroread on your system
// - PDF 1.2 files can contain hrefs, and they are not included in PS 
//  conversion, preventing htDig to follow these links
//
//

#ifndef _PDF_h
#define _PDF_h_

#include "Parsable.h"
#include <stdio.h>

class StringList;
class URL;

class PDF : public Parsable
{
public:
    //
    // Construction/Destruction
    //
    PDF();
    ~PDF();

    //
    // Main parser interface.
    //
    virtual void	parse(Retriever &retriever, URL &url);
    virtual void	setContents(char* contents, int length);
	
private:
    // Contents and its length : to put it in a file, it's easier to use a
    // char* than String defined in Parsable.
    char *_data;
    int _dataLength;

    // Associated retriever : title, head and words are given to it.
    Retriever *_retriever;

    // True if inside text block (inside BT / ET block)
    int _inTextBlock;

    // A line, and even a single word, can be split over several strings
    // and several TJ or Tj commands. When this variable is true, text is
    // appended to _parsedString instead of parsing it.
    int _continueString;

    // Sometimes the character spacing, as set by the Tc command, is set
    // to a very high value, and is used to treat the characters in the next
    // Tj as separate words. When this variable is true, text is appended
    // to _parsedString with a space after each character, instead of as
    // a single word.
    int _bigSpacing;

    // String beeing read
    String _parsedString;

    // Head of the document
    String _head;

    // Total pages and current page number, got using %%Pages and %%Page
    // comments
    int _pages;
    int _curPage;

    // Base name of temp files (no direcory and no extension)
    String _tempFileBase;

    //-----------------------------------------------------------------------
    // Private methods

    // Initalize the parser
    void initParser();

    // Read a line in the PostScript file
    int readLine(FILE* in, String &line);

    // Parse a line outside a text block :
    // looks for title and pages
    void parseNonTextLine(String &line);

    // Parse a line inside a text block
    void parseTextLine(String &line);

    // Add a PostScript string to _parsedString
    char *addToString(char *position);

    // Parse _parsedString when _continueString is false
    void parseString();

};

#endif


