//
// Metaphone.cc
//
// Implementation of Metaphone
//
//
//

#include "Metaphone.h"
#include <ctype.h>
#include "Dictionary.h"


//*****************************************************************************
// Metaphone::Metaphone()
//
Metaphone::Metaphone()
{
	name = "metaphone";
}


//*****************************************************************************
// Metaphone::~Metaphone()
//
Metaphone::~Metaphone()
{
}


//*****************************************************************************
// void Metaphone::generateKey(char *word, String &key)
//
/*
 * This code was copied from the slapd package developed at umich.
 * it was debugged and cleaned up in February 1999 by Geoffrey Hutchison
 * for the ht://Dig Project.
 */
/*
 * Metaphone copied from C Gazette, June/July 1991, pp 56-57,
 * author Gary A. Parker, with changes by Bernard Tiffany of the
 * University of Michigan, and more changes by Tim Howes of the
 * University of Michigan.
 */

/* Character coding array */
static char     vsvfn[26] = {
           1, 16, 4, 16, 9, 2, 4, 16, 9, 2, 0, 2, 2,
        /* A   B  C   D  E  F  G   H  I  J  K  L  M  */
           2, 1, 4, 0, 2, 4, 4, 1, 0, 0, 0, 8, 0};
        /* N  O  P  Q  R  S  T  U  V  W  X  Y  Z  */

/* Macros to access character coding array */
#define vscode(x)  ((x) >= 'A' && (x) <= 'Z' ? vsvfn[(x) - 'A'] : 0)
#define vowel(x)   ((x) != '\0' && vscode(x) & 1)   /* AEIOU */
#define same(x)    ((x) != '\0' && vscode(x) & 2)   /* FJLMNR */
#define varson(x)  ((x) != '\0' && vscode(x) & 4)   /* CGPST */
#define frontv(x)  ((x) != '\0' && vscode(x) & 8)   /* EIY */
#define noghf(x)   ((x) != '\0' && vscode(x) & 16)  /* BDH */

#define	MAXPHONEMELEN	6

