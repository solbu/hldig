/*                  */
/* dirent_local.c              */
/*                  */
/*    POSIX directory routines for Windows.        */
/*                  */
/* Part of the ht://Dig package   <http://www.htdig.org/>    */
/* Copyright (c) 2003 The ht://Dig Group        */
/* For copyright details, see the file COPYING in your distribution  */
/* or the GNU Library General Public License version 2 or later    */
/* <http://www.gnu.org/copyleft/lgpl.html>        */
/*                  */
/* $Id: direct_local.c               */
/*                  */

/* 
 * POSIX directory routines for Windows.
 *
 * Added by Neal Richter, RightNow Technologies
 * June 2003
 * 
 */

#ifdef _MSC_VER /* _WIN32 */

#include <windows.h>
#include <iostream.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "dirent_local.h"


/* ----- opendir ------------------------------------- Oct 23, 1999  21:52 ---
 * Open a directory handle 
 */
DIR *
opendir(const char *name)
{
    DIR *dirp;
    char dirpath[256];

    if ((dirp=malloc(sizeof(struct DIRstruct))) == NULL) {
        errno = ENOMEM;
        return(NULL);
    }

    sprintf(dirpath, "%s/*", name);
    if ((dirp->filehand = FindFirstFile(dirpath, &dirp->finddata)) ==
        INVALID_HANDLE_VALUE) {
        errno = ENOENT;
        return(NULL);
    }

    dirp->priv = 1;

    return(dirp);
}


/* ----- readdir ------------------------------------- Oct 23, 1999  21:52 ---
 * Return the next file for a directory handle
 */
struct dirent *
readdir(DIR *dirp)
{
    if (!dirp) {
        errno = EBADF;
        return(NULL);
    }

    if (dirp->priv) {
        /* this is the first time, so return the results of FindFirstFile */
        dirp->priv = 0;
        strcpy(dirp->file.d_name, dirp->finddata.cFileName);
        return(&dirp->file);
    }

    /* this is a subsequent call so get next file */
    if (!FindNextFile(dirp->filehand, &dirp->finddata)) 
        return(NULL);

    strcpy(dirp->file.d_name, dirp->finddata.cFileName);
    return(&dirp->file);
}


/* ----- closedir ------------------------------------ Oct 23, 1999  21:52 ---
 * Close directory handle
 */
int
closedir(DIR *dirp)
{
    if (!dirp) {
        errno = EBADF;
        return(-1);
    }

    FindClose(dirp->filehand);

    free(dirp);

    return(0);
}

#endif /* _MSC_VER (WIN32) */
