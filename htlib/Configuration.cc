//
// Configuration.cc
//
// Implementation of the Configuration class
//
// $Log: Configuration.cc,v $
// Revision 1.1  1997/02/03 17:11:04  turtle
// Initial revision
//
//
#if RELEASE
static char	RCSid[] = "$Id: Configuration.cc,v 1.1 1997/02/03 17:11:04 turtle Exp $";
#endif

#include "Configuration.h"
#include "String.h"
#include "ParsedString.h"
#include <fstream.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>


//*********************************************************************
// Configuration::Configuration()
//
Configuration::Configuration()
{
	separators = new String("=:");
	allow_multiple = 0;
}


//*********************************************************************
// Configuration::~Configuration()
//
Configuration::~Configuration()
{
	delete separators;
}


//*********************************************************************
// void Configuration::NameValueSeparators(char *s)
//
void Configuration::NameValueSeparators(char *s)
{
	delete separators;
	separators = new String(s);
}


//*********************************************************************
// void Configuration::Add(char *str)
//   Add an entry to the configuration table.
//
void Configuration::Add(char *str)
{
	String	name, value;
	
	while (str && *str)
	{
		while (isspace(*str))
			str++;
		name = 0;
		if (!isalpha(*str))
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

		if (!strchr(separators->get(), *str))
		{
			//
			// We are now at a new name.  The previous one needs to be
			// set to boolean TRUE
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
			// Ah!  A quoted value.  This should be easy to deal
			// with...  (Just kidding!)
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
		else
		{
			//
			// A non-quoted string.  This string will terminate at the next blank
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
// void Configuration::Add(char *name, char *value)
//   Add an entry to the configuration table.
//
void Configuration::Add(char *name, char *value)
{
	ParsedString	*ps = new ParsedString(value);
	dict.Add(name, ps);
	if (mystrcasecmp(name, "locale") == 0)
	{
		setlocale(LC_ALL, value);
	}
}


//*********************************************************************
// int Configuration::Remove(char *name)
//   Remove an entry from both the hash table and from the list of keys.
//
int Configuration::Remove(char *name)
{
	return dict.Remove(name);
}


//*********************************************************************
// Object *Configuration::Find(char *name)
//   Retrieve a variable from the configuration database.  This variable
//   will be parsed and a new String object will be returned.
//
char *Configuration::Find(char *name)
{
	ParsedString	*ps = (ParsedString *) dict[name];
	if (ps)
	{
		return ps->get(dict);
	}
	else
	{
		return 0;
	}
}


//*********************************************************************
// int Configuration::Value(char *name, int default_value)
//
int Configuration::Value(char *name, int default_value)
{
	int		value = default_value;
	char	*s = Find(name);
	if (s && *s)
	{
		value = atoi(s);
	}

	return value;
}


//*********************************************************************
// double Configuration::Double(char *name, double default_value)
//
double Configuration::Double(char *name, double default_value)
{
	double		value = default_value;
	char	*s = Find(name);
	if (s && *s)
	{
		value = atof(s);
	}

	return value;
}


//*********************************************************************
// int Configuration::Boolean(char *name, int default_value)
//
int Configuration::Boolean(char *name, int default_value)
{
	int		value = default_value;
	char	*s = Find(name);
	if (s && *s)
	{
		if (mystrcasecmp(s, "true") == 0 ||
			mystrcasecmp(s, "yes") == 0 ||
			mystrcasecmp(s, "1") == 0)
				value = 1;
		else if (mystrcasecmp(s, "false") == 0 ||
			mystrcasecmp(s, "no") == 0 ||
			mystrcasecmp(s, "0") == 0)
				value = 0;
	}

	return value;
}


//*********************************************************************
// char *Configuration::operator[](char *name)
//
char *Configuration::operator[](char *name)
{
	return Find(name);
}


//*********************************************************************
// int Configuration::Read(char *filename)
//
int Configuration::Read(char *filename)
{
	ifstream	in(filename);

	if (in.bad() || in.eof())
		return NOTOK;

	char	buffer[1000];
	char	*current;
	String	line;
	String	name;
	char	*value;
	while (!in.bad())
	{
		in.getline(buffer, sizeof(buffer));
		if (in.eof())
			break;
		line << buffer;
		line.chop("\r\n");
		if (line.last() == '\\')
		{
			line.chop(1);
			continue;				// Append the next line to this one
		}

		current = line.get();
		if (*current == '#' || *current == '\0')
		{
			line = 0;
			continue;				// Comments and blank lines are skipped
		}

		name = strtok(current, ": =\t");
		value = strtok(0, "\r\n");
		if (!value)
			value = "";				// Blank value

		//
		// Skip any whitespace before the actual text
		//
		while (*value == ' ' || *value == '\t')
			value++;

		Add(name, value);
		line = 0;
	}
	in.close();
	return OK;
}


//*********************************************************************
// void Configuration::Defaults(ConfigDefaults *array)
//
void Configuration::Defaults(ConfigDefaults *array)
{
	for (int i = 0; array[i].name; i++)
	{
		Add(array[i].name, array[i].value);
	}
}

