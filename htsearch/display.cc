//
// display.cc
//
// Routines to display the search results
//
// $Log: display.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
// Revision 1.2  1996/01/03 19:01:10  turtle
// Before rewrite
//
// Revision 1.1  1995/11/04 01:02:44  turtle
// Does straight searching
//
//
#if RELEASE
static char RCSid[] = "$Id: display.cc,v 1.1.1.1 1997/02/03 17:11:05 turtle Exp $";
#endif

#include "htsearch.h"
#include "WeightWord.h"
#include "ResultList.h"
#include <StringMatch.h>
#include <ctype.h>

void compute_score(ResultMatch *m, DocMatch *dm, StringMatch &match);
void excerpt(DocumentRef &doc, List &searchWords, char *url, StringMatch &match);
void display_short_match(DocumentRef &doc, int maxscore, List &searchWords);
void display_raw_match(DocumentRef &doc, int maxscore, int match_number);
void display_match(DocumentRef &doc, int maxscore, List &searchWords, StringMatch &match);


void raw_show_vars(Dictionary &vars)
{
	cout << "[vars]\n";

	vars.Start_Get();
	char	*key;

	while ((key = vars.Get_Next()))
	{
		cout << key << ": " << ((String *) vars[key])->get() << endl;
	}
}

void nothing_found(int raw_output, Dictionary *vars)
{
	int		freeup = 0;
	if (!vars)
	{
		vars = new Dictionary();
		vars->Add("MATCHES", new String("0"));
		vars->Add("PLURAL_MATCHES", new String("s"));
		vars->Add("MAX_MATCHES", new String(form("%d", n_matches)));
		vars->Add("MAX_STARS", new String(config["max_stars"]));
		if (do_and)
			vars->Add("MATCH_MESSAGE", new String("all"));
		else
			vars->Add("MATCH_MESSAGE", new String("at least one"));

		vars->Add("LOGICAL_WORDS", new String(""));
		vars->Add("WORDS", new String(""));
		freeup = 1;
	}
	if (!raw_output)
	{
		dump_text(config["nothing_found_file"], *vars);
	}
	if (freeup)
	{
		delete vars;
	}
}


//*****************************************************************************
// void display(char *indexfile, char *docfile, ResultList *results, List &searchWords, int raw_output)
//   The results dictionary is indexed by document id and contains
//   lists of DocMatch objects.
//
void display(char *indexfile, char *docfile, ResultList *results, List &searchWords, int raw_output)
{
    Database	*doc_index = Database::getDatabaseInstance();
    DocumentDB	docdb;
    String		url;
    List		all_matches;
    WeightWord	*ww;

    //
    // The header of the results can have variables in it.  We will
    // set up a Dictionary with these variables so that they can be
    // used by the dump_text() function which will actually display
    // the header.
    //
    Dictionary	vars;
    vars.Add("MATCHES", new String(form("%d", all_matches.Count())));
    vars.Add("PLURAL_MATCHES", new String(all_matches.Count() == 1 ? "" : "s"));
    vars.Add("MAX_MATCHES", new String(form("%d", n_matches)));
    vars.Add("MAX_STARS", new String(config["max_stars"]));
    if (do_and)
		vars.Add("MATCH_MESSAGE", new String("all"));
    else
		vars.Add("MATCH_MESSAGE", new String("at least one"));

    vars.Add("LOGICAL_WORDS", new String(logicalWords));
    vars.Add("WORDS", new String(originalWords));

	if (raw_output)
	{
		raw_show_vars(vars);
	}
	
    if (results->Count() == 0)
    {
		//
		// No matches
		//
		nothing_found(raw_output, &vars);
		return;
    }

    StringMatch	wmatch;
    wmatch.IgnoreCase();
    if (searchWords.Count() > 1)
    {
		String		pattern;
		
		searchWords.Start_Get();
		while ((ww = (WeightWord *) searchWords.Get_Next()))
		{
			if (pattern.length())
				pattern.append(" ");
			pattern.append(ww->word);
		}
		wmatch.Pattern(pattern);
    }

    //
    // Open the index which will translate document id numbers into URLs.
    //
    doc_index->OpenRead(indexfile);
    docdb.Read(docfile);

    char	*id;
    results->Start_Get();
    while ((id = results->Get_Next()))
    {
		//
		// Extract document information from the database
		//
		if (doc_index->Get(id, url) == NOTOK)
		{
			continue;
		}

		//
		// Check if the URL is within the limits set up for the
		// documents to search through.
		//
		if (limit_to.FindFirst(url) < 0)
		{
			//
			// Not in the limits.  Do not include this document in the results.
			//
			continue;
		}

		//
		// Record the match in our all_matches list.
		//
		ResultMatch	*m = new ResultMatch;
		m->setURL(url);
		all_matches.Add(m);

		//
		// Compute the score for this document.  If an excerpt is
		// available, we want to look through it and see if the words
		// ever occur right next to each other.
		//
		m->setRef(docdb[m->getURL()]);
		compute_score(m, results->find(id), wmatch);
    }

    //
    // If all the matches we found were filtered out, behave the same
    // as the nothing found case
    //
    if (all_matches.Count() == 0)
    {
		nothing_found(raw_output, &vars);
		return;
    }

	//
	// Now that we know the number of matches, it can be used as
	// a variable for the header and footer output.
	//
    vars.Add("MATCHES", new String(form("%d", all_matches.Count())));

    //
    // The List all_matches now contains all the matched documents.
    // We will sort them by score.
    //
    sort(all_matches);

	if (!raw_output)
		dump_text(config["search_results_header"], vars);

    //
    // Finally we can display the results
    //
    DocumentRef		*ref;
    ResultMatch		*match = (ResultMatch *) all_matches[0];
    int				maxscore = match->getScore();
    int				n = 0;
    all_matches.Start_Get();
    while ((match = (ResultMatch *) all_matches.Get_Next()) && n < n_matches)
    {
		ref = match->getRef();
		ref->DocAnchor(match->getAnchor());
		ref->DocScore(match->getScore());
		if (raw_output)
			display_raw_match(*ref, maxscore, n + 1);
		else
		{
			if (do_short)
				display_short_match(*ref, maxscore, searchWords);
			else
				display_match(*ref, maxscore, searchWords, wmatch);
		}
		n++;
    }

	if (!raw_output)
		dump_text(config["search_results_footer"], vars);
}


