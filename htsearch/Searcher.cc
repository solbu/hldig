//
// Searcher.cc
//
// Searcher: Retrieves and scores the document list
//
// Combines former functionality from parser.cc and
//  the searching functions of htsearch.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Searcher.cc,v 1.1 1999/10/15 03:38:14 jtillman Exp $
//

#if RELEASE
static char RCSid[] = "$Id";
#endif

#include "Searcher.h"
#include "HtPack.h"

#define	WORD	1000
#define	DONE	1001


//*****************************************************************************
Searcher::Searcher(): words(config)
{
	tokens = 0;
  result = 0;
  current = 0;
  dbf = 0;
  valid = 1;
  minimum_word_length = 3;
  config.Defaults(&defaults[0]);
}


//*****************************************************************************
// int Searcher::checkSyntax(List *tokenList, Database *dbf)
//   As the name of the function implies, we will only perform a syntax check
//   on the list of tokens.
//
int
Searcher::checkSyntax(List *tokenList)
{
	tokens = tokenList;
	valid = 1;
	fullexpr(0);
	return valid;
}

//*****************************************************************************
void
Searcher::fullexpr(int output)
{
	tokens->Start_Get();
	lookahead = lexan();
	expr(output);
	if (valid && lookahead != DONE)
	{
		setError("end of expression");
	}
}

//*****************************************************************************
int
Searcher::lexan()
{
	current = (WeightWord *) tokens->Get_Next();
	if (!current)
		return DONE;
	else if (mystrcasecmp((char*)current->word, "&") == 0)
		return '&';
	else if (mystrcasecmp((char*)current->word, "|") == 0)
		return '|';
	else if (mystrcasecmp((char*)current->word, "!") == 0)
		return '!';
	else if (mystrcasecmp((char*)current->word, "(") == 0)
		return '(';
	else if (mystrcasecmp((char*)current->word, ")") == 0)
		return ')';
	else if (mystrcasecmp((char*)current->word, "\"") == 0)
		return '"';
	else
		return WORD;
}


//*****************************************************************************
//   Attempt to deal with expressions in the form
//		term | term | term ...
//
void
Searcher::expr(int output)
{
	term(output);
	while (1)
	{
		if (match('|'))
		{
			term(output);
	  	if (output)
				perform_or();
		}
		else
	  	break;
	}
	if (valid && lookahead == WORD)
		setError("'AND' or 'OR'");
}


//*****************************************************************************
void
Searcher::term(int output)
{
    int	isand;

    factor(output);
    while (1)
    {
	if ((isand = match('&')) || match('!'))
	{
	    factor(output);
	    if (output)
	    {
		perform_and(isand);
	    }
	}
	else
	    break;
    }
}

//*****************************************************************************
void
Searcher::factor(int output)
{
    phrase(output);

    if (match('('))
    {
	expr(output);
	if (match(')'))
	{
	    return;
	}
	else
	{
	    setError("')'");
	}
    }
    else if (lookahead == WORD)
    {
	if (output)
	{
	    perform_push();
	}
	lookahead = lexan();
    }
    //    else
    //    {
    //	setError("a search word");
    //    }
}

//*****************************************************************************
void
Searcher::phrase(int output)
{
  if (match('"'))
    {
      List *wordList = new List;
      double weight = 1.0;

      while (1)
	{
	  if (match('"'))
	    {
	      if (output)
		score(wordList, weight);
	      break;
	    }
	  else if (lookahead == WORD)
	    {
	      weight *= current->weight;
	      if (output)
		perform_phrase(*wordList);
	
	      lookahead = lexan();
	    }

	} // end while
      delete wordList;
    } // end if
}

//*****************************************************************************
int
Searcher::match(int t)
{
    if (lookahead == t)
    {
	lookahead = lexan();
	return 1;
    }
    else
	return 0;
}

//*****************************************************************************
void
Searcher::setError(char *expected)
{
    if (valid)
    {
	valid = 0;
	error = 0;
	error << "Expected " << expected;
	if (lookahead == DONE || !current)
	{
	    error << " at the end";
	}
	else
	{
	    error << " instead of '" << current->word.get();
	    error << '\'';
	    switch (lookahead)
	    {
	    case '&':	error << " or 'AND'";	break;
	    case '|':	error << " or 'OR'";	break;
	    case '!':	error << " or 'NOT'";	break;
	    }
	}
    }
}

