
#include"HtConfiguration.h"
#include "ParsedString.h"
#include <stdlib.h>


//********************************************************************
//  Add complex entry to the configuration
//
void
HtConfiguration::Add(char *name, char *value, HtConfiguration *aList) {

  if (strcmp("url",name)==0) {  //add URL entry
    URL tmpUrl(strdup(value));
    if (Dictionary *paths=(Dictionary *)dcUrls[tmpUrl.host()]) {
      paths->Add(tmpUrl.path(),aList);
    } else {
      paths=new Dictionary();
      paths->Add(tmpUrl.path(),aList);
      dcUrls.Add(tmpUrl.host(),paths);
      //      return;
    }
  } else 
    if (strcmp("server",name)==0) {
      dcServers.Add(value,aList);
      //      return;
    } else {

      Object *treeEntry=dcGlobalVars[name];
      if (treeEntry!=NULL) {
	((Dictionary *)treeEntry)->Add(value,aList);
      } else {
	treeEntry=new Dictionary(16);
	((Dictionary *)treeEntry)->Add(value,aList);
	dcGlobalVars.Add(name, treeEntry);
      }
    }
}

//*********************************************************************
const String HtConfiguration::Find(const char *blockName,const char *name,const char *value) const
{
  if (!(blockName && name && value) )
    return NULL;
  union {
    void      *ptr;
    Object *obj;
    Dictionary *dict;
    HtConfiguration *conf;
  } tmpPtr;
  String chr;
  
  if (strcmp("url",blockName)==0) { // URL needs special compare
    URL paramUrl(name);     // split URL to compare separatly host and path
    chr=Find(&paramUrl,value);
    if (chr[0]!=0) {
      return chr;
    }
  } else  // end "url"
    if (strcmp("server",blockName)==0) {
      tmpPtr.obj=dcServers[name];
      if (tmpPtr.obj) {
	chr=tmpPtr.conf->Find(value);
	if (chr[0]!=0)
	  return chr;
      }
    }
    else { // end "server"
      tmpPtr.obj=tmpPtr.dict->Find(name);
      if (tmpPtr.ptr) {
	chr=tmpPtr.conf->Find(value);
	if (tmpPtr.ptr)
	  return chr;
      }
    }
 
  // If this parameter is defined in global then return it
  chr=Find(value);
  if (chr[0]!=0) {
    return chr;
  }
#ifdef DEBUG
  cerr << "Could not find configuration option " << blockName<<":"
       <<name<<":"<<value<< "\n";
#endif
  return NULL;
}

//*********************************************************************
//
const String HtConfiguration::Find(URL *aUrl, const char *value) const
{
 if (!aUrl)
    return NULL;
  Dictionary *tmpPtr=(Dictionary *)dcUrls.Find( aUrl->host() );
  if (tmpPtr) {       // We've got such host in config
    tmpPtr->Start_Get();
    // Try to find best matched URL
    //
    struct {
      Object *obj;
      unsigned int  len;
    } candidate;
    candidate.len=0; 
    String candValue;
    // Begin competition: which URL is better?
    //
    // TODO: move this loop into Dictionary
    // (or create Dictionary::FindBest ?)
    // or make url list sorted ?
    // or implement abstract Dictionary::Compare?
    char *strParamUrl=aUrl->path();
    while ( char *confUrl=tmpPtr->Get_Next() ) {   
      if (strncmp(confUrl,strParamUrl,strlen(confUrl))==0 
	  && (strlen(confUrl)>=candidate.len))  {
	// it seems this URL match better
	candidate.obj=tmpPtr->Find(confUrl);
	// but does it has got necessery parameter?
	candValue=((HtConfiguration *)candidate.obj)->Find(value);
	if (candValue[0]!=0) {
	  // yes, it has! We've got new candidate.
	  candidate.len=strlen(candValue);
	}
      }
    }
    if (candidate.len>0) {
      return ParsedString(candValue).get(dcGlobalVars);
    }
       
  }
  return Find(value);
}


//*********************************************************************
int HtConfiguration::Value(char *blockName,char *name,char *value,
			 int default_value = 0) {
int retValue=default_value;
String tmpStr=Find(blockName,name,value);
 if (tmpStr[0]!=0) {
   retValue=atoi(tmpStr.get());
 }
return retValue;
}
//*********************************************************************
double HtConfiguration::Double(char *blockName,char *name,char *value,
				double default_value = 0) {
double retValue=default_value;
String tmpStr=Find(blockName,name,value);
 if (tmpStr[0]!=0) {
   retValue=atof(tmpStr.get());
 }
return retValue;
}
//*********************************************************************
int HtConfiguration::Boolean(char *blockName,char *name,char *value,
				 int default_value = 0) {
int retValue=default_value;
String tmpStr=Find(blockName,name,value);
 if (tmpStr[0]!=0) {
        if (mystrcasecmp(tmpStr, "true") == 0 ||
            mystrcasecmp(tmpStr, "yes") == 0 ||
            mystrcasecmp(tmpStr, "1") == 0)
            retValue = 1;
        else if (mystrcasecmp(tmpStr, "false") == 0 ||
                 mystrcasecmp(tmpStr, "no") == 0 ||
                 mystrcasecmp(tmpStr, "0") == 0)
            retValue = 0;

 }
return retValue;
}
//*********************************************************************
//*********************************************************************
int HtConfiguration::Value(URL *aUrl,char *value,
			 int default_value = 0) {
int retValue=default_value;
String tmpStr=Find(aUrl,value);
 if (tmpStr[0]!=0) {
   retValue=atoi(tmpStr.get());
 }
return retValue;
}
//*********************************************************************
double HtConfiguration::Double(URL *aUrl,char *value,
				double default_value = 0) {
double retValue=default_value;
String tmpStr=Find(aUrl,value);
 if (tmpStr[0]!=0) {
   retValue=atof(tmpStr.get());
 }
return retValue;
}
//*********************************************************************
int HtConfiguration::Boolean(URL *aUrl,char *value,
				 int default_value = 0) {
int retValue=default_value;
String tmpStr=Find(aUrl,value);
 if (tmpStr[0]!=0) {
        if (mystrcasecmp(tmpStr, "true") == 0 ||
            mystrcasecmp(tmpStr, "yes") == 0 ||
            mystrcasecmp(tmpStr, "1") == 0)
            retValue = 1;
        else if (mystrcasecmp(tmpStr, "false") == 0 ||
                 mystrcasecmp(tmpStr, "no") == 0 ||
                 mystrcasecmp(tmpStr, "0") == 0)
            retValue = 0;

 }
return retValue;
}