//*****************************************************************************
// void compute_score(ResultMatch *m, DocMatch *dm, StringMatch &match)
//   The ResultMatch object will be updated with the score for the
//   current search The StringMatch parameter contains the pattern for
//   the concatenated words
//
void compute_score(ResultMatch *m, DocMatch *dm, StringMatch &match)
{
    int				score = 0;
    WordReference	*wref;
    DocumentRef		*ref = m->getRef();

#if 0
	//
	// This code is bogus.  This will have to be done some other way
	//
	
    //
    // Compute score for cases where two or more words occur in sequence
    //
    if (wordlist->Count() > 1 && ref && ref->DocHead() && *ref->DocHead())
    {
		int		which = 0;
		int		length = 0;
		int		offset = 0;
		int		location = 0;
		int		headlength = strlen(ref->DocHead());
				
		while ((offset = match.FindFirst(ref->DocHead() + location,
										 which,
										 length)) >= 0 &&
			   location < headlength)
		{
			location += offset + length;
			score += 1000 - (1000 * location / headlength);
		}
		if (match.FindFirst(ref->DocTitle(), which, length) >= 0)
		{
			//
			// If the words are in the title, increase the score by
			// the title factor
			//
			score += (int)(1000 * config.Double("title_factor"));
		}
    }
#endif
	
    //
    // Add in the score we already got for the word...
    //
	score += (int) dm->score;

    m->setAnchor(dm->anchor);
    m->setIncompleteScore(score);
}


//*****************************************************************************
// void display_stars(int nStars, int maxStars, int useBlanks, char *url)
//
void display_stars(int nStars, int maxStars, int useBlanks, char *url)
{
#if 0
	int		i;
	char	*image = config["star_image"];
	char	*blank = config["star_blank"];

	for (i = 1; useBlanks && i < maxStars - nStars; i++)
	{
		cout << "<img src=" << blank << " alt=\" \">";
	}

	int		match = 0;
	int		length = 0;
	int		status = URLimage.FindFirst(url, match, length);

	if (status >= 0 && match >= 0)
	{
		image = ((String*) URLimageList[match])->get();
	}
	
	cout << "<img src=" << image << " alt=\"*\">";
	for (i = 0; i < nStars; i++)
		cout << "<img src=" << image << " alt=\"*\">";
	cout << "\n";
#endif
}


