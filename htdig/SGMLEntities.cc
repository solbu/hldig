//
// SGMLEntities.cc
//
// Implementation of SGMLEntities
// Translate SGML entities to ISO 8859 equivalents
//
//
#if RELEASE
static char RCSid[] = "$Id: SGMLEntities.cc,v 1.10.2.1 1999/11/24 02:06:47 grdetil Exp $";
#endif

#include "SGMLEntities.h"
#include "htString.h"
#include <ctype.h>
#include <stdlib.h>
#include "Configuration.h"
#include "StringMatch.h"

extern Configuration config;

static SGMLEntities	junk;

static struct
{
    char			*entity;
    unsigned char	equiv;
} entities[] =
  {
    { "lt",	      '<' } ,
    { "gt",	      '>' } ,
    { "amp",	      '&' } ,
    { "quot",	      '"' } ,
    { "trade",	      153 } , /* trade mark */
    { "nbsp",         160 } , /* non breaking space */
    { "iexcl",        161 } , /* inverted exclamation mark */
    { "cent",         162 } , /* cent sign */
    { "pound",        163 } , /* pound sign */
    { "curren",       164 } , /* currency sign */
    { "yen",          165 } , /* yen sign */
    { "brvbar",       166 } , /* broken vertical bar, (brkbar) */
    { "sect",         167 } , /* section sign */
    { "uml",          168 } , /* spacing diaresis */
    { "copy",         169 } , /* copyright sign */
    { "ordf",         170 } , /* feminine ordinal indicator */
    { "laquo",        171 } , /* angle quotation mark, left */
    { "not",          172 } , /* negation sign */
    { "shy",          173 } , /* soft hyphen */
    { "reg",          174 } , /* circled R registered sign */
    { "hibar",        175 } , /* spacing macron */
    { "deg",          176 } , /* degree sign */
    { "plusmn",       177 } , /* plus-or-minus sign */
    { "sup2",         178 } , /* superscript 2 */
    { "sup3",         179 } , /* superscript 3 */
    { "acute",        180 } , /* spacing acute (96) */
    { "micro",        181 } , /* micro sign */
    { "para",         182 } , /* paragraph sign */
    { "middot",       183 } , /* middle dot */
    { "cedil",        184 } , /* spacing cedilla */
    { "sup1",         185 } , /* superscript 1 */
    { "ordm",         186 } , /* masculine ordinal indicator */
    { "raquo",        187 } , /* angle quotation mark, right */
    { "frac14",       188 } , /* fraction 1/4 */
    { "frac12",       189 } , /* fraction 1/2 */
    { "frac34",       190 } , /* fraction 3/4 */
    { "iquest",       191 } , /* inverted question mark */
    { "Agrave",       192 } , /* capital A, grave accent */ 
    { "Aacute",       193 } , /* capital A, acute accent */ 
    { "Acirc",        194 } , /* capital A, circumflex accent */ 
    { "Atilde",       195 } , /* capital A, tilde */ 
    { "Auml",         196 } , /* capital A, dieresis or umlaut mark */ 
    { "Aring",        197 } , /* capital A, ring */ 
    { "AElig",        198 } , /* capital AE diphthong (ligature) */ 
    { "Ccedil",       199 } , /* capital C, cedilla */ 
    { "Egrave",       200 } , /* capital E, grave accent */ 
    { "Eacute",       201 } , /* capital E, acute accent */ 
    { "Ecirc",        202 } , /* capital E, circumflex accent */ 
    { "Euml",         203 } , /* capital E, dieresis or umlaut mark */ 
    { "Igrave",       205 } , /* capital I, grave accent */ 
    { "Iacute",       204 } , /* capital I, acute accent */ 
    { "Icirc",        206 } , /* capital I, circumflex accent */ 
    { "Iuml",         207 } , /* capital I, dieresis or umlaut mark */ 
    { "ETH",          208 } , /* capital Eth, Icelandic (Dstrok) */ 
    { "Ntilde",       209 } , /* capital N, tilde */ 
    { "Ograve",       210 } , /* capital O, grave accent */ 
    { "Oacute",       211 } , /* capital O, acute accent */ 
    { "Ocirc",        212 } , /* capital O, circumflex accent */ 
    { "Otilde",       213 } , /* capital O, tilde */ 
    { "Ouml",         214 } , /* capital O, dieresis or umlaut mark */ 
    { "times",        215 } , /* multiplication sign */ 
    { "Oslash",       216 } , /* capital O, slash */ 
    { "Ugrave",       217 } , /* capital U, grave accent */ 
    { "Uacute",       218 } , /* capital U, acute accent */ 
    { "Ucirc",        219 } , /* capital U, circumflex accent */ 
    { "Uuml",         220 } , /* capital U, dieresis or umlaut mark */ 
    { "Yacute",       221 } , /* capital Y, acute accent */ 
    { "THORN",        222 } , /* capital THORN, Icelandic */ 
    { "szlig",        223 } , /* small sharp s, German (sz ligature) */ 
    { "agrave",       224 } , /* small a, grave accent */ 
    { "aacute",       225 } , /* small a, acute accent */ 
    { "acirc",        226 } , /* small a, circumflex accent */ 
    { "atilde",       227 } , /* small a, tilde */
    { "auml",         228 } , /* small a, dieresis or umlaut mark */ 
    { "aring",        229 } , /* small a, ring */
    { "aelig",        230 } , /* small ae diphthong (ligature) */ 
    { "ccedil",       231 } , /* small c, cedilla */ 
    { "egrave",       232 } , /* small e, grave accent */ 
    { "eacute",       233 } , /* small e, acute accent */ 
    { "ecirc",        234 } , /* small e, circumflex accent */ 
    { "euml",         235 } , /* small e, dieresis or umlaut mark */ 
    { "igrave",       236 } , /* small i, grave accent */ 
    { "iacute",       237 } , /* small i, acute accent */ 
    { "icirc",        238 } , /* small i, circumflex accent */ 
    { "iuml",         239 } , /* small i, dieresis or umlaut mark */ 
    { "eth",          240 } , /* small eth, Icelandic */ 
    { "ntilde",       241 } , /* small n, tilde */ 
    { "ograve",       242 } , /* small o, grave accent */ 
    { "oacute",       243 } , /* small o, acute accent */ 
    { "ocirc",        244 } , /* small o, circumflex accent */ 
    { "otilde",       245 } , /* small o, tilde */ 
    { "ouml",         246 } , /* small o, dieresis or umlaut mark */ 
    { "divide",       247 } , /* division sign */
    { "oslash",       248 } , /* small o, slash */ 
    { "ugrave",       249 } , /* small u, grave accent */ 
    { "uacute",       250 } , /* small u, acute accent */ 
    { "ucirc",        251 } , /* small u, circumflex accent */ 
    { "uuml",         252 } , /* small u, dieresis or umlaut mark */ 
    { "yacute",       253 } , /* small y, acute accent */ 
    { "thorn",        254 } , /* small thorn, Icelandic */ 
    { "yuml",         255 } , /* small y, dieresis or umlaut mark */
    { 0, 0 }
  };

