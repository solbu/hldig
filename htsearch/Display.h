//
// Display.h
//
// Display: Implementation of Display
//          Takes results of search and fills in the HTML templates
//
// $Id: Display.h,v 1.18 1999/09/09 10:16:07 loic Exp $
//
//

#ifndef _Display_h_
#define _Display_h_

#include "Object.h"
#include "ResultList.h"
#include "ResultMatch.h"
#include "TemplateList.h"
#include "cgi.h"
#include "StringMatch.h"
#include "List.h"
#include "DocumentDB.h"
#include "Database.h"
#include "Dictionary.h"
#include "HtRegex.h"

class Display : public Object
{
public:
    //
    // Construction/Destruction
    //
    Display(char *docFile, char *indexFile, char *excerptFile);
    ~Display();

    void		setStartTemplate(char *templateName);
    void		setMatchTemplate(char *templateName);
    void		setEndTemplate(char *templateName);
	
    void		setResults(ResultList *results);
    void		setSearchWords(List *searchWords);
    void		setLimit(HtRegex *);
    void		setExclude(HtRegex *);
    void		setAllWordsPattern(StringMatch *);
    void		setLogicalWords(char *);
    void		setOriginalWords(char *);
    void		setCGI(cgi *);
	
    void		display(int pageNumber);
    void		displayMatch(DocumentRef *, int current);
    void		displayHeader();
    void		displayFooter();
    void		displayNomatch();
    void		displaySyntaxError(char *);
	
    int                 hasTemplateError() {return templateError;}

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
    // A list of words that we are searching for
    //
    List		*searchWords;

    //
    // Pattern that all result URLs must match or exclude
    //
    HtRegex		*limitTo;
    HtRegex		*excludeFrom;

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
    // Since we might have errors we cannot recover from, this tells us 
    // what happened.
    //
    int                 templateError;

    //
    // To allow the result templates to be dependant on the match URL, we need
    // the following:
    //
    StringMatch		URLtemplate;
    List		URLtemplateList;

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

    int			includeURL(char *);
    String		*readFile(char *);
    void		expandVariables(char *);
    void		outputVariable(char *);
    String		*excerpt(DocumentRef *ref, String urlanchor,
				 int fanchor, int &first);
    char		*hilight(char *str, String urlanchor, int fanchor);
    void		setupTemplates();
    void		setupImages();
    String		*generateStars(DocumentRef *, int);
    void		displayParsedFile(char *);
    void		setVariables(int, List *);
    void		createURL(String &, int);
    void		logSearch(int, List *);
};

//*****************************************************************************
inline void
Display::setLimit(HtRegex *limit)
{
    limitTo = limit;
}

inline void
Display::setExclude(HtRegex *exclude)
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

