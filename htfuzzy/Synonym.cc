//
// Synonym.cc
//
// Implementation of Synonym
//
//
#if RELEASE
static char RCSid[] = "$Id: Synonym.cc,v 1.3.2.2 1999/03/31 21:25:12 grdetil Exp $";
#endif

#include "Synonym.h"
#include "htfuzzy.h"
#include "List.h"
#include "StringList.h"
#include "Configuration.h"
#include <stdio.h>
#include <fstream.h>
#include <stdlib.h>


//*****************************************************************************
Synonym::Synonym()
{
    name = "synonyms";
    db = 0;
}


//*****************************************************************************
Synonym::~Synonym()
{
    if (db)
    {
        db->Close();
        delete db;
        db = 0;
    }
}


//*****************************************************************************
int
Synonym::createDB(Configuration &config)
{
    char	*sourceFile;
    char	*dbFile;
    char	input[1000];
    FILE	*fl;
	
    sourceFile = config["synonym_dictionary"];
    dbFile = config["synonym_db"];

    fl = fopen(sourceFile, "r");
    if (fl == NULL)
    {
	cout << "htfuzzy/synonyms: unable to open " << sourceFile << endl;
	cout << "htfuzzy/synonyms: Use the 'synonym_dictionary' attribute\n";
	cout << "htfuzzy/synonyms: to specify the file that contains the synonyms\n";
	return NOTOK;
    }

    Database	*db = Database::getDatabaseInstance();

    if (db->OpenReadWrite(dbFile, 0664) == NOTOK)
    {
	delete db;
	db = 0;
	return NOTOK;
    }

    String	data;
    String	word;
    int		count = 0;
    while (fgets(input, sizeof(input), fl))
    {
	StringList	sl(input, " \t\r\n");
	for (int i = 0; i < sl.Count(); i++)
	{
	    data = 0;
	    for (int j = 0; j < sl.Count(); j++)
	    {
		if (i != j)
		    data << sl[j] << ' ';
	    }
	    word = sl[i];
	    word.lowercase();
	    data.lowercase();
	    db->Put(word, data, data.length() - 1);
	    if (debug && (count % 10) == 0)
	    {
                cout << "htfuzzy/synonyms: " << count << ' ' << word << "\n";
		cout.flush();
	    }
	    count++;
	}
    }
    fclose(fl);
    db->Close();
    delete db;
    if (debug)
    {
        cout << "htfuzzy/synonyms: " << count << ' ' << word << "\n";
	cout << "htfuzzy/synonyms: Done.\n";
    }
    return OK;
}


//*****************************************************************************
int
Synonym::openIndex(Configuration &)
{
    char	*dbFile = config["synonym_db"];
	
    if (db)
    {
        db->Close();
        delete db;
        db = 0;
    }
    db = Database::getDatabaseInstance();
    if (db->OpenRead(dbFile) == NOTOK)
    {
	delete db;
	db = 0;
	return NOTOK;
    }
    return OK;
}


//*****************************************************************************
void
Synonym::getWords(char *originalWord, List &words)
{
    String	data;

    if (db && db->Get(originalWord, data) == OK)
    {
	char	*token = strtok(data.get(), " ");
	while (token)
	{
	    words.Add(new String(token));
	    token = strtok(0, " ");
	}
    }
}