//*****************************************************************************
// Perform a lookup of the current word and push the result onto the stack
//
void
Searcher::perform_push()
{
    static int	maximum_word_length = config.Value("maximum_word_length", 12);
    String	temp = current->word.get();
    char	*p;

    if (current->isIgnore)
    {
	//
	// This word needs to be ignored.  Make it so.
	//
	return;
    }

    temp.lowercase();
    p = temp.get();
    if (temp.length() > maximum_word_length)
	p[maximum_word_length] = '\0';

    List* result = words[p];
    score(result, current->weight);
    delete result;
}

//*****************************************************************************
void
Searcher::perform_phrase(List &oldWords)
{
    static int	maximum_word_length = config.Value("maximum_word_length", 12);
    String	temp = current->word.get();
    char	*p;
    List	*newWords = 0;
    HtWordReference *oldWord, *newWord;

    if (current->isIgnore)
    {
	//
	// This word needs to be ignored.  Make it so.
	//
	return;
    }

    temp.lowercase();
    p = temp.get();
    if (temp.length() > maximum_word_length)
	p[maximum_word_length] = '\0';

    newWords = words[p];

    // If we don't have a prior list of words, we want this one...
    if (oldWords.Count() == 0)
      {
	newWords->Start_Get();
	while ((newWord = (HtWordReference *) newWords->Get_Next()))
	  oldWords.Add(newWord);
	return;
      }

    // OK, now we have a previous list in wordList and a new list
    List	*results = new List;

    oldWords.Start_Get();
    while ((oldWord = (HtWordReference *) oldWords.Get_Next()))
      {
	newWords->Start_Get();
	while ((newWord = (HtWordReference *) newWords->Get_Next()))
	  {
	    if (oldWord->DocID() == newWord->DocID())
	      if ((oldWord->Location() + 1) == newWord->Location())
		{
		  HtWordReference *result = new HtWordReference(*oldWord);

		  result->Flags(oldWord->Flags() & newWord->Flags());
		  result->Location(newWord->Location());
		
		  results->Add(result);
		}
	  }
      }

    oldWords.Destroy();
    results->Start_Get();
    while ((newWord = (HtWordReference *) results->Get_Next()))
      oldWords.Add(newWord);
    results->Release();
    delete results;

    newWords->Destroy();
    delete newWords;
}

//*****************************************************************************
void
Searcher::score(List *wordList, double weight)
{
    ResultList	*list = new ResultList();
    DocMatch	*dm;
    HtWordReference *wr;

    stack.push(list);

    if (!wordList || wordList->Count() == 0)
      {
	// We can't score an empty list, so this should be ignored...
	list->isIgnore = 1;
	return;
      }

    wordList->Start_Get();
    while ((wr = (HtWordReference *) wordList->Get_Next()))
      {
	dm = list->find(wr->DocID());
	if (dm)
	  {

	    unsigned int prevAnchor;
	    double prevScore;
	    prevScore = dm->score;
	    prevAnchor = dm->anchor;
	    // We wish to *update* this, not add a duplicate
	    list->remove(wr->DocID());

	    dm = new DocMatch;

	    dm->score = (wr->Flags() & FLAG_TEXT) * config.Value("text_factor", 1);
	    dm->score += (wr->Flags() & FLAG_CAPITAL) * config.Value("caps_factor", 1);
	    dm->score += (wr->Flags() & FLAG_TITLE) * config.Value("title_factor", 1);
	    dm->score += (wr->Flags() & FLAG_HEADING) * config.Value("heading_factor", 1);
	    dm->score += (wr->Flags() & FLAG_KEYWORDS) * config.Value("keywords_factor", 1);
	    dm->score += (wr->Flags() & FLAG_DESCRIPTION) * config.Value("meta_description_factor", 1);
	    dm->score += (wr->Flags() & FLAG_AUTHOR) * config.Value("author_factor", 1);
	    dm->score += (wr->Flags() & FLAG_LINK_TEXT) * config.Value("description_factor", 1);
	    dm->id = wr->DocID();
	    dm->score = weight * dm->score + prevScore;
	    if (prevAnchor > wr->Anchor())
	      dm->anchor = wr->Anchor();
	    else
	      dm->anchor = prevAnchor;
	
	  }
	else
	  {

	    //
	    // *******  Compute the score for the document
	    //
	    dm = new DocMatch;
	    dm->score = (wr->Flags() & FLAG_TEXT) * config.Value("text_factor", 1);
	    dm->score += (wr->Flags() & FLAG_CAPITAL) * config.Value("caps_factor", 1);
	    dm->score += (wr->Flags() & FLAG_TITLE) * config.Value("title_factor", 1);
	    dm->score += (wr->Flags() & FLAG_HEADING) * config.Value("heading_factor", 1);
	    dm->score += (wr->Flags() & FLAG_KEYWORDS) * config.Value("keywords_factor", 1);
	    dm->score += (wr->Flags() & FLAG_DESCRIPTION) * config.Value("meta_description_factor", 1);
	    dm->score += (wr->Flags() & FLAG_AUTHOR) * config.Value("author_factor", 1);
	    dm->score += (wr->Flags() & FLAG_LINK_TEXT) * config.Value("description_factor", 1);
	    dm->score *= weight;
	    dm->id = wr->DocID();
	    dm->anchor = wr->Anchor();
	  }
	list->add(dm);
      }
}


