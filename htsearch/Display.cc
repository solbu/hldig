//
// Display.cc
//
// Implementation of Display
//
// $Log: Display.cc,v $
// Revision 1.8  1998/08/03 09:57:20  ghutchis
//
// Fixed spelling mistake for "ellipses"
//
// Revision 1.7  1998/07/22 10:04:31  ghutchis
//
// Added patches from Sylvain Wallez <s.wallez.alcatel@e-mail.com> to
// Display.cc to use the filename if no title is found and Chris Jason
// Richards <richards@cs.tamu.edu> to htnotify.cc to fix problems with sendmail.
//
// Revision 1.6  1998/07/21 09:56:58  ghutchis
//
// Added patch by Rob Stone <rob@psych.york.ac.uk> to create new
// environment variables to htsearch: SELECTED_FORMAT and SELECTED_METHOD.
//
// Revision 1.5  1998/07/16 15:15:28  ghutchis
//
// Added patch from Stephan Muehlstrasser <smuehlst@Rational.Com> to fix
// delete syntax and a memory leak.
//
// Revision 1.4  1998/06/21 23:20:10  turtle
// patches by Esa and Jesse to add BerkeleyDB and Prefix searching
//
// Revision 1.3  1998/04/03 17:10:44  turtle
// Patch to make excludes work
//
// Revision 1.2  1997/06/16 15:31:04  turtle
// Added PERCENT and VERSION variables for the output templates
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Display.cc,v 1.8 1998/08/03 09:57:20 ghutchis Exp $";
#endif

#include "htsearch.h"
#include "Display.h"
#include "ResultMatch.h"
#include "WeightWord.h"
#include <QuotedStringList.h>
#include <URL.h>
#include <fstream.h>
#include <stdio.h>
#include <ctype.h>


//*****************************************************************************
//
Display::Display(char *indexFile, char *docFile)
{
    docIndex = Database::getDatabaseInstance();
    docIndex->OpenRead(indexFile);
    docDB.Read(docFile);

    limitTo = 0;
    excludeFrom = 0;
    needExcerpt = 0;

    maxStars = config.Value("max_stars");
    maxScore = 100;
    setupImages();

    templates.createFromString(config["template_map"]);
    currentTemplate = templates.get(config["template_name"]);
    if (!currentTemplate)
    {
	//
	// Must have been some error.  Resort to the first template
	//
	currentTemplate = (Template *) templates.templates[0];
    }
	
    if (mystrcasestr(currentTemplate->getMatchTemplate(), "excerpt"))
	needExcerpt = 1;
}

//*****************************************************************************
Display::~Display()
{
    delete docIndex;
}

//*****************************************************************************
//
void
Display::display(int pageNumber)
{
    List		*matches = buildMatchList();
    int			currentMatch = 0;
    int			numberDisplayed = 0;
    ResultMatch	*match = 0;
    int			number = config.Value("matches_per_page");
    int			startAt = (pageNumber - 1) * number;

    cout << "Content-type: text/html\r\n\r\n";

    setVariables(pageNumber, matches);
	
    //
    // The first match is guaranteed to have the highest score of
    // all the matches.  We use this to compute the number of stars
    // to display for all the other matches.
    //
    match = (ResultMatch *) (*matches)[0];
    if (!match)
    {
	//
	// No matches.
	//
        delete matches;
	displayNomatch();
	return;
    }
    maxScore = match->getScore();
	
    displayHeader();
	
    //
    // Display the window of matches requested.
    //
    if (currentTemplate->getStartTemplate())
    {
	expandVariables(currentTemplate->getStartTemplate());
    }
    matches->Start_Get();
    while ((match = (ResultMatch *)matches->Get_Next()) &&
	   numberDisplayed < number)
    {
	if (currentMatch >= startAt)
	{
	    match->setRef(docDB[match->getURL()]);
	    DocumentRef	*ref = match->getRef();
	    if (!ref)
		continue;	// The document isn't present for some reason
	    ref->DocAnchor(match->getAnchor());
	    ref->DocScore(match->getScore());
	    displayMatch(match);
	    numberDisplayed++;
	}
	currentMatch++;
    }

    if (currentTemplate->getEndTemplate())
    {
	expandVariables(currentTemplate->getEndTemplate());
    }
    displayFooter();

    delete matches;
}

//*****************************************************************************
// Return true if the specified URL should be counted towards the results.
int
Display::includeURL(char *url)
{
    if (limitTo && limitTo->FindFirst(url) < 0)
    {
	return 0;
    }
    else
    {
	if (excludeFrom &&
            excludeFrom->hasPattern() &&
            excludeFrom->FindFirst(url) < 0)
	    return 0;
	else
	    return 1;
    }
}

