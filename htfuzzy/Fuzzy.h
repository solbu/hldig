//
// Fuzzy.h
//
// This is the base class for all the different types of fuzzy searches.
// We only define the interface.
//
// There are two main uses of classes derived from this class:
//    1) Creation of a fuzzy index
//    2) Searching for a word using the fuzzy index
//
// The Fuzzy classes take the raw words from the user's query and generate
// a list of words to be looked up in the database. These words are created
// using the getWords call and can either be picked off from a separate fuzzy
// database specific to the method, or by generating words on the fly.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Fuzzy.h,v 1.4 1999/09/10 01:37:39 ghutchis Exp $
//
//
#ifndef _Fuzzy_h_
#define _Fuzzy_h_

#include "Object.h"
#include "htString.h"
#include "Database.h"
#include "HtWordType.h"
#include "WordList.h"

class Configuration;
class Dictionary;
class List;


class Fuzzy : public Object
{
public:
    //
    // Construction/Destruction
    //
    Fuzzy();
    virtual		~Fuzzy();

    //
    // Given a single work, generate a list of replacement words using
    // the current algorithm.
    //
    virtual void	getWords(char *word, List &words);

    //
    // For the current algorithm, open the key database
    //
    virtual int		openIndex(Configuration &config);

    //
    // For searching, we will need to keep track of the weight associated
    // with a particular fuzzy algorithm.
    //
    void		setWeight(double w)		{weight = w;}
    double		getWeight()			{return weight;}

    //*******************************************************************
    // The following are used in the creation of the fuzzy databases.
    //
    // For the current algorithm, write the database to disk.
    //
    virtual int		writeDB(Configuration &config);

    //
    // For the current algorithm, create the database.
    // This is for those algoritms that don't need a list of words
    // to work.
    //
    virtual int		createDB(Configuration &config);
	
    //
    // Given a word from the htdig word database, create the appropriate
    // entries into memory which will later be written out with writeDB().
    //
    virtual void	addWord(char *word);

    //
    // Each algorithm has a name...
    //
    char		*getName()			{return name;}

    //
    // Fuzzy algorithm factory.  This returns a new Fuzzy algorithm
    // object that belongs to the given name.
    //
    static Fuzzy	*getFuzzyByName(char *name);
	
protected:
    //
    // Given a single word, generate a database key
    //
    virtual void	generateKey(char *word, String &key);

    char		*name;
    Database		*index;
    Dictionary		*dict;
    double		weight;
};

#endif