//*****************************************************************************
// The top two entries in the stack need to be ANDed together.
//
void
Searcher::perform_and(int isand)
{
    ResultList		*l1 = (ResultList *) stack.pop();
    ResultList		*l2 = (ResultList *) stack.pop();
    ResultList		*result = new ResultList;
    int			i;
    DocMatch		*dm, *dm2, *dm3;
    HtVector		*elements;

    //
    // If either of the arguments is not present, we will use the other as
    // the result.
    //
    if (!l1 && l2)
    {
	stack.push(l2);
	return;
    }
    else if (l1 && !l2)
    {
	stack.push(l1);
	return;
    }
    else if (!l1 && !l2)
    {
	stack.push(result);
	return;
    }

    //
    // If either of the arguments is set to be ignored, we will use the
    // other as the result.
    //
    if (l1->isIgnore)
    {
	stack.push(l2);
	delete l1;
	return;
    }
    else if (l2->isIgnore)
    {
	stack.push(isand ? l1 : result);
	delete l2;
	return;
    }

    stack.push(result);
    elements = l2->elements();
    for (i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	dm2 = l1->find(dm->id);
	if (dm2 ? isand : (isand == 0))
	{
	    //
	    // Duplicate document.  We just need to add the scored together.
	    //
	    dm3 = new DocMatch;
	    dm3->score = dm->score + (dm2 ? dm2->score : 0);
	    dm3->id = dm->id;
	    dm3->anchor = dm->anchor;
	    if (dm2 && dm2->anchor < dm3->anchor)
		dm3->anchor = dm2->anchor;
	    result->add(dm3);
	}
    }
    elements->Release();
    delete elements;
    delete l1;
    delete l2;
}

//*****************************************************************************
// The top two entries in the stack need to be ORed together.
//
void
Searcher::perform_or()
{
    ResultList		*l1 = (ResultList *) stack.pop();
    ResultList		*result = (ResultList *) stack.peek();
    int			i;
    DocMatch		*dm, *dm2;
    HtVector		*elements;

    //
    // If either of the arguments is not present, we will use the other as
    // the results.
    //
    if (!l1 && result)
    {
	return;
    }
    else if (l1 && !result)
    {
	stack.push(l1);
	return;
    }
    else if (!l1 & !result)
    {
	stack.push(new ResultList);
	return;
    }

    //
    // If either of the arguments is set to be ignored, we will use the
    // other as the result.
    //
    if (l1->isIgnore)
    {
	delete l1;
	return;
    }
    else if (result->isIgnore)
    {
	result = (ResultList *) stack.pop();
	stack.push(l1);
	delete result;
	return;
    }

    elements = l1->elements();
    for (i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	dm2 = result->find(dm->id);
	if (dm2)
	{
	    //
	    // Duplicate document.  We just need to add the scores together
	    //
	    dm2->score += dm->score;
	    if (dm->anchor < dm2->anchor)
		dm2->anchor = dm->anchor;
	}
	else
	{
	    dm2 = new DocMatch;
	    dm2->score = dm->score;
	    dm2->id = dm->id;
	    dm2->anchor = dm->anchor;
	    result->add(dm2);
	}
    }
    elements->Release();
    delete elements;
    delete l1;
}