//*****************************************************************************
void
Display::displayMatch(ResultMatch *match)
{
    String	*str;
	
    DocumentRef	*ref = match->getRef();

    if (needExcerpt)
    {
	vars.Add("EXCERPT", excerpt(ref));
    }
    char    *url = match->getURL();
    vars.Add("URL", new String(url));
    vars.Add("SCORE", new String(form("%d", match->getScore())));
    char	*title = ref->DocTitle();
    if (!title || !*title)
      {
	title = strrchr(url, '/');
	if (title)
	  {
	    title++; // Skip slash
	    str = new String(form("[%s]", title));
	  }
	else
	  // URL without '/' ??
	  str = new String("[No title]");
      }
    else
      str = new String(title);
    vars.Add("TITLE", str);
    vars.Add("STARSRIGHT", generateStars(ref, 1));
    vars.Add("STARSLEFT", generateStars(ref, 0));
    vars.Add("SIZE", new String(form("%d", ref->DocSize())));
    vars.Add("SIZEK", new String(form("%d",
					  (ref->DocSize() + 1023) / 1024)));

    if (maxScore != 0)
	vars.Add("PERCENT", new String(form("%d", (int)(ref->DocScore() * 100 /
							(double)maxScore))));
    else
	vars.Add("PERCENT", new String("100"));
    
    {
	str = new String();
	char		buffer[100];
	time_t		t = ref->DocTime();
	if (t)
	{
	    struct tm	*tm = localtime(&t);
//			strftime(buffer, sizeof(buffer), "%e-%h-%Y", tm);
	    strftime(buffer, sizeof(buffer), "%x", tm);
	    *str << buffer;
	}
	vars.Add("MODIFIED", str);
    }
	
    vars.Add("HOPCOUNT", new String(form("%d", ref->DocHopCount())));
    vars.Add("DOCID", new String(form("%d", ref->DocID())));
	
    {
	str = new String();
	List	*list = ref->Descriptions();
	int		n = list->Count();
	for (int i = 0; i < n; i++)
	{
	    *str << ((String*) (*list)[i])->get() << "<br>\n";
	}
	vars.Add("DESCRIPTIONS", str);
    }

    expandVariables(currentTemplate->getMatchTemplate());
}

