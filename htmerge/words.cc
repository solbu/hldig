//
// words.cc
//
// Implementation of htmerge
//
// $Log: words.cc,v $
// Revision 1.8  1999/01/14 00:27:38  ghutchis
// Small speed improvements.
//
// Revision 1.7  1999/01/07 03:13:50  ghutchis
// Fix minor memory leaks.
//
// Revision 1.6  1998/12/12 17:49:38  bergolth
// Small change so that it compiles on AIX
//
// Revision 1.5  1998/12/12 01:44:58  ghutchis
// Fixed a bug where pointer, rather than strings were assigned. Silly
// references...
//
// Revision 1.4  1998/12/06 18:43:31  ghutchis
// Check for word entries that are duplicates and compact them.
//
// Revision 1.3  1998/12/05 00:53:24  ghutchis
// Don't store c:1 and a:0 entries in db.wordlist to save space.
//
// Revision 1.2  1998/11/15 22:24:19  ghutchis
//
// Change \r to \n as noted by Andrew Bishop.
//
// Revision 1.1.1.1  1997/02/03 17:11:06  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: words.cc,v 1.8 1999/01/14 00:27:38 ghutchis Exp $";
#endif

#include "htmerge.h"


//*****************************************************************************
// void mergeWords(char *wordtmp, char *wordfile)
//
void
mergeWords(char *wordtmp, char *wordfile)
{
    FILE	*wordlist = fopen(form("%s.new", wordtmp), "w");
    Database	*dbf = Database::getDatabaseInstance();
    String	out;
    String	currentWord;
    int		removeBadUrls = config.Boolean("remove_bad_urls");
    char	buffer[1000];
    String	word;
    char        *sid;
    char	*name, *value, *pair;
    int		word_count = 0;
    WordRecord	wr, last_wr;
    String      last_word;

    //
    // Check for file access errors
    //
    if (!wordlist)
    {
	reportError(form("Unable to create temporary word file '%s.new'",
			 wordtmp));
    }
    if (access(wordtmp, R_OK) < 0)
    {
	reportError(form("Unable to open word list file '%s'", wordtmp));
    }
    if (dbf->OpenReadWrite(wordfile, 0664) == NOTOK)
    {
	reportError(form("Unable to create/open the word database '%s'",
			 wordfile));
    }
	
    //
    // Sort the list of words.  This uses the unix sort program since it
    // is very good at it.  :-)
    //
    if (verbose)
	cout << "htmerge: Sorting..." << endl;

    String	command = SORT_PROG;
    String	tmpdir = getenv("TMPDIR");
    if (tmpdir.length())
    {
	command << " -T " << tmpdir;
    }
    command << ' ' << wordtmp;
    FILE	*sorted = popen(command, "r");
    if (!sorted)
    {
	reportError("Unable to sort");
	exit(1);
    }
	
    //
    // Read sorted lines
    //
    while (fgets(buffer, sizeof(buffer), sorted))
    {
	if (*buffer == '+')
	{
	    //
	    // This tells us that the document hasn't changed and we
	    // are to reuse the old words
	    //
	}
	else if (*buffer == '-')
	{
 	    if (removeBadUrls)
	    {
		discard_list.Add(strtok(buffer + 1, "\n"), 0);
		if (verbose)
		    cout << "htmerge: Removing doc #" << buffer + 1 << endl;
	    }
	}
	else if (*buffer == '!')
	{
	    discard_list.Add(strtok(buffer + 1, "\n"), 0);
	    if (verbose)
		cout << "htmerge: doc #" << buffer + 1 <<
		    " has been superceeded." << endl;
	}
	else
	{
	    //
	    // Split the line up into the word, count, location, and
	    // document id.
	    //
	    word = good_strtok(buffer, "\t");
	    pair = good_strtok("\t");
	    wr.Clear();   // Reset count to 1, anchor to 0, and all that
	    sid = "-";
	    while (pair && *pair)
	    {
		name = strtok(pair, ":");
		value = strtok(0, "\n");
		if (name && *name && value && *value)
		{
		    switch (*name)
		    {
			case 'c':
			    wr.count = atoi(value);
			    break;
			case 'l':
			    wr.location = atoi(value);
			    break;
			case 'i':
			    sid = value;
			    wr.id = atoi(value);
			    break;
			case 'w':
			    wr.weight = atoi(value);
			    break;
			case 'a':
			    wr.anchor = atoi(value);
			    break;
		    }
		}
		pair = good_strtok("\t");
	    }

	    //
	    // If the word is from a document we need to discard, we
	    // don't want to add it to the database
	    //
	    if (discard_list.Exists(sid))
	    {
		if (verbose > 1)
		{
		    cout << "htmerge: Discarding " << word << " in doc #"
			 << sid << "     \n";
		    cout.flush();
		}
		continue;
	    }

	    // Are we on the first word?
	    // If so, last_word == NULL and we don't want to add that!
	    if (last_word.length() == 0)
	      {
		last_word = word;
		last_wr = wr;
		continue;
	      }

	    // OK... Now that we have our new WordRecord parsed
	    // Do we (by horrible chance) duplicate the last entry?
	    // If we do, update last_word and keep going
	    if ((last_wr.id == wr.id)
		&& (last_word == word))
	      {
		last_wr.count += wr.count;
		last_wr.weight += wr.weight;
		if (wr.location < last_wr.location)
		  last_wr.location = wr.location;
		if (wr.anchor < last_wr.anchor)
		  last_wr.anchor = wr.anchor;
		continue;
	      }

	    //
	    // Record the word in the new wordlist file
	    //
	    fprintf(wordlist, "%s", last_word.get());
            fprintf(wordlist, "\ti:%d\tl:%d\tw:%d",
		    last_wr.id,
		    last_wr.location,
		    last_wr.weight);
            if (last_wr.count != 1)
            {
               	fprintf(wordlist, "\tc:%d", last_wr.count);
            }
	    if (last_wr.anchor != 0)
            {
               	fprintf(wordlist, "\ta:%d",last_wr.anchor);
            }
            putc('\n', wordlist);
	    
	    //
	    // Since we will be storing binary equivalents of the
	    // data, we need to get them into the form that we are
	    // going to use (shorts and ints)
	    //

	    if (currentWord.length() == 0)
	    {
		//
		// First word.  Special case.
		//
		out = 0;
		out.append((char *) &last_wr, sizeof(last_wr));
		currentWord = last_word;
	    }
	    else if (strcmp(last_word, currentWord) == 0)
	    {
		//
		// Add to current record
		//
		out.append((char *) &last_wr, sizeof(last_wr));
	    }
	    else
	    {
		//
		// New word.  Terminate the previous one
		//
		dbf->Put(currentWord, out.get(), out.length());

		currentWord = last_word;

		out = 0;
		out.append((char *) &last_wr, sizeof(last_wr));
		word_count++;
		if (verbose && word_count == 1)
		{
		    cout << "htmerge: Merging..." << endl;
		}
		if (verbose && word_count % 100 == 0)
		{
		    cout << "htmerge: " << word_count << ':' << word
			 << "              \n";
		    cout.flush();
		}
	    }
	    
	    // Save the new word for comparison with the next ones
	    last_wr = wr;
	    last_word = word;
	}
    }

    //
    // Check for successful completion of sort
    //
    int		sortRC = pclose(sorted);
    if (sortRC)
    {
	reportError("Word sort failed");
	exit(1);
    }

    // We still have to add the last word
    // This could be cleaned up by putting this code and that from the loop
    // above into a separate function. It's not pretty, but it works correctly.
    fprintf(wordlist, "%s", last_word.get());
    fprintf(wordlist, "\ti:%d\tl:%d\tw:%d",
		last_wr.id,
		last_wr.location,
		last_wr.weight);
    if (last_wr.count != 1)
      {
	fprintf(wordlist, "\tc:%d", last_wr.count);
      }
    if (last_wr.anchor != 0)
      {
	fprintf(wordlist, "\ta:%d",last_wr.anchor);
      }
    putc('\n', wordlist);
	    
    if (currentWord.length() == 0)
      {
	//
	// First word.  Special case.
	//
	out = 0;
	out.append((char *) &last_wr, sizeof(last_wr));
	currentWord = last_word;
      }
    else if (strcmp(last_word, currentWord) == 0)
      {
	//
	// Add to current record
	//
	out.append((char *) &last_wr, sizeof(last_wr));
      }
    else
      {
	//
	// New word.  Terminate the previous one
	//
	dbf->Put(currentWord, out.get(), out.length());
	
	currentWord = last_word;
	
	out = 0;
	out.append((char *) &last_wr, sizeof(last_wr));
	word_count++;
	if (verbose && word_count == 1)
	  {
	    cout << "htmerge: Merging..." << endl;
	  }
	if (verbose && word_count % 100 == 0)
	  {
	    cout << "htmerge: " << word_count << ':' << word
		 << "              \n";
	    cout.flush();
	  }
      }
    
    dbf->Put(currentWord, out.get(), out.length());
    dbf->Close();
    delete dbf;

    if (verbose)
	cout << "\n";
    if (stats)
	cout << "htmerge: Total word count: " << word_count << endl;

    //
    // Deal with the new wordlist file.  We need to replace the old file with
    // the new one.
    //
    fclose(wordlist);
    unlink(wordtmp);
    link(form("%s.new", wordtmp), wordtmp);
    unlink(form("%s.new", wordtmp));
}