//*****************************************************************************
// void display_short_match(Document &doc, int maxscore, List &searchWords)
//   Output HTML for one match.  This will display the fields that
//   were specified in the configuration file.
//
void display_short_match(DocumentRef &doc, int maxscore, List &searchWords)
{
    double	score = doc.DocScore() / (double)maxscore;
    int		block = int(score * (config.Value("max_stars") - 1) + 0.5);
    int		used_description = 0;

    if (config.Boolean("use_star_image"))
    {
		display_stars(block, config.Value("max_stars"), 1, doc.DocURL());
    }

    cout << " <a href=\"" << doc.DocURL() << "\"><b>";
    if (doc.DocTitle() && doc.DocTitle()[0])
		cout << hilight(doc.DocTitle(), searchWords);
    else
    {
		if (doc.Descriptions()->Count())
		{
			String	*str = (String *) (*doc.Descriptions())[0];
			cout << str->get();
			used_description = 1;
		}
		else
			cout << doc.DocURL();
    }
    cout << "</b></a><br>\n";
}


//*****************************************************************************
// void display_raw_match(Document &doc, int maxscore, int match_number)
//   Output raw information for this match.
//
void display_raw_match(DocumentRef &doc, int maxscore, int match_number)
{
    double	score = doc.DocScore() / (double)maxscore;
	int		i;
	
	cout << "\n";		// Blank line separates matches
	cout << form("[match %d]\n", match_number);
	cout << "title: " << doc.DocTitle() << endl;
	cout << "URL: " << doc.DocURL() << endl;
	cout << "accessed: " << doc.DocAccessed() << endl;
	cout << "modified: " << doc.DocTime() << endl;
	cout << "size: " << doc.DocSize() << endl;
	cout << "image_size: " << doc.DocImageSize() << endl;
	cout << "score: " << form("%.4f", score) << endl;
	cout << "hop_count: " << doc.DocHopCount() << endl;
	cout << "excerpt: " << doc.DocHead() << endl;
	List	*d = doc.Descriptions();
	for (i = 0; i < d->Count(); i++)
		cout << "description: " << (String *)((*d)[i]) << endl;
}


