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
// $Id: Configuration.h,v 1.6.2.9 2000/03/21 00:37:20 ghutchis Exp $
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
  char	*programs;		// Whitespace separated list of programs/modules using this attribute
  char	*block;			// Configuration block this can be used in (can be blank)
  char	*version;		// Version that introduced the attribute
  char	*category;		// Attribute category (to split documentation)
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
#ifndef SWIG
    Configuration(const Configuration& config) :
	dcGlobalVars(config.dcGlobalVars),
	separators(config.separators)
      {
        allow_multiple = config.allow_multiple;
      }
#endif /* SWIG */
    ~Configuration() {}

    //
    // Adding and deleting items to and from the Configuration
    //
#ifndef SWIG
    void		Add(const String& str);
#endif /* SWIG */
    void		Add(const String& name, const String& value);
    void		AddParsed(const String& name, const String& value);
    int			Remove(const String& name);

    //
    // Let the Configuration know how to parse name value pairs
    //
    void		NameValueSeparators(const String& s);
	
    //
    // We need some way of reading in the database from a configuration file
    //
    virtual int         Read(const String& filename);

    //
    // Searching can be done with the Find() member or the array indexing
    // operator
    //
    const String	Find(const String& name) const;
#ifndef SWIG
    const String	operator[](const String& name) const;
#endif /* SWIG */
    int		Value(const String& name, int default_value = 0) const;
    double	Double(const String& name, double default_value = 0) const;
    int		Boolean(const String& name, int default_value = 0) const;
    Object     *Get_Object(char *name);

    //
    // Read defaults from an array
    //
    void		Defaults(const ConfigDefaults *array);

protected:
    Dictionary		dcGlobalVars;
    String		separators;
    int			allow_multiple;
};

#endif
