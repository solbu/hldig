//
// parser.cc
//
// parser: Parses a boolean expression tree, retrieving and scoring 
//         the resulting document list
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: parser.cc,v 1.38 2004/07/11 10:28:23 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "parser.h"
#include "HtPack.h"
#include "Collection.h"
#include "Dictionary.h"
#include "QuotedStringList.h"

#define	WORD	1000
#define	DONE	1001

QuotedStringList	boolean_syntax_errors;
enum ErrorIndices { EXPECTED, SEARCH_WORD, AT_END, INSTEAD_OF, END_OF_EXPR, QUOTE };

//*****************************************************************************
Parser::Parser() :
  words(*(HtConfiguration::config()))
{
    tokens = 0;
    result = 0;
    current = 0;
    valid = 1;
}


//*****************************************************************************
// int Parser::checkSyntax(List *tokenList)
//   As the name of the function implies, we will only perform a syntax check
//   on the list of tokens.
//
int
Parser::checkSyntax(List *tokenList)
{
	HtConfiguration* config= HtConfiguration::config();
    void	reportError(char *);
    // Load boolean_syntax_errors from configuration
    // they should be placed in this order:
    // 0        1               2            3            4
    // Expected "a search word" "at the end" "instead of" "end of expression"
    // 5
    // "a closing quote"
    boolean_syntax_errors.Destroy();
    boolean_syntax_errors.Create(config->Find("boolean_syntax_errors"), "| \t\r\n\001");
    if (boolean_syntax_errors.Count() == 5)
    {	// for backward compatibility
	boolean_syntax_errors.Add (new String ("a closing quote"));
	if (debug)
	    cerr << "Parser::checkSyntax() : boolean_syntax_errors should have six entries\n";
    } else if (boolean_syntax_errors.Count() != 6)
	reportError("boolean_syntax_errors attribute should have six entries");
    tokens = tokenList;
    valid = 1;
    fullexpr(0);
    return valid;
}

//*****************************************************************************
/* Called by:	Parser::parse(List*, ResultList&), checkSyntax(List*) */
/* Inputs:	output -- if zero, simply check syntax */
/* 		       otherwise, list matching documents in head of "stack" */
void
Parser::fullexpr(int output)
{
    tokens->Start_Get();
    lookahead = lexan();
    expr(output);
    if (valid && lookahead != DONE)
    {
	setError(boolean_syntax_errors[END_OF_EXPR]);
    }
}

//*****************************************************************************
int
Parser::lexan()
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
/* Called by:	Parser::fullexpr(int), factor(int) */
/* Inputs:	output -- if zero, simply check syntax */
void
Parser::expr(int output)
{
    term(output);
    while (1)
    {
	if (match('|'))
	{
	    term(output);
	    if (output)
	    {
		if(debug) cerr << "or--" << endl;
		perform_or();
		if(debug) cerr << "stack:" << stack.Size() << endl;
	    }
	}
	else
	    break;
    }
    if (valid && lookahead == WORD)
    {
	String	expected = "'";
	expected << boolean_keywords[AND] << "' "<< boolean_keywords[OR] <<" '"
		 << boolean_keywords[OR] << "'";
	setError(expected.get());
    }
}

//*****************************************************************************
//   Attempt to deal with terms in the form
//		factor & factor & factor ...
/* Called by:	Parser::expr(int) */
/* Inputs:	output -- if zero, simply check syntax */
void
Parser::term(int output)
{
    
    factor(output);
	if(debug) cerr << "term:factor" << endl;
    while (1)
    {
	if(match('&'))
	{
		factor(output);
		if(output)
		{
			if(debug) cerr << "and--" << endl;
			perform_and();
			if(debug) cerr << "stack:" << stack.Size() << endl;
		}
	}
	else if(match('!'))
	{
		factor(output);
		if(output)
		{
			if(debug) cerr << "not--" << endl;
			perform_not();
			if(debug) cerr << "stack:" << stack.Size() << endl;
		}
	}
	else
	{
		break;
	}
    }
}

