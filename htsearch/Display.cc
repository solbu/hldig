//
// Display.cc
//
// Implementation of Display
// Takes results of search and fills in the HTML templates
//
//
#if RELEASE
static char RCSid[] = "$Id: Display.cc,v 1.54.2.18 1999/11/26 21:56:33 grdetil Exp $";
#endif

#include "htsearch.h"
#include "Display.h"
#include "ResultMatch.h"
#include "WeightWord.h"
#include "StringMatch.h"
#include "QuotedStringList.h"
#include "URL.h"
#include <fstream.h>
#include <stdio.h>
#include <ctype.h>
#include <syslog.h>
#include <locale.h>
#include "HtURLCodec.h"
#include "HtWordType.h"

//*****************************************************************************
//
Display::Display(char *indexFile, char *docFile)
{
    docIndex = Database::getDatabaseInstance();
    docIndex->OpenRead(indexFile);

    // Check "uncompressed"/"uncoded" urls at the price of time
    // (extra DB probes).
    docDB.SetCompatibility(config.Boolean("uncoded_db_compatible", 1));

    docDB.Read(docFile);

    limitTo = 0;
    excludeFrom = 0;
    //    needExcerpt = 0;
    templateError = 0;

    maxStars = config.Value("max_stars");
    maxScore = 100;
    setupImages();

    if (!templates.createFromString(config["template_map"]))
      {
	// Error in createFromString.
	// Let's try the default template_map
	
	config.Add("template_map", 
		   "Long builtin-long builtin-long Short builtin-short builtin-short");
        if (!templates.createFromString(config["template_map"]))
	  {
	    // Unrecoverable Error
	    // (No idea why this would happen)
	    templateError = 1;
	  }
      }

    currentTemplate = templates.get(config["template_name"]);
    if (!currentTemplate)
    {
	//
	// Must have been some error.  Resort to the builtin-long (slot 0)
	//
	currentTemplate = (Template *) templates.templates[0];
    }
    if (!currentTemplate)
      {
        //
        // Another error!? Time to bail out...
        //
	templateError = 1;
      }
    //    if (mystrcasestr(currentTemplate->getMatchTemplate(), "excerpt"))
    //	needExcerpt = 1;
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
    ResultMatch		*match = 0;
    int			number = config.Value("matches_per_page");
    if (number <= 0)
	number = 10;
    int			startAt = (pageNumber - 1) * number;

    if (config.Boolean("logging"))
    {
        logSearch(pageNumber, matches);
    }

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
	cout << "Content-type: text/html\r\n\r\n";
	displayNomatch();
	return;
    }
    // maxScore = match->getScore();	// now done in buildMatchList()
    	
    cout << "Content-type: text/html\r\n\r\n";
    String	wrap_file = config["search_results_wrapper"];
    String	*wrapper = 0;
    char	*header = 0, *footer = 0;
    if (wrap_file.length())
    {
	wrapper = readFile(wrap_file.get());
	if (wrapper && wrapper->length())
	{
	    char	wrap_sepr[] = "HTSEARCH_RESULTS";
	    char	*h = wrapper->get();
	    char	*p = strstr(h, wrap_sepr);
	    if (p)
	    {
		if (p > h && p[-1] == '$')
		{
		    footer = p + strlen(wrap_sepr);
		    header = h;
		    p[-1] = '\0';
		}
		else if (p > h+1 && p[-2] == '$' &&
			 (p[-1] == '(' || p[-1] == '{') &&
			 (p[strlen(wrap_sepr)] == ')' ||
				p[strlen(wrap_sepr)] == '}'))
		{
		    footer = p + strlen(wrap_sepr) + 1;
		    header = h;
		    p[-2] = '\0';
		}
	    }
	}
    }
    if (header)
	expandVariables(header);
    else
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
	    displayMatch(match,currentMatch+1);
	    numberDisplayed++;
	    match->setRef(NULL);
	    delete ref;
	}
	currentMatch++;
    }

    if (currentTemplate->getEndTemplate())
    {
	expandVariables(currentTemplate->getEndTemplate());
    }
    if (footer)
	expandVariables(footer);
    else
	displayFooter();

    if (wrapper)
	delete wrapper;
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
            excludeFrom->FindFirst(url) >= 0)
	    return 0;
	else
	    return 1;
    }
}

