//
// HtSGMLCodec.cc
//
// A Specialized HtWordCodec class to convert between SGML ISO 8859-1
// entities and high-bit characters.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
//
#if RELEASE
static char	RCSid[] = "$Id: HtSGMLCodec.cc,v 1.1 1999/03/14 03:15:41 ghutchis Exp $";
#endif

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
  myFromString.Append("&brvbar;|&sect;|&uml;|&copy;|&ordf;|&laquo;|");
  myFromString.Append("&not;|&shy;|&reg;|&hibar;|&deg;|&plusmn;|&sup2;|");
  myFromString.Append("&sup3;|&acute;|&micro;|&para;|&middot;|&cedil;|");
  myFromString.Append("&sup1;|&ordm;|&raquo;|&frac14;|&frac12;|");
  myFromString.Append("&frac34;|&iquest;|&Agrave;|&Aacute;|&Acirc;|");
  myFromString.Append("&Atilde;|&Auml;|&Aring;|&AElig;|&Ccedil;|");
  myFromString.Append("&Egrave;|&Eacute;|&Ecirc;|&Euml;|&Igrave;|");
  myFromString.Append("&Iacute;|&Icirc;|&Iuml;|&ETH;|&Ntilde;|");
  myFromString.Append("&Ograve;|&Oacute;|&Ocirc;|&Otilde;|&Ouml;|");
  myFromString.Append("&times;|&Oslash;|&Ugrave;|&Uacute;|&Ucirc;|");
  myFromString.Append("&Uuml;|&Yacute;|&THORN;|&szlig;|&agrave;|");
  myFromString.Append("&aacute;|&acirc;|&atilde;|&auml;|&aring;|");
  myFromString.Append("&aelig;|&ccedil;|&egrave;|&eacute;|&ecirc;|");
  myFromString.Append("&euml;|&igrave;|&iacute;|&icirc;|&iuml;|&eth;|");
  myFromString.Append("&ntilde;|&ograve;|&oacute;|&ocirc;|&otilde;|");
  myFromString.Append("&ouml;|&divide;|&oslash;|&ugrave;|&uacute;|");
  myFromString.Append("&ucirc;|&uuml;|&yacute;|&thorn;|&yuml;");

  myFromList->Create(myFromString, '|');

  for (int i = 160; i <= 255; i++)
    {
      String temp = 0;
      temp << (char) i;
      myToList->Add(temp);
      //      myToList->Add(temp);
    }

  if (config.Boolean("translate_quot"))
    {
      myFromList->Add("&quot;");
      myToList->Add("\"");
      //      myFromList->Add("&#34;");
      //      myToList->Add("\"");
    }

  if (config.Boolean("translate_amp"))
    {
      myFromList->Add("&amp;");
      myToList->Add("&");
      //      myFromList->Add("&#34;");
      //      myToList->Add("&");
    }

  if (config.Boolean("translate_lt_gt"))
    {
      myFromList->Add("&lt;");
      myToList->Add("<");
      //      myFromList->Add("&#60;");
      //      myToList->Add("<");
      myFromList->Add("&gt;");
      myToList->Add(">");
      //      myFromList->Add("&#62;");
      //      myToList->Add(">");
    }

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