//*****************************************************************************
// void Searcher::parse(List *tokenList, ResultList &resultMatches)
// void Searcher::collate(List *tokenList, ResultList &resultMatches)
//
void
Searcher::collate(ResultList &resultMatches)
{
    tokens = &searchWords;
    fullexpr(1);

    ResultList	*result = (ResultList *) stack.pop();
    if (!result)  // Ouch!
      {
	valid = 0;
	error = 0;
	error << "Expected to have something to search for!";
	return;
      }
    HtVector		*elements = result->elements();
    DocMatch	*dm;

    for (int i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	resultMatches.add(dm);
    }
    elements->Release();
    result->Release();
    delete elements;
    delete result;
}


//END OF ROUTINES TAKEN FROM PARSER


//ROUTINES TAKEN FROM HTSEARCH


//*****************************************************************************
// Dictionary *htsearch(char *wordfile, List &searchWords, Parser *parser)
//   This returns a dictionary indexed by document ID and containing a
//   List of HtWordReference objects.
//
ResultList *
Searcher::execute()
{

	//init HT stuff
//	HtWordType::Initialize(config);
    WordType::Initialize(config);
	
	//we have to this here for now,
	// later, the parser will do it for us
	setupWords(originalWords, 0);
	createLogicalWords();



	//perform access checks
    const String	word_db = config["word_db"];

   if (debug)
		cerr << "Word db is " << config["word_db"] << endl;

    if (access(word_db, R_OK) < 0)
    {
			valid = 0;
    	error << "Unable to read word database file " << word_db.get() << "'\nDid you run htmerge?";
			return 0;
    }

  String	index = config["doc_index"];
  if (access(index, R_OK) < 0)
  {
		valid = 0;
		error << "Unable to read document index file '" << index.get() << "'\nDid you run htmerge?";
		return 0;
  }
  String doc_db = config["doc_db"];
	if (access(doc_db, R_OK) < 0)
  {
		error << "Unable to read document database file '" << doc_db.get() << "'\nDid you run htmerge?";
		return 0;
	}

  String doc_excerpt = config ["doc_excerpt"];
	if (access(doc_excerpt, R_OK) < 0)
  {
		error << "Unable to read document excerpts '" << doc_excerpt.get() << "'\nDid you run htmerge?";
		return 0;
	}
	
	ResultList *matches = new ResultList(config ["doc_db"], 0, config ["doc_excerpt"]);

  if (searchWords.Count() > 0)
  {
		setDatabase(word_db);
		collate(*matches);
  }
  return matches;
}



//UTILITY FUNCTIONS

//*****************************************************************************
void
Searcher::createLogicalWords()
{
    String		pattern;
    int			i;
    int			wasHidden = 0;
    int			inPhrase = 0;

    for (i = 0; i < searchWords.Count(); i++)
    {
	WeightWord	*ww = (WeightWord *) searchWords[i];
	if (!ww->isHidden)
	{
	    if (strcmp((char*)ww->word, "&") == 0 && wasHidden == 0)
		logicalWords << " and ";
	    else if (strcmp((char*)ww->word, "|") == 0 && wasHidden == 0)
		logicalWords << " or ";
	    else if (strcmp((char*)ww->word, "!") == 0 && wasHidden == 0)
		logicalWords << " not ";
	    else if (strcmp((char*)ww->word, "\"") == 0 && wasHidden == 0)
	      {
		if (inPhrase)
		  logicalWords.chop(' ');
		inPhrase = !inPhrase;
		logicalWords << "\"";
	      }
	    else if (wasHidden == 0)
	    {
	      logicalWords << ww->word;
	      if (inPhrase)
		logicalWords << " ";
	    }
	    wasHidden = 0;
	}
	else
	    wasHidden = 1;
	if (ww->weight > 0			// Ignore boolean syntax stuff
	    && !ww->isIgnore)			// Ignore short or bad words
	{
	    if (pattern.length() && !inPhrase)
		pattern << '|';
	    else if (pattern.length() && inPhrase)
	      pattern << ' ';
	    pattern << ww->word;
	}
    }
    logicalPattern = pattern;

    if (debug)
    {
	cerr << "LogicalWords: " << logicalWords << endl;
	cerr << "Pattern: " << pattern << endl;
    }
}