//*****************************************************************************
void
Display::setVariables(int pageNumber, List *matches)
{
    String	tmp;
    int		i;
    int		nMatches = 0;

    if (matches)
	nMatches = matches->Count();
	
    int		matchesPerPage = config.Value("matches_per_page");
    int		nPages = (nMatches + matchesPerPage - 1) / matchesPerPage;

    if (nPages < 1)
	nPages = 1;			// We always have at least one page...
    vars.Add("MATCHES_PER_PAGE", new String(config["matches_per_page"]));
    vars.Add("MAX_STARS", new String(config["max_stars"]));
    vars.Add("CONFIG", new String(config["config"]));
    vars.Add("VERSION", new String(config["version"]));
    vars.Add("RESTRICT", new String(config["restrict"]));
    vars.Add("EXCLUDE", new String(config["exclude"]));
    if (mystrcasecmp(config["match_method"], "and") == 0)
	vars.Add("MATCH_MESSAGE", new String("all"));
    else if (mystrcasecmp(config["match_method"], "or") == 0)
	vars.Add("MATCH_MESSAGE", new String("some"));
    vars.Add("MATCHES", new String(form("%d", nMatches)));
    vars.Add("PLURAL_MATCHES", new String(nMatches == 0 ? "" : "s"));
    vars.Add("PAGE", new String(form("%d", pageNumber)));
    vars.Add("PAGES", new String(form("%d", nPages)));
    vars.Add("FIRSTDISPLAYED",
		 new String(form("%d", (pageNumber - 1) *
				 matchesPerPage + 1)));
    if (nPages > 1)
	vars.Add("PAGEHEADER", new String(config["page_list_header"]));
    else
	vars.Add("PAGEHEADER", new String(config["no_page_list_header"]));
	
    i = pageNumber * matchesPerPage;
    if (i > nMatches)
	i = nMatches;
    vars.Add("LASTDISPLAYED", new String(form("%d", i)));
	
    vars.Add("CGI", new String(getenv("SCRIPT_NAME")));
	
    String	*str;
    char	*format = input->get("format");
    String	*in;

    vars.Add("SELECTED_FORMAT", new String(format));

    str = new String();
    *str << "<select name=format>\n";
    for (i = 0; i < templates.displayNames.Count(); i++)
    {
	in = (String *) templates.internalNames[i];
	*str << "<option value=\"" << in->get() << '"';
	if (format && mystrcasecmp(in->get(), format) == 0)
	{
	    *str << " selected";
	}
	*str << '>' << ((String*)templates.displayNames[i])->get() << '\n';
    }
    *str << "</select>\n";
    vars.Add("FORMAT", str);

    str = new String();
    QuotedStringList	ml(config["method_names"], " \t\r\n");
    *str << "<select name=method>\n";
    for (i = 0; i < ml.Count(); i += 2)
    {
	*str << "<option value=" << ml[i];
	if (mystrcasecmp(ml[i], config["match_method"]) == 0)
	    *str << " selected";
	*str << '>' << ml[i + 1] << '\n';
    }
    *str << "</select>\n";
    vars.Add("METHOD", str);

    vars.Add("SELECTED_METHOD", new String(config["match_method"]));
	
    //
    // If a paged output is required, set the appropriate variables
    //
    if (nMatches > config.Value("matches_per_page"))
    {
	if (pageNumber > 1)
	{
	    str = new String("<a href=\"");
	    tmp = 0;
	    createURL(tmp, pageNumber - 1);
	    *str << tmp << "\">" << config["prev_page_text"] << "</a>";
	}
	else
	{
	    str = new String(config["no_prev_page_text"]);
	}
	vars.Add("PREVPAGE", str);
		
	if (pageNumber < nPages)
	{
	    str = new String("<a href=\"");
	    tmp = 0;
	    createURL(tmp, pageNumber + 1);
	    *str << tmp << "\">" << config["next_page_text"] << "</a>";
	}
	else
	{
	    str = new String(config["no_next_page_text"]);
	}
	vars.Add("NEXTPAGE", str);

	str = new String();
	char	*p;
	QuotedStringList	pnt(config["page_number_text"], " \t\r\n");
	QuotedStringList	npnt(config["no_page_number_text"], " \t\r\n");
	if (nPages > config.Value("maximum_pages", 10))
	    nPages = config.Value("maximum_pages");
	for (i = 1; i <= nPages; i++)
	{
	    if (i == pageNumber)
	    {
		p = npnt[i - 1];
		if (!p)
		    p = form("%d", i);
		*str << p << ' ';
	    }
	    else
	    {
		p = pnt[i - 1];
		if (!p)
		    p = form("%d", i);
		*str << "<a href=\"";
		tmp = 0;
		createURL(tmp, i);
		*str << tmp << "\">" << p << "</a> ";
	    }
	}
	vars.Add("PAGELIST", str);
    }
}

//*****************************************************************************
void
Display::createURL(String &url, int pageNumber)
{
    String	s;

    url << getenv("SCRIPT_NAME") << '?';
    if (input->exists("restrict"))
	s << "restrict=" << input->get("restrict") << '&';
    if (input->exists("exclude"))
	s << "exclude=" << input->get("exclude") << '&';
    if (input->exists("config"))
	s << "config=" << input->get("config") << '&';
    if (input->exists("method"))
	s << "method=" << input->get("method") << '&';
    if (input->exists("format"))
	s << "format=" << input->get("format") << '&';
    if (input->exists("matchesperpage"))
	s << "matchesperpage=" << input->get("matchesperpage") << '&';
    if (input->exists("words"))
	s << "words=" << input->get("words") << '&';
    s << "page=" << pageNumber;
    encodeURL(s);
    url << s;
}

//*****************************************************************************
void
Display::displayHeader()
{
    displayParsedFile(config["search_results_header"]);
}

//*****************************************************************************
void
Display::displayFooter()
{
    displayParsedFile(config["search_results_footer"]);
}

//*****************************************************************************
void
Display::displayNomatch()
{
    displayParsedFile(config["nothing_found_file"]);
}

//*****************************************************************************
void
Display::displaySyntaxError(char *message)
{
    cout << "Content-type: text/html\r\n\r\n";

    setVariables(0, 0);
    vars.Add("SYNTAXERROR", new String(message));
    displayParsedFile(config["syntax_error_file"]);
}

//*****************************************************************************
void
Display::displayParsedFile(char *filename)
{
    FILE	*fl = fopen(filename, "r");
    char	buffer[1000];

    while (fl && fgets(buffer, sizeof(buffer), fl))
    {
	expandVariables(buffer);
    }
    if (fl)
	fclose(fl);
}

