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
// $Id: Configuration.h,v 1.5 1999/09/24 10:29:03 loic Exp $
//

#ifndef	_Configuration_h_
#define	_Configuration_h_

#include "Dictionary.h"
#include "htconfig.h"
#include "htString.h"

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
      dict(config.dict),
      separators(config.separators)
      {
	allow_multiple = config.allow_multiple;
      }
    ~Configuration() {}

    //
    // Adding and deleting items to and from the Configuration
    //
    void		Add(const char *name, const char *value);
    void		Add(const char *str);
    int			Remove(const char *name);

    //
    // Let the Configuration know how to parse name value pairs
    //
    void		NameValueSeparators(const char *s);
	
    //
    // We need some way of reading in the database from a configuration file
    //
    int			Read(const String& filename);

    //
    // Searching can be done with the Find() member or the array indexing
    // operator
    //
    const String	Find(const char *name) const;
    const String	operator[](const char *name) const;
    int			Value(const char *name, int default_value = 0) const;
    double		Double(const char *name, double default_value = 0) const;
    int			Boolean(const char *name, int default_value = 0) const;

    //
    // Read defaults from an array
    //
    void		Defaults(const ConfigDefaults *);

protected:
    Dictionary		dict;
    String		separators;
    int			allow_multiple;
};

#endif
