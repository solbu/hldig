//
// Display.h
//
// Display: Takes results of search and fills in the HTML templates
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Display.h,v 1.23 1999/10/15 03:35:17 jtillman Exp $
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
//    Display(char *docFile, char *indexFile, char *excerptFile);
		Display();
    ~Display();

    void		setStartTemplate(const String& templateName);
    void		setMatchTemplate(const String& templateName);
    void		setEndTemplate(const String& templateName);
	
    inline void		setResults(ResultList *results);
    inline void		setSearchWords(List *searchWords);
    inline void		setLimit(HtRegex *);
    inline void		setExclude(HtRegex *);
    inline void		setAllWordsPattern(StringMatch *);
    inline void		setLogicalWords(char *);
    inline void		setOriginalWords(char *);
    inline void		setCGI(cgi *);
	
    void		display(int pageNumber);
    void		displayMatch(DocumentRef *, int current);
    void		displayHeader();
    void		displayFooter();
    void		displayNomatch();
    void		displaySyntaxError(const String &);
	
    int                 hasTemplateError() {return templateError;}

protected:
    //
    // The list of search results.
    //
    ResultList		*results;

    //
    // The database that contains documents.
    //
    //DocumentDB		docDB;

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

    int			includeURL(const String&);
    String		*readFile(const String&);
    void		expandVariables(const String&);
    void		outputVariable(const String&);
    String		*excerpt(DocumentRef *ref, String urlanchor,
				 int fanchor, int &first);
    String		hilight(const String& str, const String& urlanchor, int fanchor);
    void		setupTemplates();
    void		setupImages();
    String		*generateStars(DocumentRef *, int);
    void		displayParsedFile(const String&);
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




