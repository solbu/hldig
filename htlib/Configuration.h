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
// $Id: Configuration.h,v 1.4 1999/09/11 05:03:51 ghutchis Exp $
//

#ifndef	_Configuration_h_
#define	_Configuration_h_

#include "Dictionary.h"
#include "htconfig.h"

class String;


struct ConfigDefaults
{
    char	*name;
    char	*value;
};


class Configuration : public Object
{
public:
    //
    // Construction/Destruction
    //
    Configuration();
    ~Configuration();

    //
    // Adding and deleting items to and from the Configuration
    //
    void		Add(char *name, char *value);
    void		Add(char *str);
    int			Remove(char *name);

    //
    // Let the Configuration know how to parse name value pairs
    //
    void		NameValueSeparators(char *s);
	
    //
    // We need some way of reading in the database from a configuration file
    //
    int			Read(char *filename);

    //
    // Searching can be done with the Find() member or the array indexing
    // operator
    //
    char		*Find(char *name);
    char		*operator[](char *name);
    int			Value(char *name, int default_value = 0);
    double		Double(char *name, double default_value = 0);
    int			Boolean(char *name, int default_value = 0);

    //
    // Read defaults from an array
    //
    void		Defaults(ConfigDefaults *);

protected:
    Dictionary		dict;
    String		*separators;
    int			allow_multiple;
};


#endif