void
Metaphone::generateKey(char *word, String &key)
{
    if (!word || !*word)
      return;

    char			*n;
    String			ntrans;
	
    /*
     * Copy Word to internal buffer, dropping non-alphabetic characters
     * and converting to upper case
     */

    ntrans << "0000";
	
    for (; *word; word++)
    {
        if (isalpha(*word))
            ntrans << *word;
    }
    ntrans.uppercase();
	
    /* ntrans[0] will always be == 0 */
    n = ntrans.get();
    *n++ = 0;
    *n++ = 0;
    *n++ = 0;
    *n = 0;                 /* Pad with nulls */
    n = ntrans.get() + 4;   /* Assign pointer to start */

    /* Check for PN, KN, GN, AE, WR, WH, and X at start */
    switch (*n)
    {
    case 'P':
    case 'K':
    case 'G':
        /* 'PN', 'KN', 'GN' becomes 'N' */
        if (*(n + 1) == 'N')
            *n++ = 0;
        break;
    case 'A':
        /* 'AE' becomes 'E' */
        if (*(n + 1) == 'E')
            *n++ = 0;
        break;
    case 'W':
        /* 'WR' becomes 'R', and 'WH' to 'W' */
        if (*(n + 1) == 'R')
            *n++ = 0;
        else if (*(n + 1) == 'H') {
            *(n + 1) = *n;
            *n++ = 0;
        }
        break;
    case 'X':
        /* 'X' becomes 'S' */
        *n = 'S';
        break;
    }

    /*
     * Now, loop step through string, stopping at end of string or when
     * the computed 'metaph' is MAXPHONEMELEN characters long
     */

    for (; *n && key.length() < MAXPHONEMELEN; n++)
    {
        /* Drop duplicates except for CC */
        if (*(n - 1) == *n && *n != 'C')
	  continue;
	/* Check for F J L M N R or first letter vowel */
	if (same(*n) || *(n - 1) == '\0' && vowel(*n))
	  key << *n;
        else
        {
            switch (*n)
            {
            case 'B':
                /*
                 * B unless in -MB
                 */
                if (*(n + 1) || *(n - 1) != 'M')
                    key << *n;
                break;
            case 'C':
                /*
                 * X if in -CIA-, -CH- else S if in
                 * -CI-, -CE-, -CY- else dropped if
                 * in -SCI-, -SCE-, -SCY- else K
                 */
                if (*(n - 1) != 'S' || !frontv(*(n + 1)))
                {
                    if (*(n + 1) == 'I' && *(n + 2) == 'A')
                        key << 'X';
                    else if (frontv(*(n + 1)))
                        key << 'S';
                    else if (*(n + 1) == 'H')
                        key << (((*(n - 1) == '\0' && !vowel(*(n + 2)))
                                 || *(n - 1) == 'S')
                                ? 'K' : 'X');
                    else
                        key << 'K';
                }
                break;
            case 'D':
                /*
                 * J if in DGE or DGI or DGY else T
                 */
                key << ((*(n + 1) == 'G' && frontv(*(n + 2)))
                        ? (char) 'J' : (char) 'T');
                break;
            case 'G':
                /*
                 * F if in -GH and not B--GH, D--GH,
                 * -H--GH, -H---GH else dropped if
                 * -GNED, -GN, -DGE-, -DGI-, -DGY-
                 * else J if in -GE-, -GI-, -GY- and
                 * not GG else K
                 *
                  */
	      if ((*(n + 1) != 'G' || vowel(*(n + 2))) &&
		  (*(n + 1) != 'N' || (*(n + 1) &&
				       (*(n + 2) != 'E' ||
					*(n + 3) != 'D'))) &&
		  (*(n - 1) != 'D' || !frontv(*(n + 1))))
		if (frontv(*(n + 1)) && *(n + 2) != 'G')
		  key << 'J';
	        else
		  key << 'K';
	      else if (*(n + 1) == 'H' && !noghf(*(n - 3)) &&
		       *(n - 4) != 'H')
		       key << 'F';
	      break;
            case 'H':
                /*
                 * H if before a vowel and not after
                 * C, G, P, S, T else dropped
                 */
                if (!varson(*(n - 1)) && (!vowel(*(n - 1
                                                   )) ||
                                          vowel(*(n + 1))))
                    key << 'H';
                break;
            case 'K':
                /*
                 * dropped if after C else K
                 */
                if (*(n - 1) != 'C')
                    key << 'K';
                break;
            case 'P':
                /*
                 * F if before H, else P
                 */
                key << (*(n + 1) == 'H' ?
                        (char) 'F' : (char) 'P');
                break;
            case 'Q':
                /*
                 * K
                 */
                key << 'K';
                break;
            case 'S':
                /*
                 * X in -SH-, -SIO- or -SIA- else S
                 */
                key << ((*(n + 1) == 'H' ||
                         (*(n + 1) == 'I' && (*(n + 2) == 'O' ||
                                              *(n + 2) == 'A')))
                        ? (char) 'X' : (char) 'S');
                break;
            case 'T':
                /*
                 * X in -TIA- or -TIO- else 0 (zero)
                 * before H else dropped if in -TCH-
                 * else T
                 */
                if (*(n + 1) == 'I' && (*(n + 2) == 'O' ||
                                        *(n + 2) == 'A'))
                    key << 'X';
                else if (*(n + 1) == 'H')
                    key << '0';
                else if (*(n + 1) != 'C' || *(n + 2) != 'H')
                    key << 'T';
                break;
            case 'V':
                /*
                 * F
                 */
                key << 'F';
                break;
            case 'W':
                /*
                 * W after a vowel, else dropped
                 */
            case 'Y':
                /*
                 * Y unless followed by a vowel
                 */
                if (vowel(*(n + 1)))
                    key << *n;
                break;
            case 'X':
                /*
                 * KS
                 */
                if (*(n - 1) == '\0')
                    key << 'S';
                else
                    key << "KS";        /* Insert K, then S */
                break;
            case 'Z':
                /*
                 * S
                 */
                key << 'S';
                break;
            }
        }
    }
}


//*****************************************************************************
// void Metaphone::addWord(char *word)
//
void
Metaphone::addWord(char *word)
{
    if (!dict)
    {
        dict = new Dictionary;
    }

    String	key;
    generateKey(word, key);

    if (key.length() == 0)
        return;
    String	*s = (String *) dict->Find(key);
    if (s)
    {
      //        if (mystrcasestr(s->get(), word) != 0)
            (*s) << ' ' << word;
    }
    else
    {
        dict->Add(key, new String(word));
    }
}
