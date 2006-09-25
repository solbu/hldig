//
// filecopy.c
//
//  Copies files from one file to another.
//  Contains both Unix & Native Win32 Implementations
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later or later
// <http://www.gnu.org/copyleft/lgpl.html>
// 
// Copyright (c) 2002 RightNow Technologies, Inc.
// Donated to The ht://Dig Group under LGPL License
 
#include <stdio.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#else /* UNIX */

#include <stdio.h>
#include <sys/types.h>        
#include <sys/stat.h>        
#include <fcntl.h>                                                                             
#include <unistd.h>
#include <errno.h>

#endif /* _WIN32 | UNIX  */


#include "filecopy.h" 



//----------------------------------------------------------------------
// int file_copy (char * from, char * to, char flags)
//----------------------------------------------------------------------
//
// copy file 'from' -> 'to'
// 
// set flags to FILECOPY_OVERWRITE_ON to overwrite the 'to' file if 
// it exists
//
// set flags to FILECOPY_OVERWRITE_OFF to not overwrite the 'to' file
// if it exists
//
// returns 0/FALSE if unsucessful
// returns 1/TRUE if  sucessful
//
//
#ifdef _WIN32

int file_copy (char * from, char * to, char flags)
{
    if (flags == FILECOPY_OVERWRITE_ON)
    {
        //overwrite
        if (TRUE != (CopyFile(from , to, FALSE)))
            return (FALSE);
    }
    else if (flags == FILECOPY_OVERWRITE_OFF)
    {
         //don't overwrite
        if (TRUE != (CopyFile(from , to, TRUE)))
            return (FALSE);
    }
    else //bad flag
    {
        return (FALSE);
    }
    
    return (TRUE);
}

#else //UNIX
        
int file_copy (char * from, char * to, char flags)
{
    size_t nmemb;
    //int nmemb;
    FILE *ifp, *ofp;
    char buf[BUFSIZ];

    if (flags == FILECOPY_OVERWRITE_OFF) {
        if (access(to, F_OK) == 0) {
            //OUTLOG((FUNC, TRWRN, "file %s already exists\n", to));
            return(FALSE);
        }
        else if (errno != ENOENT) {
            //OUTLOG((FUNC, TRERR, "access(%s, F_OK) failed\n", to));
            return(FALSE);
        }
    }

    if ((ifp=fopen(from, "r")) == NULL) {
        //OUTLOG((FUNC, TRERR, "%s doesn't exist\n", from));
        return(FALSE);
    }
    
    if ((ofp=fopen(to, "w+")) == NULL) {
        //OUTLOG((FUNC, TRERR, "can't create %s\n", to));
        fclose(ifp);
        return(FALSE);
    }

    while ((nmemb=fread(buf, 1, sizeof(buf), ifp)) > 0) {
        if (fwrite(buf, 1, nmemb, ofp) != nmemb) {
            //OUTLOG((FUNC, TRERR, "fwrite failed\n"));
            fclose(ifp);
            fclose(ofp);
            return(FALSE);
        }
    }

    fclose(ifp);
    fclose(ofp);

    return (TRUE);
}

#endif /* _WIN32 | UNIX  */

