//--------------------------------------------------------------------
//
// ResultFetch.h
//
// 2/6/2002 created for libhtdig
//
// Neal Richter nealr@rightnow.com
//
//
// ResultFetch: Takes results of search and fills in the HTML templates
//
// FOR USE IN LIBHTDIG... does NOT stream to stdout!!
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library Public License version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: ResultFetch.h,v 1.2 2003/06/23 22:28:17 nealr Exp $
//
//--------------------------------------------------------------------

#ifndef _ResultFetch_h_
#define _ResultFetch_h_

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

class ResultFetch : public Object
{
public:
    //
    // Construction/Destruction
    //
    // Display(const String& docFile, const String& indexFile, const String& excerptFile);

    ResultFetch(Dictionary *selected_collections, const StringList& templist);
    ResultFetch(Dictionary *selected_collections);
    ~ResultFetch();

    void		setStartTemplate(const String& templateName);
    void		setMatchTemplate(const String& templateName);
    void		setEndTemplate(const String& templateName);
	
    // inline void		setResults(ResultList *results);
    // inline void		setSearchWords(List *searchWords);
    inline void		setLimit(HtRegex *);
    inline void		setExclude(HtRegex *);
    // inline void		setAllWordsPattern(StringMatch *);
    inline void		setLogicalWords(char *);
    inline void		setOriginalWords(char *);
    inline void		setCGI(cgi *);
	
    //void		        fetch(int pageNumber);
    //void      		fetchMatch(ResultMatch *match, DocumentRef *ref, int current);
    List *		        fetch();
    Dictionary *		fetchMatch(ResultMatch *match, DocumentRef *ref, int current);
    void	        	displayHeader();
    void		        displayFooter();
    void        		displayNomatch();
    void		        displaySyntaxError(const String &);
	
    int                 hasTemplateError() {return templateError;}

protected:
    //
    // Multiple database support
    //
    Dictionary          *selected_collections;

    //
    // Search Policy
    char                *search_policy;

    //
    // The list of search results.
    //
    // ResultList		*results;

    //
    // The database that contains documents.
    //
    // DocumentDB		docDB;

    // List of databases to search on
    StringList  collectionList; 
    
    //
    // A list of words that we are searching for
    //
    // List		*searchWords;

    //
    // Pattern that all result URLs must match or exclude
    //
    HtRegex		*limitTo;
    HtRegex		*excludeFrom;

    //
    // Pattern of all the words
    //
    // StringMatch		*allWordsPattern;
	
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
    double		maxScore;
    double		minScore;

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
    String		*excerpt(ResultMatch *match, DocumentRef *ref, String urlanchor,
				 int fanchor, int &first);
    String		hilight(ResultMatch *match, const String& str, const String& urlanchor, int fanchor);
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
ResultFetch::setLimit(HtRegex *limit)
{
    limitTo = limit;
}

inline void
ResultFetch::setExclude(HtRegex *exclude)
{
    excludeFrom = exclude;
}

#if 0
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
#endif

inline void
ResultFetch::setLogicalWords(char *s)
{
    logicalWords = s;
    vars.Add("LOGICAL_WORDS", new String(logicalWords));
}

inline void
ResultFetch::setOriginalWords(char *s)
{
    originalWords = s;
    vars.Add("WORDS", new String(originalWords));
}

inline void
ResultFetch::setCGI(cgi *aCgi)
{
    input = aCgi;
}

#endif

