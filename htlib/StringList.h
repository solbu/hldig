//
// StringList.h
//
// StringList: Specialized List containing String objects. 
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: StringList.h,v 1.7.2.3 2000/01/03 11:49:09 bosc Exp $
//

#ifndef _StringList_h_
#define _StringList_h_

#include "Object.h"
#include "List.h"
#include "htString.h"


class StringList : public List
{
public:
    //
    // Construction/Destruction
    //
    StringList();
    ~StringList();

    void SRelease();
    
    //
    // Creation of a String from a string or String
    //
    StringList(const char *str, char sep = '\t') { Create(str, sep); }
    StringList(const String &str, char sep = '\t') { Create(str, sep); }
    StringList(const char *str, const char *sep) { Create(str, sep); }
    StringList(const String &str, const char *sep) { Create(str, sep); }

    int			Create(const char *str, char sep = '\t');
    int			Create(const String &str, char sep = '\t') { return Create(str.get(), sep); }
    int			Create(const char *str, const char *sep);
    int			Create(const String &str, const char *sep) { return Create(str.get(), sep); }

    //
    // Standard List operations...
    //
    void		Add(char *);
    void		Add(Object *obj);
    void		Insert(char *, int pos);
    void		Insert(Object *obj, int pos);
    void		Assign(char *, int pos);
    void		Assign(Object *obj, int pos);
    int			Remove(int pos);
    int			Remove(Object *obj);

    //
    // Since we know we only store strings, we can reliably sort them.
    // If direction is 1, the sort will be in descending order
    //
    void		Sort(int direction = 0);
	
    //
    // Join the Elements of the StringList together
    //
    String              Join(char) const;

    //
    // Getting at the parts of the StringList
    //
    char		*operator [] (int n);

private:
};

#endif
