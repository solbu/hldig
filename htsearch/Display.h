//
// Display.h
//
// $Id: Display.h,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $
//
// $Log: Display.h,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _Display_h_
#define _Display_h_

#include <Object.h>
#include "ResultList.h"
#include "ResultMatch.h"
#include "TemplateList.h"
#include <cgi.h>
#include <StringMatch.h>
#include <StringMatch.h>
#include <List.h>
#include <DocumentDB.h>
#include <Database.h>
#include <Dictionary.h>

class Display : public Object
{
public:
    //
    // Construction/Destruction
    //
    Display(char *indexFile, char *docFile);
    ~Display();

    void		setStartTemplate(char *templateName);
    void		setMatchTemplate(char *templateName);
    void		setEndTemplate(char *templateName);
	
    void		setResults(ResultList *results);
    void		setSearchWords(List *searchWords);
    void		setLimit(StringMatch *);
    void		setExclude(StringMatch *);
    void		setAllWordsPattern(StringMatch *);
    void		setLogicalWords(char *);
    void		setOriginalWords(char *);
    void		setCGI(cgi *);
	
    void		display(int pageNumber);
    void		displayMatch(ResultMatch *);
    void		displayHeader();
    void		displayFooter();
    void		displayNomatch();
    void		displaySyntaxError(char *);
	
protected:
    //
    // The list of search results.
    //
    ResultList		*results;

    //
    // The database that contains documents.
    //
    DocumentDB		docDB;

    //
    // The database that translates document IDs to URLs.
    //
    Database		*docIndex;

    //
    // A list of words that we are searching for
    //
    List		*searchWords;

    //
    // Pattern that all result URLs must match or exclude
    //
    StringMatch		*limitTo;
    StringMatch		*excludeFrom;

    //
    // Pattern of all the words
    //
    StringMatch		*allWordsPattern;
	
    //
    // Variables for substitution into text are stored in a dictionary
    //
    Dictionary		vars;

    //
    // Since the creation of excerpts is somewhat time consuming, we will
    // only compute them if they're actually going to be used.  This is the
    // flag that tells us if we will need the excerpt.
    //
    int			needExcerpt;

    //
    // To allow the star images to be dependant on the match URL, we need
    // the following:
    //
    StringMatch		URLimage;
    List		URLimageList;

    //
    // Maximum number of stars to display
    //
    int			maxStars;
    int			maxScore;

    //
    // For display, we have different versions of the list of words.
    //
    String		logicalWords;
    String		originalWords;

    //
    // To be able to recreate the URL that will get to us again, we need
    // the info from the HTML form that called us.
    //
    cgi			*input;

    //
    // Match output is done through templates.  This is the interface to these
    // templates.
    //
    TemplateList	templates;
    Template		*currentTemplate;
	
    //
    // Methods...
    //
    List		*buildMatchList();
    void		sort(List *);
    static int		compare(const void *, const void *);
    int			includeURL(char *);
    String		*readFile(char *);
    void		expandVariables(char *);
    String		*excerpt(DocumentRef *ref, char *url = 0);
    char		*hilight(char *, char *);
    void		setupImages();
    String		*generateStars(DocumentRef *, int);
    void		displayParsedFile(char *);
    void		setVariables(int, List *);
    void		createURL(String &, int);
};

//*****************************************************************************
inline void
Display::setLimit(StringMatch *limit)
{
    limitTo = limit;
}

inline void
Display::setExclude(StringMatch *exclude)
{
    excludeFrom = exclude;
}

inline void
Display::setAllWordsPattern(StringMatch *pattern)
{
    allWordsPattern = pattern;
}

inline void
Display::setResults(ResultList *results)
{
    this->results = results;
}

inline void
Display::setSearchWords(List *searchWords)
{
    this->searchWords = searchWords;
}

inline void
Display::setLogicalWords(char *s)
{
    logicalWords = s;
    vars.Add("LOGICAL_WORDS", new String(logicalWords));
}

inline void
Display::setOriginalWords(char *s)
{
    originalWords = s;
    vars.Add("WORDS", new String(originalWords));
}

inline void
Display::setCGI(cgi *aCgi)
{
    input = aCgi;
}

#endif

