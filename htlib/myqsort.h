/*
 * Part of the ht://Dig package   <http://www.htdig.org/>
 * Copyright (c) 1999, 2000 The ht://Dig Group
 * For copyright details, see the file COPYING in your distribution
 * or the GNU General Public License version 2 or later 
 * <http://www.gnu.org/copyleft/gpl.html>
 */
#ifndef _myqsort_h
#define _myqsort_h

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*myqsort_cmp)(void *data, void *a, void *b);

void myqsort(void *const pbase, size_t total_elems, size_t size, myqsort_cmp cmp, void *data);

#ifdef __cplusplus
}
#endif

#endif /* _myqsort_h */
