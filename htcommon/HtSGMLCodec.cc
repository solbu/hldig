//
// HtSGMLCodec.cc
//
// HtSGMLCodec: A Specialized HtWordCodec class to convert between SGML 
//              ISO 8859-1 entities and high-bit characters.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: HtSGMLCodec.cc,v 1.4 2003/06/24 20:05:44 nealr Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "HtSGMLCodec.h"
#include "HtConfiguration.h"

// Constructor: parses the appropriate parameters using the
// encapsulated HtWordCodec class.
// Only used in privacy.
HtSGMLCodec::HtSGMLCodec()
{
  HtConfiguration* config= HtConfiguration::config();
  int translate_latin1 = config->Boolean("translate_latin1", 1);
  StringList *myTextFromList = new StringList(); // For &foo;
  StringList *myNumFromList = new StringList(); // For &#nnn;
  StringList *myToList = new StringList();
  String myTextFromString(770); // Full text list
  
  // Is this really the best way to do this?
 if (!translate_latin1 )
 {
  myTextFromString = "&nbsp;";
 }
 else
 {
  myTextFromString = "&nbsp;|&iexcl;|&cent;|&pound;|&curren;|&yen;|&brvbar;|&sect;|";
  myTextFromString << "&uml;|&copy;|&ordf;|&laquo;|&not;|&shy;|&reg;|&macr;|&deg;|";
  myTextFromString << "&plusmn;|&sup2;|&sup3;|&acute;|&micro;|&para;|&middot;|&cedil;|";
  myTextFromString << "&sup1;|&ordm;|&raquo;|&frac14;|&frac12;|&frac34;|&iquest;|&Agrave;|";
  myTextFromString << "&Aacute;|&Acirc;|&Atilde;|&Auml;|&Aring;|&AElig;|&Ccedil;|&Egrave;|";
  myTextFromString << "&Eacute;|&Ecirc;|&Euml;|&Igrave;|&Iacute;|&Icirc;|&Iuml;|&ETH;|";
  myTextFromString << "&Ntilde;|&Ograve;|&Oacute;|&Ocirc;|&Otilde;|&Ouml;|&times;|&Oslash;|";
  myTextFromString << "&Ugrave;|&Uacute;|&Ucirc;|&Uuml;|&Yacute;|&THORN;|&szlig;|&agrave;|";
  myTextFromString << "&aacute;|&acirc;|&atilde;|&auml;|&aring;|&aelig;|&ccedil;|&egrave;|";
  myTextFromString << "&eacute;|&ecirc;|&euml;|&igrave;|&iacute;|&icirc;|&iuml;|&eth;|";
  myTextFromString << "&ntilde;|&ograve;|&oacute;|&ocirc;|&otilde;|&ouml;|&divide;|&oslash;|";
  myTextFromString << "&ugrave;|&uacute;|&ucirc;|&uuml;|&yacute;|&thorn;|&yuml;";
 }

  myTextFromList->Create(myTextFromString, '|');

  for (int i = 160; i <= 255; i++)
    {
      String temp = 0;
      temp << (char) i;
      myToList->Add(temp);

      temp = 0;
      temp << "&#" << i << ";";
      myNumFromList->Add(temp);
      if (!translate_latin1 )
	break;
    }

  // Now let's take care of the low-bit characters with encodings.
  myTextFromList->Add("&quot;");
  myToList->Add("\"");
  myNumFromList->Add("&#34;");

  myTextFromList->Add("&amp;");
  myToList->Add("&");
  myNumFromList->Add("&#38;");

  myTextFromList->Add("&lt;");
  myToList->Add("<");
  myNumFromList->Add("&#60;");

  myTextFromList->Add("&gt;");
  myToList->Add(">");
  myNumFromList->Add("&#62;");

  myTextWordCodec = new HtWordCodec(myTextFromList, myToList, '|');
  myNumWordCodec = new HtWordCodec(myNumFromList, myToList, '|');
}


HtSGMLCodec::~HtSGMLCodec()
{
  delete myTextWordCodec;
  delete myNumWordCodec;
}


// Supposedly used as HtSGMLCodec::instance()->ErrMsg()
// to check if HtWordCodec liked what was fed.
String& HtSGMLCodec::ErrMsg()
{
  return myErrMsg;
}


// Canonical singleton interface.
HtSGMLCodec *
HtSGMLCodec::instance()
{
  static HtSGMLCodec *_instance = 0;

  if (_instance == 0)
  {
    _instance = new HtSGMLCodec();
  }

  return _instance;
}

// End of HtSGMLCodec.cc
