/*
  scandir.c
  
  scandir.c: Portable version of scandir and alphasort for ht://Dig
             Based on code contributed through Usenet.
  
  Part of the ht://Dig package   <http://www.htdig.org/>
  Copyright (c) 2000 The ht://Dig Group
  For copyright details, see the file COPYING in your distribution
  or the GNU Public License version 2 or later 
  <http://www.gnu.org/copyleft/gpl.html>
  
  $Id: scandir.c,v 1.1.2.1 2000/11/19 06:17:22 ghutchis Exp $
*/

/*
 * Scan the directory dirname calling select to make a list of
selected
 * directory entries then sort using qsort and compare routine
dcomp.
 * Returns the number of entries and a pointer to a list of
pointers to
 * struct dirent (through namelist). Returns -1 if there were any
errors.
 */

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*
 * The DIRSIZ macro is the minimum record length which will hold
the directory
 * entry.  This requires the amount of space in struct dirent
without the
 * d_name field, plus enough space for the name and a terminating
nul byte
 * (dp->d_namlen + 1), rounded up to a 4 byte boundary.
 */
#undef DIRSIZ
#define DIRSIZ(dp) \
         ((sizeof(struct dirent) - sizeof(dp)->d_name) + \
         ((strlen((dp)->d_name) + 1 + 3) &~ 3))
#if 0
         ((sizeof(struct dirent) - sizeof(dp)->d_name) + \
         (((dp)->d_namlen + 1 + 3) &~ 3))
#endif

int Htscandir(dirname, namelist, select, dcomp)
     const char *dirname;
     struct dirent ***namelist;
     int (*select) (struct dirent *);
     int (*dcomp) (const void *, const void *);
{
  struct dirent *d, *p, **names;
  size_t nitems;
  struct stat stb;
  long arraysz;
  DIR *dirp;
  
  if ((dirp = opendir(dirname)) == NULL)
    return(-1);
  /*   if (fstat(dirp->dd_fd, &stb) < 0)
    return(-1);
  */

  /*
   * estimate the array size by taking the size of thedirectory file
   * and dividing it by a multiple of the minimum sizeentry.
   */
  arraysz = (stb.st_size / 24);
  names = (struct dirent **)malloc(arraysz * sizeof(struct dirent *));
  if (names == NULL)
    return(-1);
  
  nitems = 0;
  while ((d = readdir(dirp)) != NULL) {
    if (select != NULL && !(*select)(d))
      continue;       /* just selected names */
    /*
     * Make a minimum size copy of the data
     */
    p = (struct dirent *)malloc(DIRSIZ(d));
    if (p == NULL)
      return(-1);
    p->d_ino = d->d_ino;
    p->d_off = d->d_off;
    p->d_reclen = d->d_reclen;
    memcpy(p->d_name, d->d_name, strlen(d->d_name) +1);
#if 0
    p->d_fileno = d->d_fileno;
    p->d_type = d->d_type;
    p->d_reclen = d->d_reclen;
    p->d_namlen = d->d_namlen;
    bcopy(d->d_name, p->d_name, p->d_namlen + 1);
#endif
    /*
     * Check to make sure the array has space left and
     * realloc the maximum size.
     */
    if (++nitems >= arraysz) {
      /* if (fstat(dirp->dd_fd, &stb) < 0)
	return(-1);
      */
      arraysz = stb.st_size / 12;
      names = (struct dirent **)realloc((char*)names,
					arraysz * sizeof(struct dirent*));
      if (names == NULL)
	return(-1);
    }
    names[nitems-1] = p;
  }
  closedir(dirp);
  if (nitems && dcomp != NULL)
    qsort(names, nitems, sizeof(struct dirent *),dcomp);
  *namelist = names;
  return(nitems);
}

/*
 * Alphabetic order comparison routine for those who want it.
 */
int Htalphasort(d1, d2)
        const void *d1;
        const void *d2;
{
        return(strcmp((*(struct dirent **)d1)->d_name,
            (*(struct dirent **)d2)->d_name));
}
