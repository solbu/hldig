//
// EndingsDB.cc
//
// EndingsDB: Implementation of the private endings database
//           
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: EndingsDB.cc,v 1.17 2004/05/28 13:15:20 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "Endings.h"
#include "htfuzzy.h"
#include "SuffixEntry.h"
#include "Dictionary.h"
#include "List.h"
#include "HtConfiguration.h"

#include "filecopy.h"

// This is an attempt to get around compatibility problems 
// with the included regex
#ifdef _MSC_VER /* _WIN32 */
#include "regex_win32.h"
#else
# ifdef USE_RX
#  include <rxposix.h>
# else // Use regex
#  ifdef HAVE_BROKEN_REGEX
#   include <regex.h>
#  else // include regex code and header
#   include "gregex.h"
#  endif
# endif
#endif //_MSC_VER /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

//*****************************************************************************
//
int
Endings::createDB(const HtConfiguration &config)
{
    Dictionary  rules;
    String      tmpdir = getenv("TMPDIR");
    String      word2root, root2word;
    
#if defined(LIBHTDIG) || defined(LIBHTDIGPHP) || defined(_MSC_VER) //WIN32
    int ret = -1;
    char * source = NULL;
    char * dest = NULL;
#endif

    if (tmpdir.length())
      {
  word2root = tmpdir;
  root2word = tmpdir;
      }
    else
      {
  word2root = "/tmp";
  root2word = "/tmp";
      }

    word2root << "/word2root.db";
    root2word << "/root2word.db";

    if (debug)
  cout << "htfuzzy/endings: Reading rules\n";
  
    if (readRules(rules, config["endings_affix_file"]) == NOTOK)
  return NOTOK;

    if (debug)
  cout << "htfuzzy/endings: Creating databases\n";
  
    if (createRoot(rules, word2root, root2word,
       config["endings_dictionary"]) == NOTOK)
  return NOTOK;

    //
    // Since we used files in TMPDIR for our temporary databases, we need
    // to now move them to the correct location as defined in the config
    // database.
    //
    
#if defined(LIBHTDIG) || defined(LIBHTDIGPHP) || defined(_MSC_VER) //WIN32

    //Uses file_copy function - works on Unix/Linux & WinNT
    source = root2word.get();
    dest = (char *)config["endings_root2word_db"].get();

    //Attempt rename, if fail attempt copy & delete.
    ret = rename(source, dest);
    if (ret < 0)
    {
        ret = file_copy(source, dest, FILECOPY_OVERWRITE_ON);
        if (ret == TRUE)
            unlink(source);
        else
            return NOTOK;
    }

    source = word2root.get();
    dest = (char *)config["endings_word2root_db"].get();

    //Attempt rename, if fail attempt copy & delete.
    ret = rename(source, dest);
    if (ret < 0)
    {
        ret = file_copy(source, dest, FILECOPY_OVERWRITE_ON);
        if (ret == TRUE)
            unlink(source);
        else
            return NOTOK;
    }
    
#else //This code uses a system call - Phase this out

    struct stat stat_buf;
    String mv("mv");  // assume it's in the PATH if predefined setting fails
    if ((stat(MV, &stat_buf) != -1) && S_ISREG(stat_buf.st_mode))
  mv = MV;
    system(form("%s %s %s;%s %s %s",
  mv.get(), root2word.get(), config["endings_root2word_db"].get(),
  mv.get(), word2root.get(), config["endings_word2root_db"].get()));

#endif

    return OK;

}


//*****************************************************************************
int
Endings::readRules(Dictionary &rules, const String& rulesFile)
{
    FILE  *fl = fopen(rulesFile, "r");

    if (fl == NULL)
  return NOTOK;

    int    inSuffixes = 0;
    char  currentSuffix[2] = " ";
    char  *p;
    char  input[1024];
    String  line;
  
    while (fgets(input, sizeof(input), fl))
    {
  if (input[0] == '\n' || input[0] == '#')
      continue;

  if (mystrncasecmp(input, "suffixes", 8) == 0)
  {
      inSuffixes = 1;
      continue;
  }
  else if (mystrncasecmp(input, "prefixes", 8) == 0)
  {
      inSuffixes = 0;
      continue;
  }
  if (!inSuffixes)
      continue;

  if (mystrncasecmp(input, "flag ", 5) == 0)
  {
      p = input + 5;
      while (*p == '*' || *p == ' ' || *p == '\t')
    p++;
      currentSuffix[0] = *p;
  }
  else
  {
      line << input;
      line.chop("\r\n");
      if (line.indexOf('>') > 0)
      {
    List    *list;
    SuffixEntry  *se = new SuffixEntry(line);
      
    if (rules.Exists(currentSuffix))
    {
        list = (List *) rules[currentSuffix];
    }
    else
    {
        list = new List;
        rules.Add(currentSuffix, list);
    }
    list->Add(se);
    line = 0;
      }
  }
    }

    fclose(fl);
    return OK;
}