//*****************************************************************************
/* Gather and score a (possibly bracketed) boolean expression */
/* Called by:	Parser::term(int) */
/* Inputs:	output -- if zero, simply check syntax */
void
Parser::factor(int output)
{
    if(match('"'))
    {
	phrase(output);
    }
    else if (match('('))
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
    else
    {
    	setError(boolean_syntax_errors[SEARCH_WORD]);
//    	setError("a search word, a quoted phrase, a boolean expression between ()");
    }
}

//*****************************************************************************
/* Gather and score a quoted phrase */
/* Called by:	Parser::factor(int) */
/* Inputs:	output -- if zero, simply check syntax */
void
Parser::phrase(int output)
{
      List *wordList = 0;
      double weight = 1.0;

      while (1)
	{
	  if (match('"'))
	    {
	      if (output)
	      {
                if(!wordList) wordList = new List;
		if(debug) cerr << "scoring phrase" << endl;
		score(wordList, weight, FLAGS_MATCH_ONE); // look in all fields
	      }
	      break;
	    }
	  else if (lookahead == WORD)
	    {
	      weight *= current->weight;
	      if (output)
		perform_phrase(wordList);
	      
	      lookahead = lexan();
	    }
          else if (lookahead == DONE)
           {
	     setError(boolean_syntax_errors[QUOTE]);
	     break;
           }
	  else
	   {
	     // Be defensive: don't loop forever if unexpected lexans...
	     lookahead = lexan ();
	   }
	} // end while
	if(wordList) delete wordList;
}

//*****************************************************************************
int
Parser::match(int t)
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
Parser::setError(char *expected)
{
    if (valid)
    {
	valid = 0;
	error = 0;
	error << boolean_syntax_errors[EXPECTED] << ' ' << expected;
	if (lookahead == DONE || !current)
	{
	    error << ' ' << boolean_syntax_errors[AT_END];
	}
	else
	{
	    error << ' ' << boolean_syntax_errors[INSTEAD_OF] << " '"
		  << current->word.get() << "'";
	    switch (lookahead)
	    {
	    case '&':	error << ' ' << boolean_keywords[OR] << " '"
			      << boolean_keywords[AND] << "'";
			break;
	    case '|':	error << ' ' << boolean_keywords[OR] << " '"
			      << boolean_keywords[OR] << "'";
			break;
	    case '!':	error << ' ' << boolean_keywords[OR] << " '"
			      << boolean_keywords[NOT] << "'";
			break;
	    }
	}
	if (debug) cerr << "Syntax error: " << error << endl;
    }
}

//*****************************************************************************
// Perform a lookup of the current word and push the result onto the stack
//
void
Parser::perform_push()
{
	HtConfiguration* config= HtConfiguration::config();
    static int	maximum_word_length = config->Value("maximum_word_length", 12);
    String	temp = current->word.get();
    char	*p;

    if(debug)
	cerr << "perform_push @"<< stack.Size() << ": " << temp << endl;

    String	wildcard = config->Find("prefix_match_character");
    if (!wildcard.get())
        wildcard = "*";
    if (temp == wildcard)
    {
	if (debug) cerr << "Wild card search\n";
    	ResultList	*list = new ResultList;
	String		doc_db = config->Find("doc_db");
	DocumentDB	docdb;
	docdb.Read(doc_db);
	List		*docs = docdb.DocIDs();

	//
	// Traverse all the known documents
	//
	DocumentRef	*ref;
	IntObject	*id;
	DocMatch	*dm;
	docs->Start_Get();
	while ((id = (IntObject *) docs->Get_Next()))
	{
	    ref = docdb[id->Value()];
	    if (debug)
		cerr << (ref ? "Wildcard match" : "Wildcard empty") << endl;
	    if (ref)
	    {
		dm = new DocMatch;
		dm->score = current->weight;
		dm->id = ref->DocID();
		dm->orMatches = 1;
		dm->anchor = 0;
		list->add(dm);
	    }
	    delete ref;
	}
	delete docs;
	stack.push(list);

        return;
    }

    // Must be after wildcard: "*" is "isIgnore" because it is too short.
    if (current->isIgnore)
    {
	if(debug) cerr << "ignore: " << temp << " @" << stack.Size() << endl;
	//
	// This word needs to be ignored.  Make it so.
	//
    	ResultList	*list = new ResultList;
	list->isIgnore = 1;
    	stack.push(list);
	return;
    }

    temp.lowercase();
    p = temp.get();
    if (temp.length() > maximum_word_length)
	p[maximum_word_length] = '\0';

    List* result = words[p];
    score(result, current->weight, current->flags);
    delete result;
}

