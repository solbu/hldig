//
// ParsedString.cc
//
// ParsedString: Contains a string. The string my contain $var, ${var}, $(var)
//               `filename`. The get method will expand those using the
//               dictionary given in argument.
// 
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: ParsedString.cc,v 1.4 1999/09/11 05:03:52 ghutchis Exp $
//

#include "ParsedString.h"

#include <fstream.h>
#include <ctype.h>
#include <stdio.h>


//*****************************************************************************
// ParsedString::ParsedString()
//
ParsedString::ParsedString()
{
}


//*****************************************************************************
// ParsedString::ParsedString(char *s)
//
ParsedString::ParsedString(char *s)
{
    value = s;
}


//*****************************************************************************
// ParsedString::~ParsedString()
//
ParsedString::~ParsedString()
{
}


//*****************************************************************************
// void ParsedString::set(char *str)
//
void
ParsedString::set(char *str)
{
    value = str;
}


//*****************************************************************************
// char *ParsedString::get(Dictionary &dict)
//   Return a fully parsed string.
//
//   Allowed syntax:
//       $var
//       ${var}
//       $(var)
//       `filename`
//
//   The filename can also contain variables
//
char *
ParsedString::get(Dictionary &dict)
{
    String		variable("");
    ParsedString	*temp;
    char		*str = value.get();
    char		delim = ' ';
    int		need_delim = 0;

    parsed = 0;
    while (*str)
    {
        if (*str == '$')
        {
            //
            // A dollar sign starts a variable.
            //
            str++;
            need_delim = 1;
            if (*str == '{')
                delim = '}';
            else if (*str == '(')
                delim = ')';
            else
                need_delim = 0;
            if (need_delim)
                str++;
            variable = 0;
            while (isalpha(*str) || *str == '_' || *str == '-')
            {
                variable << *str++;
            }
            if (*str)
            {
                if (need_delim && *str == delim)
                {
                    //
                    // Found end of variable
                    //
                    temp = (ParsedString *) dict[variable];
                    if (temp)
                        parsed << temp->get(dict);
                    str++;
                }
                else if (need_delim)
                {
                    //
                    // Error.  Probably an illegal value in the name We'll
                    // assume the variable ended here.
                    //
                    temp = (ParsedString *) dict[variable];
                    if (temp)
                        parsed << temp->get(dict);
                }
                else
                {
                    //
                    // This variable didn't have a delimiter.
                    //
                    temp = (ParsedString *) dict[variable];
                    if (temp)
                        parsed << temp->get(dict);
                }
            }
            else
            {
                //
                // End of string reached.  We'll assume that this is also
                // the end of the variable
		//
                temp = (ParsedString *) dict[variable];
                if (temp)
                    parsed << temp->get(dict);
            }
        }
        else if (*str == '`')
        {
            //
            // Back-quote delimits a filename which we need to insert
            //
            str++;
            variable = 0;
            while (*str && *str != '`')
            {
                variable << *str++;
            }
            if (*str == '`')
                str++;
            ParsedString	filename(variable);
            variable = 0;
            getFileContents(variable, filename.get(dict));
            parsed << variable;
        }
        else if (*str == '\\')
        {
            //
            // Backslash escapes the next character
            //
            str++;
            if (*str)
                parsed << *str++;
        }
        else
        {
            //
            // Normal character
            //
            parsed << *str++;
        }
    }
    return parsed.get();
}


void
ParsedString::getFileContents(String &str, char *filename)
{
    FILE	*fl = fopen(filename, "r");
    char	buffer[1000];

    if (!fl)
        return;
    while (fgets(buffer, sizeof(buffer), fl))
    {
        String	s(buffer);
        s.chop("\r\n\t ");
        str << s << ' ';
    }
    str.chop(1);
    fclose(fl);
}

