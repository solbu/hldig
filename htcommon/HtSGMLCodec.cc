//
// HtSGMLCodec.cc
//
// HtSGMLCodec: A Specialized HtWordCodec class to convert between SGML 
//              ISO 8859-1 entities and high-bit characters.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: HtSGMLCodec.cc,v 1.1.2.1 2000/02/13 03:41:47 ghutchis Exp $
//

#include "HtSGMLCodec.h"
#include "defaults.h" // For "config"

// Constructor: parses the appropriate parameters using the
// encapsulated HtWordCodec class.
// Only used in privacy.
HtSGMLCodec::HtSGMLCodec()
{
  StringList *myFromList = new StringList();
  StringList *myToList = new StringList();
  String myFromString(1443); // Full list including numeric entities...
  
  // Is this really the best way to do this?
  myFromString = "&nbsp;|&iexcl;|&cent;|&pound;|&curren;|&yen;|";
  myFromString << "&brvbar;|&sect;|&uml;|&copy;|&ordf;|&laquo;|";
  myFromString << "&not;|&shy;|&reg;|&hibar;|&deg;|&plusmn;|&sup2;|";
  myFromString << "&sup3;|&acute;|&micro;|&para;|&middot;|&cedil;|";
  myFromString << "&sup1;|&ordm;|&raquo;|&frac14;|&frac12;|";
  myFromString << "&frac34;|&iquest;|&Agrave;|&Aacute;|&Acirc;|";
  myFromString << "&Atilde;|&Auml;|&Aring;|&AElig;|&Ccedil;|";
  myFromString << "&Egrave;|&Eacute;|&Ecirc;|&Euml;|&Igrave;|";
  myFromString << "&Iacute;|&Icirc;|&Iuml;|&ETH;|&Ntilde;|";
  myFromString << "&Ograve;|&Oacute;|&Ocirc;|&Otilde;|&Ouml;|";
  myFromString << "&times;|&Oslash;|&Ugrave;|&Uacute;|&Ucirc;|";
  myFromString << "&Uuml;|&Yacute;|&THORN;|&szlig;|&agrave;|";
  myFromString << "&aacute;|&acirc;|&atilde;|&auml;|&aring;|";
  myFromString << "&aelig;|&ccedil;|&egrave;|&eacute;|&ecirc;|";
  myFromString << "&euml;|&igrave;|&iacute;|&icirc;|&iuml;|&eth;|";
  myFromString << "&ntilde;|&ograve;|&oacute;|&ocirc;|&otilde;|";
  myFromString << "&ouml;|&divide;|&oslash;|&ugrave;|&uacute;|";
  myFromString << "&ucirc;|&uuml;|&yacute;|&thorn;|&yuml;";

  myFromList->Create(myFromString, '|');

  for (int i = 160; i <= 255; i++)
    {
      String temp = 0;
      temp << (char) i;
      myToList->Add(temp);
      //      myToList->Add(temp);
    }

  // Now let's take care of the low-bit characters with encodings.
  myFromList->Add("&quot;");
  myToList->Add("\"");
  //      myFromList->Add("&#34;");
  //      myToList->Add("\"");

  myFromList->Add("&amp;");
  myToList->Add("&");
  //      myFromList->Add("&#34;");
  //      myToList->Add("&");

  myFromList->Add("&lt;");
  myToList->Add("<");
  //      myFromList->Add("&#60;");
  //      myToList->Add("<");
  myFromList->Add("&gt;");
  myToList->Add(">");
  //      myFromList->Add("&#62;");
  //      myToList->Add(">");

  myWordCodec = new HtWordCodec(myFromList, myToList, '|');
}


HtSGMLCodec::~HtSGMLCodec()
{
  delete myWordCodec;
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