//*****************************************************************************
// BUG:  Phrases containing "bad words" can have *any* "bad word" in that
//       position.  Words less than  minimum_word_length  ignored entirely,
//       as they are not indexed.
void
Parser::perform_phrase(List * &oldWords)
{
	HtConfiguration* config= HtConfiguration::config();
    static int	maximum_word_length = config->Value("maximum_word_length", 12);
    String	temp = current->word.get();
    char	*p;
    List	*newWords = 0;
    HtWordReference *oldWord, *newWord;

    // how many words ignored since last checked word?
    static int	ignoredWords = 0;

    // if the query is empty, no further effort is needed
    if(oldWords && oldWords->Count() == 0)
    {
	if(debug) cerr << "phrase not found, skip" << endl;
	return;
    }

    if(debug) cerr << "phrase current: " << temp << endl;
    if (current->isIgnore)
    {
	//
	// This word needs to be ignored.  Make it so.
	//
	if (temp.length() >= config->Value ("minimum_word_length") && oldWords)
	    ignoredWords++;
	if(debug) cerr << "ignoring: " << temp << endl;
	return;
    }

    temp.lowercase();
    p = temp.get();
    if (temp.length() > maximum_word_length)
	p[maximum_word_length] = '\0';

    newWords = words[p];
    if(debug) cerr << "new words count: " << newWords->Count() << endl;

    // If we don't have a prior list of words, we want this one...
    if (!oldWords)
      {
	oldWords = new List;
	if(debug) cerr << "phrase adding first: " << temp << endl;
	newWords->Start_Get();
	while ((newWord = (HtWordReference *) newWords->Get_Next()))
	{
	  oldWords->Add(newWord);
	}
	if(debug) cerr << "old words count: " << oldWords->Count() << endl;
	return;
      }

    // OK, now we have a previous list in wordList and a new list
    List	*results = new List;

    Dictionary  newDict(5000);

    String nid;
    newWords->Start_Get();
    while ((newWord = (HtWordReference *) newWords->Get_Next()))
      {
	nid = "";
	int did =  newWord->DocID();
	nid << did;
	nid << "-";
	int loc = newWord->Location();
	nid << loc;
	if (! newDict.Exists(nid)) {
	   newDict.Add(nid, (Object *)newWord);
	} else {
//	   cerr << "perform_phrase: NewWords Duplicate: " << nid << "\n";
//	    Double addition is a problem if you don't want your original objects deleted
	}
      }

    String oid;
    oldWords->Start_Get();
    while ((oldWord = (HtWordReference *) oldWords->Get_Next()))
      {
	oid = "";
	int did =  oldWord->DocID();
	oid << did;
	oid << "-";
	int loc = oldWord->Location();
	oid << loc + ignoredWords+1;
	if (newDict.Exists(oid))
	  {
	    newWord = (HtWordReference *)newDict.Find(oid);

	    HtWordReference *result = new HtWordReference(*oldWord);

	    result->Flags(oldWord->Flags() & newWord->Flags());
	    result->Location(newWord->Location());
		  
	    results->Add(result);
	  }
      }
    ignoredWords = 0;	// most recent word is not a non-ignored word

    newDict.Release();

    if(debug) cerr << "old words count: " << oldWords->Count() << endl;
    if(debug) cerr << "results count: " << results->Count() << endl;
    oldWords->Destroy();
    results->Start_Get();
    while ((newWord = (HtWordReference *) results->Get_Next()))
    {
      oldWords->Add(newWord);
    }
    if(debug) cerr << "old words count: " << oldWords->Count() << endl;
    results->Release();
    delete results;

    newWords->Destroy();
    delete newWords;

}