//*****************************************************************************
int
Endings::createRoot(Dictionary &rules, char *word2root, char *root2word, const String& dictFile)
{
    FILE  *fl = fopen(dictFile, "r");
    if (fl == NULL)
  return NOTOK;

    Database  *w2r = Database::getDatabaseInstance(DB_BTREE);
    Database  *r2w = Database::getDatabaseInstance(DB_BTREE);

    w2r->OpenReadWrite(word2root, 0664);
    r2w->OpenReadWrite(root2word, 0664);
  
    char  input[1024];
    char  *p;
    String  words;
    String  word;
    List  wordList;
    int    count = 0;
    String  data;
  
    while (fgets(input, sizeof(input), fl))
    {
  if ((count % 100) == 0 && debug == 1)
  {
      cout << "htfuzzy/endings: words: " << count << '\n';
      cout.flush();
  }
  count++;
    
  p = strchr(input, '/');
  if (p == NULL)
      continue;    // Only words that have legal endings are used

  *p++ = '\0';

  mungeWord(input, word);
  expandWord(words, wordList, rules, word, p);

  if (debug > 1)
      cout << "htfuzzy/endings: " << word << " --> " << words << endl;

  //
  // Store the root mapped to the list of expanded words.
  //
  r2w->Put(word, words);

  //
  // For each of the expanded words, build a map to its root.
  //
  for (int i = 0; i < wordList.Count(); i++)
  {
      //
      // Append to existing record if there is one.
      //
      data = "";
      if (w2r->Get(*(String *)wordList[i], data) == OK)
    data << ' ';
      data << word;
      w2r->Put(*(String *)wordList[i], data);
  }
    }

    if (debug == 1)
  cout << endl;
  
    fclose(fl);
    w2r->Close();
    r2w->Close();
    delete w2r;
    delete r2w;

    return OK;
}


//*****************************************************************************
// Convert a word from the dictionary format into something we can actually
// use.  This means that the word will be converted to lowercase and that
// any accents will be combined into single characters.
//
void
Endings::mungeWord(char *input, String &word)
{
    char  *p = input + 1;
    
    word = 0;
    while (*input)
    {
  p = input + 1;
  switch (*p)
  {
          case '"':  // The previous character needs to get an umlaut
    switch (*input)
    {
        case 'a':
        case 'A':
      word << char(228);
      input += 2;
      continue;
      break;
        case 'e':
        case 'E':
      word << char(235);
      input += 2;
      continue;
      break;
        case 'i':
        case 'I':
      word << char(239);
      input += 2;
      continue;
      break;
        case 'o':
        case 'O':
      word << char(246);
      input += 2;
      continue;
      break;
        case 'u':
        case 'U':
      word << char(252);
      input += 2;
      continue;
      break;
    }
    break;
    
      case 'S':  // See if the previous character needs to be an sz
    if (*input == 's')
    {
        word << char(223);
        input += 2;
        continue;
    }
    else
    {
        word << *input;
    }
    break;
    
      default:
    word << *input;
    break;
  }
  input++;
    }
    word.lowercase();
}


//*****************************************************************************
void
Endings::expandWord(String &words, List &wordList,
        Dictionary &rules, char *word, char *suffixes)
{
    char  suffix[2] = " ";
    String  root;
    SuffixEntry  *entry;
    List  *suffixRules;
    char  *p;
    String  rule;
  
    words = 0;
    wordList.Destroy();

    while (*suffixes > ' ')
    {
  suffix[0] = *suffixes++;
  if (!rules.Exists(suffix))
      continue;

  suffixRules = (List *) rules[suffix];
  for (int i = 0; i < suffixRules->Count(); i++)
  {
      entry = (SuffixEntry *) (*suffixRules)[i];
      root = word;
      regex_t  reg;
      rule = entry->rule;
      if (strchr((char*)rule, '\''))
    continue;
      if (debug > 2)
    cout << "Applying regex '" << entry->expression << "' to " << word << endl;
      regcomp(&reg, (char*)entry->expression, REG_ICASE | REG_NOSUB | REG_EXTENDED);
      if (regexec(&reg, word, 0, NULL, 0) == 0)
      {
    //
    // Matched
    //
    if (rule[0] == '-')
    {
        //
        // We need to remove something...
        //
        p = strchr((char*)rule, ',');
        if (p)
        {
      *p++ = '\0';
      root.chop((int)strlen(rule.get()) - 1);
      root << p;
        }
    }
    else
    {
        root << rule;
    }
    root.lowercase();
    if (debug > 2)
        cout << word << " with " << rule << " --> '" << root << "'\n";
    wordList.Add(new String(root));
    words << root << ' ';
      }
      regfree(&reg);
  }
    }
    words.chop(1);
}
