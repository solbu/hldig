//
// Configuration.h
//
// Configuration: This class provides an object lookup table.  Each object 
//                in the Configuration is indexed with a string.  The objects 
//                can be returned by mentioning their string index. Values may
//                include files with `/path/to/file` or other configuration
//                variables with ${variable}
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Configuration.h,v 1.6.2.1 1999/11/21 15:24:52 vadim Exp $
//

#ifndef	_Configuration_h_
#define	_Configuration_h_

#include "Dictionary.h"
#include "htconfig.h"
#include "htString.h"
#include "URL.h"



struct ConfigDefaults
{
  char	*name;			// Name of the attribute
  char	*value;			// Default value
  char	*type;			// Type of the value (string, integer, boolean)
  char	*programs;		// White separated list of programs/modules using this attribute
  char	*example;		// Example usage of the attribute (HTML)
  char	*description;		// Long description of the attribute (HTML)
};


class Configuration : public Object
{
public:
    //
    // Construction/Destruction
    //
    Configuration();

    Configuration(const Configuration& config) :
      dcGlobalVars(config.dcGlobalVars),
      dcServers(config.dcServers),
      dcUrls(config.dcUrls),
      separators(config.separators)
      {
        allow_multiple = config.allow_multiple;
      }

    ~Configuration() {}

    //
    // Adding and deleting items to and from the Configuration
    //
    void		Add(const String& name, const String& value);
    void		Add(const String& str);
    void		Add(char *name, char *value, Configuration *aList);
    int			Remove(const String& name);

    //
    // Let the Configuration know how to parse name value pairs
    //
    void		NameValueSeparators(const String& s);
	
    //
    // We need some way of reading in the database from a configuration file
    //
    int			Read(const String& filename);

    //
    // Searching can be done with the Find() member or the array indexing
    // operator
    //
    const String	Find(const String& name) const;
    const String	Find(const char *blockName, const char *name, const char *value) const;
    const String	Find(URL *aUrl, const char *value) const;
    const String	operator[](const String& name) const;
    int		Value(const String& name, int default_value = 0) const;
    double	Double(const String& name, double default_value = 0) const;
    int		Boolean(const String& name, int default_value = 0) const;
    int		Value(char *blockName,char *name,char *value,int default_value = 0);
    double	Double(char *blockName,char *name,char *value,double default_value = 0);
    int		Boolean(char *blockName,char *name,char *value,int default_value = 0);
    int		Value(URL *aUrl,char *value,int default_value = 0);
    double	Double(URL *aUrl,char *value,double default_value = 0);
    int		Boolean(URL *aUrl,char *value,int default_value = 0);
    Object     *Get_Object(char *name);

    //
    // Read defaults from an array
    //
    void		Defaults(const ConfigDefaults *);

protected:
    Dictionary		dcGlobalVars;
    Dictionary          dcServers;
    Dictionary          dcUrls;
    String		separators;
    int			allow_multiple;
};

#endif