//*****************************************************************************
// Allocate scores based on words in  wordList.
// Fields within which the word must appear are specified in  flags
// (see HtWordReference.h).
void
Parser::score(List *wordList, double weight, unsigned int flags)
{
	HtConfiguration* config= HtConfiguration::config();
    DocMatch	*dm;
    HtWordReference *wr;
    static double text_factor = config->Double("text_factor", 1);
    static double caps_factor = config->Double("caps_factor", 1);
    static double title_factor = config->Double("title_factor", 1);
    static double heading_factor = config->Double("heading_factor", 1);
    static double keywords_factor = config->Double("keywords_factor", 1);
    static double meta_description_factor = config->Double("meta_description_factor", 1);
    static double author_factor = config->Double("author_factor", 1);
    static double description_factor = config->Double("description_factor", 1);
    double	  wscore;
    int		  docanchor;
    int		  word_count;

    if (!wordList || wordList->Count() == 0)
      {
 	// We can't score an empty list, so push a null pointer...
 	if(debug) cerr << "score: empty list, push 0 @" << stack.Size() << endl;
 
 	stack.push(0);
	return;
      }

    ResultList	*list = new ResultList;
    if(debug) cerr << "score: push @" << stack.Size() << endl;
    stack.push(list);
    // We're now guaranteed to have a non-empty list
    // We'll use the number of occurences of this word for scoring
    word_count = wordList->Count();

    wordList->Start_Get();
    while ((wr = (HtWordReference *) wordList->Get_Next()))
      {
	//
	// *******  Compute the score for the document
	//

	// If word not in one of the required fields, skip the entry.
	// Plain text sets no flag in dbase, so treat it separately in
	// second line.  A capitalised word is still "plain".
	if (!(wr->Flags() & flags) &&
		((wr->Flags() & ~FLAG_CAPITAL) || !(flags & FLAG_PLAIN)))
	{
	    if (debug > 2)
		cerr << "Flags " << wr->Flags() << " lack " << flags << endl;
	    continue;
	}

	wscore = 0.0;
	if ((wr->Flags() & ~FLAG_CAPITAL) == FLAG_TEXT)
	    					wscore += text_factor;
	if (wr->Flags() & FLAG_CAPITAL)		wscore += caps_factor;
	if (wr->Flags() & FLAG_TITLE)		wscore += title_factor;
	if (wr->Flags() & FLAG_HEADING)		wscore += heading_factor;
	if (wr->Flags() & FLAG_KEYWORDS)	wscore += keywords_factor;
	if (wr->Flags() & FLAG_DESCRIPTION)	wscore += meta_description_factor;
	if (wr->Flags() & FLAG_AUTHOR)		wscore += author_factor;
	if (wr->Flags() & FLAG_LINK_TEXT)	wscore += description_factor;
	wscore *= weight;
	wscore = wscore / (double)word_count;
	docanchor = wr->Anchor();
	dm = list->find(wr->DocID());
	if (dm)
	  {
	    wscore += dm->score;
	    if (dm->anchor < docanchor)
		docanchor = dm->anchor;
	    // We wish to *update* this, not add a duplicate
	    list->remove(wr->DocID());
	  }

	if (debug > 2)
	    cerr << "Flags " << wr->Flags() << " score " << wscore << endl;

	dm = new DocMatch;
	dm->id = wr->DocID();
	dm->score = wscore;
	dm->orMatches = 1;		// how many "OR" terms this doc has
	dm->anchor = docanchor;
	list->add(dm);
      }
}


