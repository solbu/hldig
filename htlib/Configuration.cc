//
// Configuration.cc
//
// Configuration: This class provides an object lookup table.  Each object 
//                in the Configuration is indexed with a string.  The objects 
//                can be returned by mentioning their string index. Values may
//                include files with `/path/to/file` or other configuration
//                variables with ${variable}
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Configuration.cc,v 1.17 2002/02/01 22:49:33 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include "Configuration.h"
#include "htString.h"
#include "ParsedString.h"

#include <stdlib.h>
#include <ctype.h>
#include <locale.h>


//*********************************************************************
// Configuration::Configuration()
//
Configuration::Configuration() : separators("=:"), allow_multiple(0)
{
}


//*********************************************************************
// void Configuration::NameValueSeparators(char *s)
//
void Configuration::NameValueSeparators(const String& s)
{
    separators = s;
}


//*********************************************************************
//   Add an entry to the configuration table.
//
void Configuration::Add(const String& str_arg)
{
    const char* str = str_arg;
    String	name, value;
	
    while (str && *str)
    {
        while (isspace(*str))
            str++;
        name = 0;
        if (!isalpha(*str))
            break;
        // Some isalnum() implementations don't allow all the letters that
        // isalpha() does, e.g. accented ones.  They're not POSIX.2 compliant
        // but we won't punish them with an infinite loop...
        if (!isalnum(*str))
            break;
        while (isalnum(*str) || *str == '-' || *str == '_')
            name << *str++;

        name.lowercase();
		
        //
        // We have the name.  Let's see if we will get a value
        //
        while (isspace(*str))
            str++;
        if (!*str)
        {
            //
            // End of string.  We need to store the name as a boolean TRUE
            //
            Add(name, "true");
            return;
        }

        if (!strchr((char*)separators, *str))
        {
            //
            // We are now at a new name.  The previous one needs to be set
            // to boolean TRUE
            //
            Add(name, "true");
            continue;
        }

        //
        // We now need to deal with the value
        //
        str++;			// Skip the separator
        while (isspace(*str))
            str++;
        if (!*str)
        {
            //
            // End of string reached.  The value must be blank
            //
            Add(name, "");
            break;
        }
        value = 0;
        if (*str == '"')
        {
            //
            // Ah!  A quoted value.  This should be easy to deal with...
            // (Just kidding!)
            //
            str++;
            while (*str && *str != '"')
            {
                value << *str++;
            }
            Add(name, value);
            if (*str == '"')
                str++;
            continue;
        }
        else if (*str == '\'')
        {
            // A single quoted value.
            str++;
            while (*str && *str != '\'')
            {
                value << *str++;
            }
            Add(name, value);
            if (*str == '\'')
                str++;
            continue;
        }
        else
        {
            //
            // A non-quoted string.  This string will terminate at the
            // next blank
            //
            while (*str && !isspace(*str))
            {
                value << *str++;
            }
            Add(name, value);
            continue;
        }
    }
}


//*********************************************************************
//   Add an entry to the configuration table, without allowing variable
//   or file expansion of the value.
//
void Configuration::Add(const String& name, const String& value)
{
    String	escaped;
    const char	*s = value.get();
    while (*s)
    {
        if (strchr("$`\\", *s))
            escaped << '\\';
        escaped << *s++;
    }
    ParsedString	*ps = new ParsedString(escaped);
    dcGlobalVars.Add(name, ps);
}


//*********************************************************************
//   Add an entry to the configuration table, allowing parsing for variable
//   or file expansion of the value.
//
void Configuration::AddParsed(const String& name, const String& value)
{
    ParsedString	*ps = new ParsedString(value);
    if (mystrcasecmp(name, "locale") == 0)
    {
        String str(setlocale(LC_ALL, ps->get(dcGlobalVars)));
        ps->set(str);

        //
        // Set time format to standard to avoid sending If-Modified-Since
        // http headers in native format which http servers can't
        // understand
        //
        setlocale(LC_TIME, "C");
    }
    dcGlobalVars.Add(name, ps);
}


//*********************************************************************
//   Remove an entry from both the hash table and from the list of keys.
//
int Configuration::Remove(const String& name)
{
    return dcGlobalVars.Remove(name);
}