//*****************************************************************************
// If the star images need to depend on the URL of the match, we need
// an efficient way to determine which image to use.  To do this, we
// will build a StringMatch object with all the URL patterns and also
// a List parallel to that pattern that contains the actual images to
// use for each URL.
//
void
Display::setupImages()
{
    char	*starPatterns = config["star_patterns"];
    if (!starPatterns || !*starPatterns)
    {
	//
	// Set the StringMatch object up so that it will never match
	// anything.  We know that '<' is an illegal character for
	// URLs, so this will effectively disable the matching.
	//
	URLimage.Pattern("<<<");
    }
    else
    {
	//
	// The starPatterns string will have pairs of values.  The first
	// value of a pair will be a pattern, the second value will be an
	// URL to an image.
	//
	char	*token = strtok(starPatterns, " \t\r\n");
	String	pattern;
	while (token)
	{
	    //
	    // First token is a pattern...
	    //
	    pattern << token << '|';

	    //
	    // Second token is an URL
	    //
	    token = strtok(0, " \t\r\n");
	    URLimageList.Add(new String(token));
	}
	pattern.chop(1);
	URLimage.Pattern(pattern);
    }
}

//*****************************************************************************
String *
Display::generateStars(DocumentRef *ref, int right)
{
    int		i;
    String	*result = new String();
    char	*image = config["star_image"];
    char	*blank = config["star_blank"];
    double	score;

    if (maxScore != 0)
    {
	score = ref->DocScore() / (double)maxScore;
    }
    else
    {
	maxScore = ref->DocScore();
	score = 1;
    }
    int		nStars = int(score * (maxStars - 1) + 0.5) + 1;

    if (right)
    {
	for (i = 0; i < maxStars - nStars; i++)
	{
	    *result << "<img src=\"" << blank << "\" alt=\" \">";
	}
    }

    int		match = 0;
    int		length = 0;
    int		status = URLimage.FindFirst(ref->DocURL(), match, length);

    if (status >= 0 && match >= 0)
    {
	image = ((String*) URLimageList[match])->get();
    }

    for (i = 0; i < nStars; i++)
    {
	*result << "<img src=\"" << image << "\" alt=\"*\">";
    }
	
    if (!right)
    {
	for (i = 0; i < maxStars - nStars; i++)
	{
	    *result << "<img src=\"" << blank << "\" alt=\" \">";
	}
    }

    *result << "\n";
    return result;
}

//*****************************************************************************
String *
Display::readFile(char *filename)
{
    FILE	*fl;
    String	*s = new String();
    char	line[1024];

    fl = fopen(filename, "r");
    while (fl && fgets(line, sizeof(line), fl))
    {
	*s << line;
    }
    return s;
}

//*****************************************************************************
void
Display::expandVariables(char *str)
{
    int		state = 0;
    String	var = "";
    String	*temp;

    while (str && *str)
    {
	switch (state)
	{
	    case 0:
		if (*str == '\\')
		    state = 1;
		else if (*str == '$')
		    state = 3;
		else
		    cout << *str;
		break;
	    case 1:
		cout << *str;
		state = 0;
		break;
	    case 2:
		//
		// We have a complete variable in var. Look it up and
		// see if we can find a good replacement for it.
		//
		temp = (String *) vars[var];
		if (temp)
		    cout << *temp;
		var = "";
		if (*str == '$')
		    state = 3;
		else if (*str == '\\')
		    state = 1;
		else
		{
		    state = 0;
		    cout << *str;
		}
		break;
	    case 3:
		if (*str == '(')
		    state = 4;
		else if (isalpha(*str) || *str == '_')
		{
		    var << *str;
		    state = 5;
		}
		else
		    state = 0;
		break;
	    case 4:
		if (*str == ')')
		    state = 2;
		else if (isalpha(*str) || *str == '_')
		    var << *str;
		else
		    state = 0;
		break;
	    case 5:
		if (isalpha(*str) || *str == '_')
		    var << *str;
		else if (*str == '$')
		    state = 6;
		else
		{
		    state = 2;
		    continue;
		}
		break;
	    case 6:
		//
		// We have a complete variable in var. Look it up and
		// see if we can find a good replacement for it.
		//
		temp = (String *) vars[var];
		if (temp)
		    cout << *temp;
		var = 0;
		if (*str == '(')
		    state = 4;
		else if (isalpha(*str) || *str == '_')
		{
		    var << *str;
		    state = 5;
		}
		else
		    state = 0;
		break;
	}
	str++;
    }
    if (state == 5)
    {
	//
	// The end of string was reached, but we are still trying to
	// put a variable together.  Since we now have a complete
	// variable, we will look up the value for it.
	//
	temp = (String *) vars[var];
	if (temp)
	    cout << *temp;
    }
}