//*****************************************************************************
// The top two entries in the stack need to be ANDed together.
//
//	a	b	a and b
//	0	0	0
//	0	1	0
//	0	x	0
//	1	0	0
//	1	1	intersect(a,b)
//	1	x	a
//	x	0	0
//	x	1	b
//	x	x	x
//
void
Parser::perform_and()
{
    ResultList		*l1 = (ResultList *) stack.pop();
    ResultList		*l2 = (ResultList *) stack.pop();
    int			i;
    DocMatch		*dm, *dm2, *dm3;
    HtVector		*elements;

    if(!(l2 && l1))
    {
	if(debug) cerr << "and: at least one empty operator, pushing 0 @" << stack.Size() << endl;
	stack.push(0);
	if(l1) delete l1;
	if(l2) delete l2;
	return;
    }

    //
    // If either of the arguments is set to be ignored, we will use the
    // other as the result.
    // remember l2 and l1, l2 not l1

    if (l1->isIgnore && l2->isIgnore)
    {
	if(debug) cerr << "and: ignoring all, pushing ignored list @" << stack.Size() << endl;
	ResultList *result = new ResultList;
	result->isIgnore = 1;
	delete l1; delete l2;
	stack.push(result);
	return;
    }
    else if (l1->isIgnore)
    {
	if(debug) cerr << "and: ignoring l1, pushing l2 @" << stack.Size() << endl;
	stack.push(l2);
	delete l1;
	return;
    }
    else if (l2->isIgnore)
    {
	if(debug) cerr << "and: ignoring l2, pushing l2 @" << stack.Size() <<  endl;
	stack.push(l1);
	delete l2;
	return;
    }
    
    ResultList		*result = new ResultList;
    stack.push(result);
    elements = l2->elements();

    if(debug)
	cerr << "perform and: " << elements->Count() << " " << l1->elements()->Count() << " ";

    for (i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	dm2 = l1->find(dm->id);
	if (dm2)
	{
	    //
	    // Duplicate document.  Add scores and average "OR-matches" count
	    //
	    dm3 = new DocMatch;
//		"if (dm2)" means "?:" operator not needed...
//	    dm3->score = dm->score + (dm2 ? dm2->score : 0);
//	    dm3->orMatches = (dm->orMatches + (dm2 ? dm2->orMatches : 0))/2;
	    dm3->score = dm->score + dm2->score;
	    dm3->orMatches = (dm->orMatches + dm2->orMatches)/2;
	    dm3->id = dm->id;
	    dm3->anchor = dm->anchor;
//	    if (dm2 && dm2->anchor < dm3->anchor)
	    if (dm2->anchor < dm3->anchor)
		dm3->anchor = dm2->anchor;
	    result->add(dm3);
	}
    }
    if(debug)
	cerr << result->elements()->Count() << endl;

    elements->Release();
    delete elements;
    delete l1;
    delete l2;
}