void
Searcher::dumpWords(List &words, char *msg = "")
{
	
    if (debug)
    {
	cerr << msg << ": '";
	for (int i = 0; i < words.Count(); i++)
	{
	    WeightWord	*ww = (WeightWord *) words[i];
	    cerr << ww->word << ':' << ww->isHidden << ' ';
	}
	cerr << "'\n";
    }
}





//*****************************************************************************
// void setupWords(char *allWords, List &searchWords,
//		   int boolean, Parser *parser, String &originalPattern)
//
void
Searcher::setupWords(char *allWords, int boolean)
{
    List	tempWords;
    int		i;

    //
    // Parse the words we need to search for.  It should be a list of words
    // with optional 'and' and 'or' between them.  The list of words
    // will be put in the searchWords list and at the same time in the
    // String pattern separated with '|'.
    //

    //
    // Convert the string to a list of WeightWord objects.  The special
    // characters '(' and ')' will be put into their own WeightWord objects.
    //
    unsigned char	*pos = (unsigned char*) allWords;
    unsigned char	t;
    String		word;
    // Why use a char type if String is the new char type!!!
    const String	prefix_suffix = config["prefix_match_character"];
    while (*pos)
    {
	while (1)
	{
	    t = *pos++;
	    if (isspace(t))
	    {
		continue;
	    }
	    else if (t == '"')
	      {
		tempWords.Add(new WeightWord("\"", -1.0));
		break;
	      }
	    else if (boolean && (t == '(' || t == ')'))
	    {
		char	s[2];
		s[0] = t;
		s[1] = '\0';
		tempWords.Add(new WeightWord(s, -1.0));
		break;
	    }
	    else if (HtIsWordChar(t) || t == ':' ||
			 (strchr(prefix_suffix, t) != NULL) || (t >= 161 && t <= 255))
	    {
		word = 0;
		while (t && (HtIsWordChar(t) ||
			     t == ':' || (strchr(prefix_suffix, t) != NULL) || (t >= 161 && t <= 255)))
		{
		    word << (char) t;
		    t = *pos++;
		}

		pos--;
		if (boolean && mystrcasecmp(word.get(), "and") == 0)
		{
		    tempWords.Add(new WeightWord("&", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), "or") == 0)
		{
		    tempWords.Add(new WeightWord("|", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), "not") == 0)
		{
		    tempWords.Add(new WeightWord("!", -1.0));
		}
		else if (boolean && mystrcasecmp(word.get(), "+") == 0)
		  tempWords.Add(new WeightWord("&", -1.0));
		else if (boolean && mystrcasecmp(word.get(), "-") == 0)
		  tempWords.Add(new WeightWord("!", -1.0));
		else
		{
		    // Add word to excerpt matching list
		    originalWords << word << "|";
		    WeightWord	*ww = new WeightWord(word, 1.0);
		    if(HtWordNormalize(word) & WORD_NORMALIZE_NOTOK)
			ww->isIgnore = 1;
		    tempWords.Add(ww);
		}
		break;
	    }
	}
    }

    dumpWords(tempWords, "tempWords");
	
    //
    // If the user specified boolean expression operators, the whole
    // expression has to be syntactically correct.  If not, we need
    // to report a syntax error.
    //
    if (boolean)
    {
	if (!checkSyntax(&tempWords))
	{
	    for (i = 0; i < tempWords.Count(); i++)
	    {
		searchWords.Add(tempWords[i]);
	    }
	    tempWords.Release();
	    return;
//			reportError("Syntax error");
	}
    }
    else
    {
	convertToBoolean(tempWords);
    }
	
    dumpWords(tempWords, "Boolean");
	
    //
    // We need to assign weights to the words according to the search_algorithm
    // configuration attribute.
    // For algorithms other than exact, we need to also do word lookups.
    //
    StringList	algs(config["search_algorithm"], " \t,");
    List		algorithms;
    String		name, weight;
    double		fweight;
    Fuzzy		*fuzzy = 0;

    //
    // Generate the list of algorithms to use and associate the given
    // weights with them.
    //
    for (i = 0; i < algs.Count(); i++)
    {
	name = strtok(algs[i], ":");
	weight = strtok(0, ":");
	if (name.length() == 0)
	    name = "exact";
	if (weight.length() == 0)
	    weight = "1";
	fweight = atof((char*)weight);

	fuzzy = Fuzzy::getFuzzyByName(name, config);
	if (fuzzy)
	{
	    fuzzy->setWeight(fweight);
	    fuzzy->openIndex();
	    algorithms.Add(fuzzy);
	}
    }

    dumpWords(searchWords, "initial");
	
    //
    // For each of the words, apply all the algorithms.
    //
    for (i = 0; i < tempWords.Count(); i++)
    {
	WeightWord	*ww = (WeightWord *) tempWords[i];
	if (ww->weight > 0 && !ww->isIgnore)
	{
	    //
	    // Apply all the algorithms to the word.
	    //
	    if (debug)
	      cerr << "Fuzzy on: " << ww->word << endl;
	    doFuzzy(ww, searchWords, algorithms);
	    delete ww;
	}
	else
	{
	    //
	    // This is '(', ')', '&', or '|'.  These will be automatically
	    // transfered to the searchWords list.
	    //
	    if (debug)
		cerr << "Add: " << ww->word << endl;
	    searchWords.Add(ww);
	}
	dumpWords(searchWords, "searchWords");
    }
    tempWords.Release();
}




