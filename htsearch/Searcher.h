//
// Searcher.h
//
// Searcher: Retrieves and scores the document list
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Searcher.h,v 1.1 1999/10/15 03:38:49 jtillman Exp $
//

#ifndef _parser_h_
#define _parser_h_

//includes from htsearch.cc
#include "WeightWord.h"
#include "../htfuzzy/Fuzzy.h"
#include "WordRecord.h"
#include "HtWordList.h"
#include "StringList.h"
#include "IntObject.h"
#include "HtURLCodec.h"
#include "WordType.h"
#include "HtRegex.h"

#include <time.h>
#include <ctype.h>
#include <signal.h>


//includes from htsearch.h

#include "List.h"
#include "StringList.h"
#include "Dictionary.h"
#include "DocumentRef.h"
#include "Database.h"
#include "good_strtok.h"
#include "DocumentDB.h"
#include "htString.h"
#include "Configuration.h"
#include "ResultMatch.h"
#include "ResultList.h"
#include "HtWordReference.h"
#include "StringMatch.h"
#include "defaults.h"

#include <stdio.h>
#include <fstream.h>
#include <stdlib.h>
#include <unistd.h>

//Includes from parser.h

#include "WeightWord.h"
#include "ResultList.h"
#include "DocMatch.h"
#include "Database.h"
#include "htString.h"
#include "Stack.h"
#include "HtWordList.h"
#include <ctype.h>

extern Configuration config;
extern ConfigDefaults defaults[];

class Searcher
{
	int minimum_word_length;
	int debug;


public:
    Searcher();
    ResultList *execute();
    int		checkSyntax(List *);
    void		search(List *, ResultList &);
    void		setDebug(int i) {debug = i;}
    void		setDatabase(const String& db)		{ words.Open(db, O_RDONLY); }
    char		*getErrorMessage()		{return error.get();}
    void		setWordDB(char *new_db) {word_db = new_db;}
    void		setSearchWords(char *new_words) {originalWords = new_words;}
    void		setRequiredWords(char *new_words) {requiredWords = new_words;}
    void		setExclusion(char *new_words) {}
    void		setRestriction(char *new_words) {}
	 void		setConfigVal(char *item, char *value) {config.Add(item, value);}
	 void		setConfigFile(char *filename) {config.Read(filename);}
    List		*getSearchWords()	{return &searchWords;}
    HtRegex *getExclusion() {return &exclude;}
    HtRegex *getRestriction() {return &restrict;}
    char		*getOriginalWords() {return originalWords;}
    char		*getLogicalWords() {return logicalWords;}
    int			hadError()			{return valid == 0;}
	
protected:
    void		fullexpr(int);
    int		lexan();
    void		expr(int);
    void		term(int);
	void phrase(int);
   void		factor(int);
    int		match(int);
    void		setError(char *);
    void		perform_push();
    void		perform_and(int);
    void		perform_or();
     void		perform_phrase(List &);

    void		score(List *, double weight);
	 void		collate(ResultList &);
//	 void 	createLogicalWords(class List &, class String &, class String &);
	 void 	createLogicalWords();
	 void		addRequiredWords(List &, StringList &);
//	  int    checkSyntax(List *tokenList);
	 void 	dumpWords(class List &, char *);
	 void		setupWords(char *, int);
	 void 	doFuzzy(class WeightWord *, class List &, class List &);
		void 	convertToBoolean(class List &);
//	 void   factor(int output);
//	 void   phrase(int output);

	
    List		*tokens;
    List		*result;
    WeightWord		*current;
    int			lookahead;
    int			valid;
    Stack		stack;
    Database		*dbf;
    String		error;
    String	word_db;
	 List searchWords;
	 String originalWords;
	 String requiredWords;
	 HtRegex	exclude;
	 HtRegex	restrict;
	 String logicalPattern;
	 String logicalWords;
   HtWordList		words;
};


#endif