//	a	b	a not b
//	0	0	0
//	0	1	0
//	0	x	0
//	1	0	a
//	1	1	intersect(a,not b)
//	1	x	a
//	x	0	x
//	x	1	x
//	x	x	x
void
Parser::perform_not()
{
    ResultList		*l1 = (ResultList *) stack.pop();
    ResultList		*l2 = (ResultList *) stack.pop();
    int			i;
    DocMatch		*dm, *dm2, *dm3;
    HtVector		*elements;


    if(!l2)
    {
	if(debug) cerr << "not: no positive term, pushing 0 @" << stack.Size() << endl;
	// Should probably be interpreted as "* not l1"
	stack.push(0);
	if(l1) delete l1;
	return;
    }
    if(!l1 || l1->isIgnore || l2->isIgnore)
    {
	if(debug) cerr << "not: no negative term, pushing positive @" << stack.Size() << endl;
        stack.push(l2);
	if(l1) delete l1;
        return;
    }

    ResultList		*result = new ResultList;
    if(debug) cerr << "not: pushing result @" << stack.Size() << endl;
    stack.push(result);
    elements = l2->elements();

    if(debug)
	cerr << "perform not: " << elements->Count() << " " << l1->elements()->Count() << " ";

    for (i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	dm2 = l1->find(dm->id);
	if (!dm2)
	{
	    //
	    // Duplicate document.
	    //
	    dm3 = new DocMatch;
	    dm3->score = dm->score;
	    dm3->orMatches = dm->orMatches;
	    dm3->id = dm->id;
	    dm3->anchor = dm->anchor;
	    result->add(dm3);
	}
    }
    if(debug)
	cerr << result->elements()->Count() << endl;

    elements->Release();
    delete elements;
    delete l1;
    delete l2;
}

//*****************************************************************************
// The top two entries in the stack need to be ORed together.
//
void
Parser::perform_or()
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
	if(debug) cerr << "or: no 2nd operand" << endl;
	return; // result in top of stack
    }
    else if (l1 && !result)
    {
	if(debug) cerr << "or: no 1st operand" << endl;
	stack.pop();
	stack.push(l1);
	return;
    }
    else if (!l1 && !result)
    {
	if(debug) cerr << "or: no operands" << endl;
	stack.pop();
	stack.push(0); // empty result
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
    if(debug)
	cerr << "perform or: " << elements->Count() << " " << result->elements()->Count() << " ";
    for (i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	dm2 = result->find(dm->id);
	if (dm2)
	{
	    //
	    // Update document.  Add scores and add "OR-match" counts
	    //
	    dm2->score += dm->score;
	    dm2->orMatches += dm->orMatches;
	    if (dm->anchor < dm2->anchor)
		dm2->anchor = dm->anchor;
	}
	else
	{
	    dm2 = new DocMatch;
	    dm2->score = dm->score;
	    dm2->orMatches = dm->orMatches;
	    dm2->id = dm->id;
	    dm2->anchor = dm->anchor;
	    result->add(dm2);
	}
    }
    if(debug)
	cerr << result->elements()->Count() << endl;
    elements->Release();
    delete elements;
    delete l1;
}

//*****************************************************************************
// void Parser::parse(List *tokenList, ResultList &resultMatches)
//
void
Parser::parse(List *tokenList, ResultList &resultMatches)
{
	HtConfiguration* config= HtConfiguration::config();
    tokens = tokenList;
    DocumentRef *ref = NULL;

    fullexpr(1);

    ResultList	*result = (ResultList *) stack.pop();
    if (!result)  // Ouch!
      {
//	It seems we now end up here on a syntax error, so don't clear anything!
//	valid = 0;
//	error = 0;
//	error << "Expected to have something to parse!";
	return;
      }
    HtVector	*elements = result->elements();
    DocMatch	*dm;

    // multimatch_factor  gives extra weight to matching documents which
    // contain more than one "OR" term.  This is applied after the whole
    // document is parsed, so multiple matches don't give exponentially
    // increasing weights
    double multimatch_factor = config->Double("multimatch_factor");

    for (int i = 0; i < elements->Count(); i++)
    {
	dm = (DocMatch *) (*elements)[i];
	ref = collection->getDocumentRef(dm->GetId());
        if(ref && ref->DocState() == Reference_normal)
	{
	    dm->collection = collection; // back reference
	    if (dm->orMatches > 1)
		dm->score *= 1+multimatch_factor;
	    resultMatches.add(dm);
	}
    }
    elements->Release();
    result->Release();
    delete elements;
    delete result;
}

void
Parser::setCollection(Collection *coll)
{
    if (coll)
        words.Open(coll->getWordFile(), O_RDONLY);
    collection = coll;
}       