//*****************************************************************************
void
Searcher::doFuzzy(WeightWord *ww, List &searchWords, List &algorithms)
{
    List		fuzzyWords;
    List		weightWords;
    Fuzzy		*fuzzy;
    WeightWord	*newWw;
    String		*word;

    algorithms.Start_Get();
    while ((fuzzy = (Fuzzy *) algorithms.Get_Next()))
    {
        if (debug > 1)
	  cout << "   " << fuzzy->getName();
	fuzzy->getWords(ww->word, fuzzyWords);
	fuzzyWords.Start_Get();
	while ((word = (String *) fuzzyWords.Get_Next()))
	{
	    if (debug > 1)
	      cout << " " << word->get();
	    newWw = new WeightWord(word->get(), fuzzy->getWeight());
	    newWw->isExact = ww->isExact;
	    newWw->isHidden = ww->isHidden;
	    weightWords.Add(newWw);
	}
	if (debug > 1)
	  cout << endl;
	fuzzyWords.Destroy();
    }

    //
    // We now have a list of substitute words.  They need to be added
    // to the searchWords.
    //
    if (weightWords.Count())
    {
	if (weightWords.Count() > 1)
	    searchWords.Add(new WeightWord("(", -1.0));
	for (int i = 0; i < weightWords.Count(); i++)
	{
	    if (i > 0)
		searchWords.Add(new WeightWord("|", -1.0));
	    searchWords.Add(weightWords[i]);
	}
	if (weightWords.Count() > 1)
	    searchWords.Add(new WeightWord(")", -1.0));
    }
    weightWords.Release();
}

//*****************************************************************************
// void convertToBoolean(List &words)
//
void
Searcher::convertToBoolean(List &words)
{
    List	list;
    int		i;
    int		do_and = strcmp(config["match_method"], "and") == 0;
    int		in_phrase = 0;

    String	quote = "\"";

    if (words.Count() == 0)
	return;
    list.Add(words[0]);

    // We might start off with a phrase match
    if (((WeightWord *) words[0])->word == quote)
	in_phrase = 1;

    for (i = 1; i < words.Count(); i++)
    {
	if (do_and && !in_phrase)
	    list.Add(new WeightWord("&", -1.0));
	else if (!in_phrase)
	    list.Add(new WeightWord("|", -1.0));
	
	if (((WeightWord *) words[i])->word == quote)
	    in_phrase = !in_phrase;

	list.Add(words[i]);
    }
    words.Release();

    for (i = 0; i < list.Count(); i++)
    {
	words.Add(list[i]);
    }
    list.Release();
}

//*****************************************************************************
// Modify the search words list to include the required words as well.
// This is done by putting the existing search words in parenthesis and
// appending the required words separated with "and".

//taken from htsearch.cc

void
Searcher::addRequiredWords(List &searchWords, StringList &requiredWords)
{
    searchWords.Insert(new WeightWord("(", -1.0), 0);
    searchWords.Add(new WeightWord(")", -1.0));

    for (int i = 0; i < requiredWords.Count(); i++)
    {
	searchWords.Add(new WeightWord("&", -1.0));
	searchWords.Add(new WeightWord(requiredWords[i], 1.0));
    }
}
