//*****************************************************************************
List *
Display::buildMatchList()
{
    char	*id;
    String	url;
    ResultMatch	*thisMatch;
    List	*matches = new List();
	
    results->Start_Get();
    while ((id = results->Get_Next()))
    {
	//
	// Convert the ID to a URL
	//
	if (docIndex->Get(id, url) == NOTOK)
	{
	    continue;
	}

	if (!includeURL(url.get()))
	{
	    continue;
	}
		
	thisMatch = new ResultMatch();
	thisMatch->setURL(url);

	//
	// Get the actual document record into the current ResultMatch
	//
//	thisMatch->setRef(docDB[thisMatch->getURL()]);

	//
	// Assign the incomplete score to this match.  This score was
	// computed from the word database only, no excerpt context was
	// known at that time, so this still needs to be done by the
	// ResultMatch object.
	//
	DocMatch	*dm = results->find(id);
		
	thisMatch->setIncompleteScore(dm->score);
	thisMatch->setAnchor(dm->anchor);
		
	//
	// Append this match to our list of matches.
	//
	matches->Add(thisMatch);
    }

    //
    // The matches need to be ordered by relevance level.
    // Sort it.
    //
    sort(matches);

    return matches;
}

//*****************************************************************************
String *
Display::excerpt(DocumentRef *ref, char *url)
{
    char	*head = ref->DocHead();
    int		which, length;
    int		first = allWordsPattern->FindFirstWord(head, which, length);
    char	*temp = head;
    String	part;
    String	*text = new String();

    if (config.Boolean("excerpt_show_top", 0))
	first = 0;

    if (first < 0)
    {
	//
	// No excerpt available
	//
	if (config["no_excerpt_text"][0])
	{
	    *text << config["no_excerpt_text"];
	}
    }
    else
    {
	int	headLength = strlen(head);
	int	length = config.Value("excerpt_length", 50);
	char	*start;
	char	*end;
		
	if (!config.Boolean("add_anchors_to_excerpt"))
	    url = 0;

	//
	// Figure out where to start the excerpt.  Basically we go back
	// half the excerpt length from the first matched word
	//
	start = &temp[first] - length / 2;
	if (start < temp)
	    start = temp;
	else
	{
	    *text << config["start_ellipses"];
	    while (*start && isalpha(*start))
		start++;
	}

	//
	// Figure out the end of the excerpt.
	//
	end = start + length;
	if (end > temp + headLength)
	{
	    end = temp + headLength;
	    *text << hilight(start, url);
	}
	else
	{
	    while (*end && isalpha(*end))
		end++;
	    *end = '\0';
	    *text << hilight(start, url);
	    *text << config["end_ellipses"];
	}
    }
    return text;
}

//*****************************************************************************
char *
Display::hilight(char *str, char *url)
{
    static String	result;
    int			pos;
    int			which, length;
    WeightWord		*ww;
    int			first = 1;

    result = 0;
    while ((pos = allWordsPattern->FindFirstWord(str, which, length)) >= 0)
    {
	result.append(str, pos);
	ww = (WeightWord *) (*searchWords)[which];
	result << "<strong>";
	if (first && url)
	    result << "<a href=\"" << url << "\">";
	result.append(str + pos, length);
	if (first && url)
	    result << "</a>";
	result << "</strong>";
	str += pos + length;
	first = 0;
    }
    result.append(str);
    return result;
}

//*****************************************************************************
void
Display::sort(List *matches)
{
    int		numberOfMatches = matches->Count();
    int		i;

    ResultMatch	**array = new ResultMatch*[numberOfMatches];
    for (i = 0; i < numberOfMatches; i++)
    {
	array[i] = (ResultMatch *)(*matches)[i];
    }
    matches->Release();

    qsort((char *) array, numberOfMatches, sizeof(ResultMatch *),
	  Display::compare);

    for (i = 0; i < numberOfMatches; i++)
    {
	matches->Add(array[i]);
    }
    delete [] array;
}

//*****************************************************************************
int
Display::compare(const void *a1, const void *a2)
{
    ResultMatch	*m1 = *((ResultMatch **) a1);
    ResultMatch *m2 = *((ResultMatch **) a2);

    return m2->getScore() - m1->getScore();
}


