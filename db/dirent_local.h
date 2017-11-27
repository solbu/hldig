/*                  */
/* dirent_local.h              */
/*                  */
/*    POSIX directory routines for Windows.        */
/*                  */
/* Part of the ht://Dig package   <http://www.htdig.org/>    */
/* Copyright (c) 2003 The ht://Dig Group        */
/* For copyright details, see the file COPYING in your distribution  */
/* or the GNU Library General Public License version 2 or later    */
/* <http://www.gnu.org/copyleft/lgpl.html>        */
/*                  */
/* $Id: direct_local.h              */
/*                  */

/* 
 * POSIX directory routines for Windows.
 *
 * Added by Neal Richter, RightNow Technologies
 * June 2003
 * 
 */

#ifndef DIRENT_LOCAL_H
#define DIRENT_LOCAL_H


#ifndef _WIN32
#include <dirent.h>

#else

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* WIN equivalent of POSIX directory routines */

#define MAXNAMLEN       255

  struct dirent
  {
    char d_name[256];           /* directory name */
  };

  struct DIRstruct
  {
    HANDLE filehand;
    WIN32_FIND_DATA finddata;
    struct dirent file;
    char priv;
  };
  typedef struct DIRstruct DIR;

  DIR *opendir (const char *name);
  struct dirent *readdir (DIR *);
  int closedir (DIR *);

#ifdef __cplusplus
}
#endif

#endif                          /* _WIN32 */

#if defined(SOLARIS) || defined(_WIN32)
int scandir (char *dirname,
             struct dirent ***namelist,
             int (*select) (struct dirent *), int (*dcomp) (void *, void *));

int alphasort (void *a, void *b);
#endif

#endif /* !DIRENT_LOCAL_H */