//*****************************************************************************
// void display_match(Document &doc, int maxscore, List &searchWords, StringMatch &match)
//   Output HTML for one match.  This will display the fields that
//   were specified in the configuration file.
//
void display_match(DocumentRef &doc, int maxscore, List &searchWords, StringMatch &match)
{
    double	score = doc.DocScore() / (double)maxscore;
    int		block = int(score * (config.Value("max_stars") - 1) + 0.5);
    int		used_description = 0;

    cout << "<dl compact><dt>";

    cout << " <a href=\"" << doc.DocURL() << "\"><b>";
    if (doc.DocTitle() && doc.DocTitle()[0])
		cout << hilight(doc.DocTitle(), searchWords);
    else
    {
		if (doc.Descriptions()->Count())
		{
			String	*str = (String *) (*doc.Descriptions())[0];
			cout << str->get();
			used_description = 1;
		}
		else
			cout << doc.DocURL();
    }
    cout << "</b></a>\n";
    if (config.Boolean("use_star_image"))
    {
		display_stars(block, 0, 0, doc.DocURL());
    }

    //
    // Display the fields that were requested
    //
    cout << "</dt>\n";
    char	*field;
    for (int fieldIndex = 0; fieldIndex < fields.Count(); fieldIndex++)
    {
		field = fields[fieldIndex];
		if (mystrcasecmp(field, "descriptions") == 0)
		{
			if (doc.Descriptions()->Count() > used_description)
			{
				cout << "<dt><strong>";
				cout << config["descriptions_heading_text"];
				cout << "</strong></dt>\n";
				String	*str;
				List	*list = doc.Descriptions();
				int		n = doc.Descriptions()->Count();
				for (int i = used_description; i < n; i++)
				{
					str = (String* ) (*list)[i];
					cout << "<dd>" << str->get() << "</dd>\n";
				}
			}
		}
		else if (mystrcasecmp(field, "wordlist") == 0)
		{
			if (doc.DocAnchor() > 0)
			{
				String	url = doc.DocURL();
				List	*list = doc.DocAnchors();
				String	*str = (String *) (*list)[doc.DocAnchor() - 1];
				if (str)
				{
					url << '#' << str->get();
					cout << "<dt><strong>" << config["wordlist_heading_text"];
					cout << "</strong></dt><dd>";
				}
			}
		}
		else if (mystrcasecmp(field, "excerpt") == 0)
		{
			if (doc.DocAnchor() > 0 && doc.DocAnchors()->Count())
			{
				String	url = doc.DocURL();
				List	*list = doc.DocAnchors();
				String	*str = (String *) (*list)[doc.DocAnchor() - 1];
				if (str)
				{
					url << '#' << str->get();
					excerpt(doc, searchWords, url, match);
				}
			}
			else
			{
				excerpt(doc, searchWords, 0, match);
			}
		}
		else if (mystrcasecmp(field, "modified") == 0)
		{
			if (doc.DocTime() > 1000000)
			{
				time_t		t = doc.DocTime();
				struct tm	*tm = localtime(&t);
				char	buffer[100];
				strftime(buffer, sizeof(buffer), "%e-%h-%Y", tm);

				cout << "<dt><strong>" << config["modified_heading_text"];
				cout << "</strong></dt><dd>" << buffer << "</dd>\n";
			}
		}
		else if (mystrcasecmp(field, "accessed") == 0)
		{
			if (doc.DocAccessed() > 1000000)
			{
				time_t		t = doc.DocAccessed();
				struct tm	*tm = localtime(&t);
				char	buffer[100];
				strftime(buffer, sizeof(buffer), "%e-%h-%Y", tm);

				cout << "<dt><strong>" << config["accessed_heading_text"];
				cout << "</strong></dt><dd>" << buffer << "</dd>\n";
			}
		}
		else if (mystrcasecmp(field, "size") == 0)
		{
			cout << "<dt><strong>" << config["size_heading_text"];
			cout << "</strong></dt><dd>" << doc.DocSize() << " bytes</dd>\n";
		}
		else if (mystrcasecmp(field, "datesize") == 0)
		{
			if (doc.DocTime() > 1000000)
			{
				time_t		t = doc.DocTime();
				struct tm	*tm = localtime(&t);
				char	buffer[100];
				strftime(buffer, sizeof(buffer), "%e-%h-%Y", tm);

				cout << "<dt><strong>" << config["datesize_heading_text"];
				cout << "</strong></dt><dd>" << buffer << ", "
					 << doc.DocSize() << " bytes</dd>\n";
			}
			else
			{
				cout << "<dt><strong>" << config["size_heading_text"];
				cout << "</strong></dt><dd>" << doc.DocSize()
					<< " bytes</dd>\n";
			}
		}
		else if (mystrcasecmp(field, "score") == 0)
		{
			cout << "<dt><strong>" << config["score_heading_text"];
			cout << "</strong></dt><dd>" << form("%5.4f", score) << "</dd>\n";
		}
		else if (mystrcasecmp(field, "url") == 0)
		{
			cout << "<dt><strong>" << config["url_heading_text"];
			cout << "</strong></dt><dd>" << doc.DocURL() << "</dd>\n";
		}
		else if (mystrcasecmp(field, "docid") == 0)
		{
			cout << "<dt><strong>docid:</strong></dt><dd>" << doc.DocID()
				<< "</dd>\n";
		}
		else
		{
			//
			// Special case to add arbitrary text to search results
			//
			if (config[field])
			{
				cout << "<dt><strong>" << field << "</strong></dt>\n";
				cout << "<dd>" << config[field] << "</dd>\n";
			}
		}
    }

    cout << "</dl>\n\n";
}


//*****************************************************************************
// void excerpt(DocumentRef &doc, List &searchWords, char *url, StringMatch &match)
//
void excerpt(DocumentRef &doc, List &searchWords, char *url, StringMatch &match)
{
    String	head = doc.DocHead();
    int		which, length;
    int		first = wm.FindFirstWord(head, which, length);
    char	*temp = head;
    String	part;

    if (searchWords.Count() > 1)
    {
		int		t;
		t = match.FindFirst(head, which, length);
		if (t >= 0)
			first = t;
    }
	if (config.Boolean("excerpt_show_top", 0))
	{
		first = 0;
	}
	
    if (first < 0)
    {
		if (config["no_excerpt_text"][0])
		{
			cout << "<dt><strong>" << config["excerpt_heading_text"];
			cout << "</strong></dt><dd>";
			cout << config["no_excerpt_text"] << "\n";
		}
    }
    else
    {
		cout << "<dt><strong>" << config["excerpt_heading_text"];
		cout << "</strong></dt><dd>";
		if (!config.Boolean("add_anchors_to_excerpt"))
			url = 0;

		int		length = config.Value("excerpt_length", 50);
		char	*start = &temp[first] - length / 2;
		if (start < temp)
			start = temp;
		else
		{
			cout << config["start_elipses"];
			while (*start && isalpha(*start))
				start++;
		}
		char	*end = start + length;
		if (end > temp + head.length())
		{
			end = temp + head.length();
			cout << hilight(start, searchWords, url);
		}
		else
		{
			while (*end && isalpha(*end))
				end++;
			*end = '\0';
			cout << hilight(start, searchWords, url);
			cout << config["end_elipses"];
		}
    }
    cout << "</dd>\n";
}