//*****************************************************************************
void
Display::displayMatch(ResultMatch *match, int current)
{
    String	*str = 0;
	
    DocumentRef	*ref = match->getRef();

    char    *url = match->getURL();
    vars.Add("URL", new String(url));
    
    int     iA = ref->DocAnchor();
    
    String  *anchor = 0;
    int             fanchor = 0;
    if (iA > 0)             // if an anchor was found
      {
	List    *anchors = ref->DocAnchors();
	if (anchors->Count() >= iA)
	  {
	    anchor = new String();
	    fanchor = 1;
	    *anchor << "#" << ((String*) (*anchors)[iA-1])->get();
	    vars.Add("ANCHOR", anchor);
	  }
      }
    
    //
    // no condition for determining excerpt any more:
    // we need it anyway to see if an anchor is relevant
    //
    int first = -1;
    String urlanchor(url);
    if (anchor)
      urlanchor << anchor;
    vars.Add("EXCERPT", excerpt(ref, urlanchor, fanchor, first));
    //
    // anchor only relevant if an excerpt was found, i.e.,
    // the search expression matches the body of the document
    // instead of only META keywords.
    //
    if (first < 0)
      {
	vars.Remove("ANCHOR");
      }
    
    vars.Add("SCORE", new String(form("%d", match->getScore())));
    vars.Add("CURRENT", new String(form("%d", current)));
    char	*title = ref->DocTitle();
    if (!title || !*title)
      {
	if ( strcmp(config["no_title_text"], "filename") == 0 )
	  {
	    // use actual file name
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
	  // use configure 'no title' text
	  str = new String(config["no_title_text"]);
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
      {
	int percent = (int)(ref->DocScore() * 100 / (double)maxScore);
	if (percent <= 0)
	  percent = 1;
	vars.Add("PERCENT", new String(form("%d", percent)));
      }
    else
	vars.Add("PERCENT", new String("100"));
    
    {
	str = new String();
	char		buffer[100];
	time_t		t = ref->DocTime();
	if (t)
	{
	    struct tm	*tm = localtime(&t);
	    char	*datefmt = config["date_format"];
	    char	*locale  = config["locale"];
	    if (!datefmt || !*datefmt)
	      {
		if (config.Boolean("iso_8601"))
		    datefmt = "%Y-%m-%d %H:%M:%S %Z";
		else
		    datefmt = "%x";
	      }
	    if ( locale && *locale )
	      {
		setlocale(LC_TIME,locale);
	      }
	    strftime(buffer, sizeof(buffer), datefmt, tm);
	    *str << buffer;
	}
	vars.Add("MODIFIED", str);
    }
	
    vars.Add("HOPCOUNT", new String(form("%d", ref->DocHopCount())));
    vars.Add("DOCID", new String(form("%d", ref->DocID())));
    vars.Add("BACKLINKS", new String(form("%d", ref->DocBackLinks())));
	
    {
	str = new String();
	List	*list = ref->Descriptions();
	int		n = list->Count();
	for (int i = 0; i < n; i++)
	{
	    *str << ((String*) (*list)[i])->get() << "<br>\n";
	}
	vars.Add("DESCRIPTIONS", str);
        String *description = new String();
        if (list->Count())
            *description << ((String*) (*list)[0]);
        vars.Add("DESCRIPTION", description);
    }

    int		index = 0;
    int		length = 0;
    int		status = -1;
    if (URLtemplate.hasPattern())
	status = URLtemplate.FindFirst(ref->DocURL(), index, length);
    if (status >= 0 && index >= 0)
	displayParsedFile( ((String*) URLtemplateList[index])->get() );
    else
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
    if (matchesPerPage <= 0)
	matchesPerPage = 10;
    int		nPages = (nMatches + matchesPerPage - 1) / matchesPerPage;

    if (nPages < 1)
	nPages = 1;			// We always have at least one page...
    if (nPages > config.Value("maximum_pages", 10))
	nPages = config.Value("maximum_pages");
    vars.Add("MATCHES_PER_PAGE", new String(config["matches_per_page"]));
    vars.Add("MAX_STARS", new String(config["max_stars"]));
    vars.Add("CONFIG", new String(config["config"]));
    vars.Add("VERSION", new String(config["version"]));
    vars.Add("RESTRICT", new String(config["restrict"]));
    vars.Add("EXCLUDE", new String(config["exclude"]));
    vars.Add("KEYWORDS", new String(config["keywords"]));
    if (mystrcasecmp(config["match_method"], "and") == 0)
	vars.Add("MATCH_MESSAGE", new String("all"));
    else if (mystrcasecmp(config["match_method"], "or") == 0)
	vars.Add("MATCH_MESSAGE", new String("some"));
    vars.Add("MATCHES", new String(form("%d", nMatches)));
    vars.Add("PLURAL_MATCHES", new String(nMatches == 1 ? (char *)"" : (char *)"s"));
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

    if (strlen(config["script_name"]) != 0) {
      vars.Add("CGI", new String(config["script_name"]));
    } else {
      vars.Add("CGI", new String(getenv("SCRIPT_NAME")));
    }
	
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

    str = new String();
    QuotedStringList	sl(config["sort_names"], " \t\r\n");
    char		*st = config["sort"];
    StringMatch		datetime;
    datetime.IgnoreCase();
    datetime.Pattern("date|time");
    *str << "<select name=sort>\n";
    for (i = 0; i < sl.Count(); i += 2)
    {
	*str << "<option value=" << sl[i];
	if (mystrcasecmp(sl[i], st) == 0 ||
		datetime.Compare(sl[i]) && datetime.Compare(st) ||
		mystrncasecmp(sl[i], st, 3) == 0 &&
		    datetime.Compare(sl[i]+3) && datetime.Compare(st+3))
	    *str << " selected";
	*str << '>' << sl[i + 1] << '\n';
    }
    *str << "</select>\n";
    vars.Add("SORT", str);
    vars.Add("SELECTED_SORT", new String(st));
	
    //
    // If a paged output is required, set the appropriate variables
    //
    if (nPages > 1)
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
	QuotedStringList	sep(config["page_number_separator"], " \t\r\n");
	for (i = 1; i <= nPages; i++)
	{
	    if (i == pageNumber)
	    {
		p = npnt[i - 1];
		if (!p)
		    p = form("%d", i);
		*str << p;
	    }
	    else
	    {
		p = pnt[i - 1];
		if (!p)
		    p = form("%d", i);
		*str << "<a href=\"";
		tmp = 0;
		createURL(tmp, i);
		*str << tmp << "\">" << p << "</a>";
	    }
	    if (i != nPages)
		*str << ((sep.Count() > 0) ? sep[(i-1)%sep.Count()] : " ");
	}
	vars.Add("PAGELIST", str);
    }
    StringList form_vars(config["allow_in_form"], " \t\r\n");
    String* key;
    for (i= 0; i < form_vars.Count(); i++)
    {
      if (config[form_vars[i]])
      {
	key= new String(form_vars[i]);
	key->uppercase();
	vars.Add(key->get(), new String(config[form_vars[i]]));
      }
    }
}

//*****************************************************************************
void
Display::createURL(String &url, int pageNumber)
{
    String	s;
    int         i;

    if (strlen(config["script_name"]) != 0) {
      url << config["script_name"];
    } else {
      url << getenv("SCRIPT_NAME");
    }

    url << '?';

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
    if (input->exists("sort"))
	s << "sort=" << input->get("sort") << '&';
    if (input->exists("matchesperpage"))
	s << "matchesperpage=" << input->get("matchesperpage") << '&';
    if (input->exists("keywords"))
	s << "keywords=" << input->get("keywords") << '&';
    if (input->exists("words"))
	s << "words=" << input->get("words") << '&';
    StringList form_vars(config["allow_in_form"], " \t\r\n");
    for (i= 0; i < form_vars.Count(); i++)
    {
      if (input->exists(form_vars[i]))
      {
	s << form_vars[i] << '=' << input->get(form_vars[i]) << '&';
      }
    }
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
// If the result templates need to depend on the URL of the match, we need
// an efficient way to determine which template file to use.  To do this, we
// will build a StringMatch object with all the URL patterns and also
// a List parallel to that pattern that contains the actual template file
// names to use for each URL.
//
void
Display::setupTemplates()
{
    char	*templatePatterns = config["template_patterns"];
    if (templatePatterns && *templatePatterns)
    {
	//
	// The templatePatterns string will have pairs of values.  The first
	// value of a pair will be a pattern, the second value will be a
	// result template file name.
	//
	char	*token = strtok(templatePatterns, " \t\r\n");
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
	    URLtemplateList.Add(new String(token));
	    if (token)
	        token = strtok(0, " \t\r\n");
	}
	pattern.chop(1);
	URLtemplate.Pattern(pattern);
    }
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
    if (starPatterns && *starPatterns)
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
	    if (token)
	        token = strtok(0, " \t\r\n");
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
    if (!config.Boolean("use_star_image", 1))
	return result;

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
    int		status;

    if (URLimage.hasPattern())
      status = URLimage.FindFirst(ref->DocURL(), match, length);
    else
      status = -1;

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
		outputVariable(var);
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
		if (*str == '(' || *str == '{')
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
		if (*str == ')' || *str == '}')
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
		outputVariable(var);
		var = "";
		if (*str == '(' || *str == '{')
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
    if (state == 2 || state == 5)
    {
	//
	// The end of string was reached, but we are still trying to
	// put a variable together.  Since we now have a complete
	// variable, we will look up the value for it.
	//
	outputVariable(var);
    }
}

//*****************************************************************************
void
Display::outputVariable(char *var)
{
    String	*temp;
    char	*ev;

    // We have a complete variable name in var. Look it up and
    // see if we can find a good replacement for it, either in our
    // vars dictionary or in the environment variables.
    temp = (String *) vars[var];
    if (temp)
	cout << *temp;
    else
    {
	ev = getenv(var);
	if (ev)
	    cout << ev;
    }
}

//*****************************************************************************
List *
Display::buildMatchList()
{
    char	*id;
    String	coded_url, url;
    ResultMatch	*thisMatch;
    List	*matches = new List();
    double      backlink_factor = config.Double("backlink_factor");
    double      date_factor = config.Double("date_factor");
    SortType	typ = sortType();
	
    results->Start_Get();
    while ((id = results->Get_Next()))
    {
	//
	// Convert the ID to a URL
	//
	if (docIndex->Get(id, coded_url) == NOTOK)
	{
	    continue;
	}

	// No special precations re: the option
	// "uncoded_db_compatible" needs to be taken.
	url = HtURLCodec::instance()->decode(coded_url);
	if (!includeURL(url.get()))
	{
	    continue;
	}
	

	thisMatch = new ResultMatch();
	thisMatch->setURL(url);
	thisMatch->setRef(NULL);

	//
	// Get the actual document record into the current ResultMatch
	//
	//	thisMatch->setRef(docDB[thisMatch->getURL()]);

	//
	// Assign the incomplete score to this match.  This score was
	// computed from the word database only, no excerpt context was
	// known at that time, or info about the document itself, 
	// so this still needs to be done.
	//
	DocMatch	*dm = results->find(id);
	double           score = dm->score;

	// We need to scale based on date relevance and backlinks
	// Other changes to the score can happen now
	// Or be calculated by the result match in getScore()

	// This formula derived through experimentation
	// We want older docs to have smaller values and the
	// ultimate values to be a reasonable size (max about 100)

	if (date_factor != 0.0 || backlink_factor != 0.0 || typ != SortByScore)
	  {
	    DocumentRef *thisRef = docDB[thisMatch->getURL()];
	    if (thisRef)   // We better hope it's not null!
	      {
		score += date_factor * 
		  ((thisRef->DocTime() * 1000 / (double)time(0)) - 900);
		int links = thisRef->DocLinks();
		if (links == 0)
		  links = 1; // It's a hack, but it helps...
		score += backlink_factor
		  * (thisRef->DocBackLinks() / (double)links);
		if (score <= 0.0)
		  score = 0.0;
		if (typ != SortByScore)
		  {
		    DocumentRef *sortRef = new DocumentRef();
		    sortRef->DocTime(thisRef->DocTime());
		    if (typ == SortByTitle)
			sortRef->DocTitle(thisRef->DocTitle());
		    thisMatch->setRef(sortRef);
		  }
	      }
	    // Get rid of it to free the memory!
	    delete thisRef;
	  }

	thisMatch->setIncompleteScore(score);
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
Display::excerpt(DocumentRef *ref, String urlanchor, int fanchor, int &first)
{
    char	*head;
    int		use_meta_description = 0;

    if (config.Boolean("use_meta_description",0) 
	&& strlen(ref->DocMetaDsc()) != 0)
      {
	// Set the head to point to description
	head = ref->DocMetaDsc();
	use_meta_description = 1;
      }
    else head = ref->DocHead(); // head points to the top

    int		which, length;
    char	*temp = head;
    String	part;
    String	*text = new String();

    // htsearch displays the description when:
    // 1) a description has been found
    // 2) the option "use_meta_description" is set to true
    // If previous conditions are false and "excerpt_show_top" is set to true
    // it shows the whole head. Else, it acts as default.

    if (config.Boolean("excerpt_show_top", 0) || use_meta_description)
      first = 0;
    else
      first = allWordsPattern->FindFirstWord(head, which, length);

    if (first < 0 && config.Boolean("no_excerpt_show_top"))
      first = 0;  // No excerpt, but we want to show the top.

    if (first < 0)
    {
	//
	// No excerpt available, don't show top, so display message
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
	 // negate flag if it's on (anchor available)
	 fanchor = 0;

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
	    while (*start && HtIsStrictWordChar(*start))
		start++;
	}

	//
	// Figure out the end of the excerpt.
	//
	end = start + length;
	if (end > temp + headLength)
	{
	    end = temp + headLength;
	    *text << hilight(start, urlanchor, fanchor);
	}
	else
	{
	    while (*end && HtIsStrictWordChar(*end))
		end++;
	    *end = '\0';
	    *text << hilight(start, urlanchor, fanchor);
	    *text << config["end_ellipses"];
	}
    }
    return text;
}

//*****************************************************************************
char *
Display::hilight(char *str, String urlanchor, int fanchor)
{
    static char		*start_highlight = config["start_highlight"];
    static char		*end_highlight = config["end_highlight"];
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
	result << start_highlight;
	if (first && fanchor)
	    result << "<a href=\"" << urlanchor << "\">";
	result.append(str + pos, length);
	if (first && fanchor)
	    result << "</a>";
	result << end_highlight;
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
	if (i == 0 || maxScore < array[i]->getScore())
	    maxScore = array[i]->getScore();
    }
    matches->Release();

    SortType	typ = sortType();
    qsort((char *) array, numberOfMatches, sizeof(ResultMatch *),
	  (typ == SortByTitle) ? Display::compareTitle :
	  (typ == SortByTime) ? Display::compareTime :
	  Display::compare);

    char	*st = config["sort"];
    if (st && *st && mystrncasecmp("rev", st, 3) == 0)
    {
	for (i = numberOfMatches; --i >= 0; )
	    matches->Add(array[i]);
    }
    else
    {
	for (i = 0; i < numberOfMatches; i++)
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

//*****************************************************************************
int
Display::compareTime(const void *a1, const void *a2)
{
    ResultMatch	*m1 = *((ResultMatch **) a1);
    ResultMatch *m2 = *((ResultMatch **) a2);
    time_t	t1 = (m1->getRef()) ? m1->getRef()->DocTime() : 0;
    time_t	t2 = (m2->getRef()) ? m2->getRef()->DocTime() : 0;

    return (int) (t2 - t1);
}

//*****************************************************************************
int
Display::compareTitle(const void *a1, const void *a2)
{
    ResultMatch	*m1 = *((ResultMatch **) a1);
    ResultMatch *m2 = *((ResultMatch **) a2);
    char	*t1 = (m1->getRef()) ? m1->getRef()->DocTitle() : (char *)"";
    char	*t2 = (m2->getRef()) ? m2->getRef()->DocTitle() : (char *)"";

    if (!t1) t1 = "";
    if (!t2) t2 = "";
    return mystrcasecmp(t1, t2);
}

//*****************************************************************************
Display::SortType
Display::sortType()
{
    static struct
    {
	char		*typest;
	SortType	type;
    }
    sorttypes[] =
    {
	{"score", SortByScore},
	{"date", SortByTime},
	{"time", SortByTime},
	{"title", SortByTitle}
    };
    int		i = 0;
    char	*st = config["sort"];
    if (st && *st)
    {
	if (mystrncasecmp("rev", st, 3) == 0)
	    st += 3;
	for (i = sizeof(sorttypes)/sizeof(sorttypes[0]); --i > 0; )
	{
	    if (mystrcasecmp(sorttypes[i].typest, st) == 0)
		break;
	}
    }
    return sorttypes[i].type;
}

//*****************************************************************************
void
Display::logSearch(int page, List *matches)
{
    // Currently unused    time_t	t;
    int		nMatches = 0;
    int         level = LOG_LEVEL;
    int         facility = LOG_FACILITY;
    char        *host = getenv("REMOTE_HOST");
    char        *ref = getenv("HTTP_REFERER");

    if (host == NULL)
      host = getenv("REMOTE_ADDR");
    if (host == NULL)
      host = "-";

    if (ref == NULL)
      ref = "-";

    if (matches)
	nMatches = matches->Count();

    openlog("htsearch", LOG_PID, facility);
    syslog(level, "%s [%s] (%s) [%s] [%s] (%d/%s) - %d -- %s\n",
	   host,
	   input->exists("config") ? input->get("config") : "default",
	   config["match_method"], input->get("words"), logicalWords.get(),
	   nMatches, config["matches_per_page"],
	   page, ref
	   );
}