//*****************************************************************************
SGMLEntities::SGMLEntities()
{
    trans = new Dictionary();
    init();
}


//*****************************************************************************
SGMLEntities::~SGMLEntities()
{
    trans->Release();
    delete trans;
}


//*****************************************************************************
unsigned char
SGMLEntities::translate(char *entity)
{
    if (!entity || !*entity)
	return ' ';
    if (junk.trans->Exists(entity))
    {
	return (unsigned char) ((int) (*junk.trans)[entity]);
    }
    else if (*entity == '#' && isdigit(entity[1]))
    {
	//
	// This looks like a numeric entity.  That's fine.
	//
	return atoi(entity + 1);
    }
    else
    {
	return ' ';	// Unrecognized entity.  Change it into a space...
    }
}

//*****************************************************************************
void
SGMLEntities::init()
{
    for (int i = 0; entities[i].entity; i++)
    {
	trans->Add(entities[i].entity, (Object *) entities[i].equiv);
    }
}

//*****************************************************************************
// This method does the same as the translate method, but it will also advance
// the character pointer to the next character after the entity.  This will
// allow us to encapsulate the entity rules inside this routine.  (There
// was some confusion on how entities are terminated...)
//
unsigned char
SGMLEntities::translateAndUpdate(unsigned char *&entityStart)
{
    String		entity;
    unsigned char	*orig = entityStart;
    static int		translate_quot = config.Boolean("translate_quot");
    static int		translate_amp = config.Boolean("translate_amp");
    static int		translate_lt_gt = config.Boolean("translate_lt_gt");
    
    if (*entityStart == '&')
	entityStart++;		// Don't need the '&' that starts the entity
    while ((isalnum(*entityStart) || *entityStart == '.' ||
	    *entityStart == '-' || *entityStart == '#') &&
	   entity.length() < 10)
      {
	entity << *entityStart++;
      }

    if ( !translate_quot )
      {
	//
	// Do NOT translate entities for '"' (quote).
	//

	static StringMatch *quoteMatch = 0;
	if (!quoteMatch)
	  {
	    quoteMatch = new StringMatch();
	    quoteMatch->Pattern("quot|#34");
	  }

	if (quoteMatch->hasPattern() &&
	    quoteMatch->FindFirstWord(entity.get()) >= 0)
	  {
	    entityStart = orig + 1;
	    return '&';
	  }
      }

    if ( !translate_amp )
      {
	//
	// Do NOT translate entities for '&' since they can
	// occur in code samples that might end up in an excerpt.
	//

	static StringMatch *ampMatch = 0;
	if (!ampMatch)
	  {
	    ampMatch = new StringMatch();
	    ampMatch->Pattern("amp|#38");
	  }
	
	if (ampMatch->hasPattern() &&
	    ampMatch->FindFirstWord(entity.get()) >= 0)
	  {
	    entityStart = orig + 1;
	    return '&';
	  }
      }

    if ( !translate_lt_gt )
      {
	//
	// Do NOT translate entities for '<' and '>' since they can
	// occur in code samples that might end up in an excerpt.
	//

	static StringMatch *ltgtMatch = 0;
	if (!ltgtMatch)
	  {
	    ltgtMatch = new StringMatch();
	    ltgtMatch->Pattern("lt|#60|gt|#62");
	  }

	if (ltgtMatch->hasPattern() &&
	    ltgtMatch->FindFirstWord(entity.get()) >= 0)
	  {
	    entityStart = orig + 1;
	    return '&';
	  }
      }

    if (entity.length() >= 10)
      {
	//
	// This must be a bogus entity.  It can't be more than 10 characters
	// long.  Well, just assume it was an error and return just the '&'.
	//
	entityStart = orig + 1;
	return '&';
    }
    
    if (*entityStart == ';')
	entityStart++;		// A final ';' is used up.
    unsigned char e = translate(entity);
    if (e == ' ' && strncmp((char *)orig, "&#32", 4) != 0)
    {
	entityStart = orig + 1;	// Catch unrecognized entities...
	return '&';
    }
    return e;
}