//*********************************************************************
// char *Configuration::Find(const char *name) const
//   Retrieve a variable from the configuration database.  This variable
//   will be parsed and a new String object will be returned.
//
const String Configuration::Find(const String& name) const
{
    ParsedString	*ps = (ParsedString *) dcGlobalVars[name];
    if (ps)
    {
        return ps->get(dcGlobalVars);
    }
    else
    {
#ifdef DEBUG
        fprintf (stderr, "Could not find configuration option %s\n", (const char*)name);
#endif
        return 0;
    }
}

//*********************************************************************
Object *Configuration::Get_Object(char *name) {
return dcGlobalVars[name];
}


//*********************************************************************
//
int Configuration::Value(const String& name, int default_value) const
{
    return Find(name).as_integer(default_value);
}


//*********************************************************************
//
double Configuration::Double(const String& name, double default_value) const
{
    return Find(name).as_double(default_value);
}


//*********************************************************************
// int Configuration::Boolean(char *name, int default_value)
//
int Configuration::Boolean(const String& name, int default_value) const
{
    int		value = default_value;
    const String s = Find(name);
    if (s[0])
    {
        if (s.nocase_compare("true") == 0 ||
            s.nocase_compare("yes") == 0 ||
            s.nocase_compare("1") == 0)
            value = 1;
        else if (s.nocase_compare("false") == 0 ||
                 s.nocase_compare("no") == 0 ||
                 s.nocase_compare("0") == 0)
            value = 0;
    }

    return value;
}


//*********************************************************************
//
const String Configuration::operator[](const String& name) const
{
    return Find(name);
}


//*********************************************************************
//
int Configuration::Read(const String& filename)
{
    FILE* in = fopen((const char*)filename, "r");
 
    if(!in) {
      fprintf(stderr, "Configuration::Read: cannot open %s for reading : ", (const char*)filename);
      perror("");
      return NOTOK;
    }

#define CONFIG_BUFFER_SIZE (50*1024) 
     //
     // Make the line buffer large so that we can read long lists of start
     // URLs.
     //
     char	buffer[CONFIG_BUFFER_SIZE + 1];
     char	*current;
     String	line;
     String	name;
     char	*value;
     int         len;
     while (fgets(buffer, CONFIG_BUFFER_SIZE, in))
     {
         line << buffer;
         line.chop("\r\n");
         if (line.last() == '\\')
         {
             line.chop(1);
             continue;			// Append the next line to this one
         }
 
         current = line.get();
         if (*current == '#' || *current == '\0')
         {
             line = 0;
             continue;			// Comments and blank lines are skipped
         }
 
         name = strtok(current, ": =\t");
         value = strtok(0, "\r\n");
         if (!value)
             value = "";			// Blank value
 
         //
         // Skip any whitespace before the actual text
         //
         while (*value == ' ' || *value == '\t')
             value++;
 	len = strlen(value) - 1;
 	//
 	// Skip any whitespace after the actual text
 	//
	while (len >= 0 && (value[len] == ' ' || value[len] == '\t'))
 	  {
 	    value[len] = '\0';
 	    len--;
 	  }
 
 	if (mystrcasecmp((char*)name, "include") == 0)
 	{
 	    ParsedString	ps(value);
 	    String		str(ps.get(dcGlobalVars));
 	    if (str[0] != '/')		// Given file name not fully qualified
 	    {
 		str = filename;		// so strip dir. name from current one
 		len = str.lastIndexOf('/') + 1;
 		if (len > 0)
 		    str.chop(str.length() - len);
 		else
 		    str = "";		// No slash in current filename
 		str << ps.get(dcGlobalVars);
 	    }
 	    Read(str);
 	    line = 0;
 	    continue;
 	}
 
         AddParsed(name, value);
         line = 0;
     }
     fclose(in);
     return OK;
}


//*********************************************************************
// void Configuration::Defaults(ConfigDefaults *array)
//
void Configuration::Defaults(const ConfigDefaults *array)
{
    for (int i = 0; array[i].name; i++)
    {
        AddParsed(array[i].name, array[i].value);
    }
}

