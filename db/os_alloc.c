/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char revid[] = "$Id: os_alloc.c,v 1.1.2.2 2000/09/14 03:13:22 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#endif

#include "db_int.h"
#include "os_jump.h"

#ifdef DIAGNOSTIC
static void __os_guard __P((void));
#endif

/*
 * !!!
 * Correct for systems that return NULL when you allocate 0 bytes of memory.
 * There are several places in DB where we allocate the number of bytes held
 * by the key/data item, and it can be 0.  Correct here so that malloc never
 * returns a NULL for that reason (which behavior is permitted by ANSI).  We
 * could make these calls macros on non-Alpha architectures (that's where we
 * saw the problem), but it's probably not worth the autoconf complexity.
 *
 * !!!
 * Correct for systems that don't set errno when malloc and friends fail.
 *
 *	Out of memory.
 *	We wish to hold the whole sky,
 *	But we never will.
 */

/*
 * CDB___os_strdup --
 *	The strdup(3) function for DB.
 *
 * PUBLIC: int CDB___os_strdup __P((DB_ENV *, const char *, void *));
 */
int
CDB___os_strdup(dbenv, str, storep)
	DB_ENV *dbenv;
	const char *str;
	void *storep;
{
	size_t size;
	int ret;
	void *p;

	*(void **)storep = NULL;

	size = strlen(str) + 1;
	if ((ret = CDB___os_malloc(dbenv, size, NULL, &p)) != 0)
		return (ret);

	memcpy(p, str, size);

	*(void **)storep = p;
	return (0);
}

/*
 * CDB___os_calloc --
 *	The calloc(3) function for DB.
 *
 * PUBLIC: int CDB___os_calloc __P((DB_ENV *, size_t, size_t, void *));
 */
int
CDB___os_calloc(dbenv, num, size, storep)
	DB_ENV *dbenv;
	size_t num, size;
	void *storep;
{
	void *p;
	int ret;

	size *= num;
	if ((ret = CDB___os_malloc(dbenv, size, NULL, &p)) != 0)
		return (ret);

	memset(p, 0, size);

	*(void **)storep = p;
	return (0);
}

/*
 * CDB___os_malloc --
 *	The malloc(3) function for DB.
 *
 * PUBLIC: int CDB___os_malloc __P((DB_ENV *, size_t, void *(*)(size_t), void *));
 */
int
CDB___os_malloc(dbenv, size, db_malloc, storep)
	DB_ENV *dbenv;
	size_t size;
	void *(*db_malloc) __P((size_t)), *storep;
{
	int ret;
	void *p;

	*(void **)storep = NULL;

	/* Never allocate 0 bytes -- some C libraries don't like it. */
	if (size == 0)
		++size;
#ifdef DIAGNOSTIC
	else
		++size;				/* Add room for a guard byte. */
#endif

	/* Some C libraries don't correctly set errno when malloc(3) fails. */
	CDB___os_set_errno(0);
	if (db_malloc != NULL)
		p = db_malloc(size);
	else if (CDB___db_jump.j_malloc != NULL)
		p = CDB___db_jump.j_malloc(size);
	else
		p = malloc(size);
	if (p == NULL) {
		ret = CDB___os_get_errno();
		if (ret == 0) {
			CDB___os_set_errno(ENOMEM);
			ret = ENOMEM;
		}
		CDB___db_err(dbenv,
		    "malloc: %s: %lu", strerror(ret), (u_long)size);
		return (ret);
	}

#ifdef DIAGNOSTIC
	/*
	 * Guard bytes: if #DIAGNOSTIC is defined, we allocate an additional
	 * byte after the memory and set it to a special value that we check
	 * for when the memory is free'd.  This is fine for structures, but
	 * not quite so fine for strings.  There are places in DB where memory
	 * is allocated sufficient to hold the largest possible string that
	 * we'll see, and then only some subset of the memory is used.  To
	 * support this CDB_usage, the CDB___os_freestr() function checks the byte
	 * after the string's nul, which may or may not be the last byte in
	 * the originally allocated memory.
	 */
	memset(p, CLEAR_BYTE, size);		/* Initialize guard byte. */
#endif
	*(void **)storep = p;

	return (0);
}

/*
 * CDB___os_realloc --
 *	The realloc(3) function for DB.
 *
 * PUBLIC: int CDB___os_realloc __P((DB_ENV *,
 * PUBLIC:     size_t, void *(*)(void *, size_t), void *));
 */
int
CDB___os_realloc(dbenv, size, db_realloc, storep)
	DB_ENV *dbenv;
	size_t size;
	void *(*db_realloc) __P((void *, size_t)), *storep;
{
	int ret;
	void *p, *ptr;

	ptr = *(void **)storep;

	/* If we haven't yet allocated anything yet, simply call malloc. */
	if (ptr == NULL && db_realloc == NULL)
		return (CDB___os_malloc(dbenv, size, NULL, storep));

	/* Never allocate 0 bytes -- some C libraries don't like it. */
	if (size == 0)
		++size;
#ifdef DIAGNOSTIC
	else
		++size;				/* Add room for a guard byte. */
#endif

	/*
	 * Some C libraries don't correctly set errno when realloc(3) fails.
	 *
	 * Don't overwrite the original pointer, there are places in DB we
	 * try to continue after realloc fails.
	 */
	CDB___os_set_errno(0);
	if (db_realloc != NULL)
		p = db_realloc(ptr, size);
	else if (CDB___db_jump.j_realloc != NULL)
		p = CDB___db_jump.j_realloc(ptr, size);
	else
		p = realloc(ptr, size);
	if (p == NULL) {
		if ((ret = CDB___os_get_errno()) == 0) {
			ret = ENOMEM;
			CDB___os_set_errno(ENOMEM);
		}
		CDB___db_err(dbenv,
		    "realloc: %s: %lu", strerror(ret), (u_long)size);
		return (ret);
	}
#ifdef DIAGNOSTIC
	((u_int8_t *)p)[size - 1] = CLEAR_BYTE;	/* Initialize guard byte. */
#endif

	*(void **)storep = p;

	return (0);
}

/*
 * CDB___os_free --
 *	The free(3) function for DB.
 *
 * PUBLIC: void CDB___os_free __P((void *, size_t));
 */
void
CDB___os_free(ptr, size)
	void *ptr;
	size_t size;
{
#ifdef DIAGNOSTIC
	if (size != 0) {
		/*
		 * Check that the guard byte (one past the end of the memory) is
		 * still CLEAR_BYTE.
		 */
		if (((u_int8_t *)ptr)[size] != CLEAR_BYTE)
			 __os_guard();

		/* Clear memory. */
		if (size != 0)
			memset(ptr, CLEAR_BYTE, size);
	}
#else
	COMPQUIET(size, 0);
#endif

	if (CDB___db_jump.j_free != NULL)
		CDB___db_jump.j_free(ptr);
	else
		free(ptr);
}

/*
 * CDB___os_freestr --
 *	The free(3) function for DB, freeing a string.
 *
 * PUBLIC: void CDB___os_freestr __P((void *));
 */
void
CDB___os_freestr(ptr)
	void *ptr;
{
#ifdef DIAGNOSTIC
	size_t size;

	size = strlen(ptr) + 1;

	/*
	 * Check that the guard byte (one past the end of the memory) is
	 * still CLEAR_BYTE.
	 */
	if (((u_int8_t *)ptr)[size] != CLEAR_BYTE)
		 __os_guard();

	/* Clear memory. */
	memset(ptr, CLEAR_BYTE, size);
#endif

	if (CDB___db_jump.j_free != NULL)
		CDB___db_jump.j_free(ptr);
	else
		free(ptr);
}

#ifdef DIAGNOSTIC
/*
 * __os_guard --
 *	Complain and abort.
 */
static void
__os_guard()
{
	/*
	 * Eventually, once we push a DB_ENV handle down to these
	 * routines, we should use the standard output channels.
	 */
	fprintf(stderr, "Guard byte incorrect during free.\n");
	abort();
	/* NOTREACHED */
}
#endif

/*
 * CDB___ua_memcpy --
 *	Copy memory to memory without relying on any kind of alignment.
 *
 *	There are places in DB that we have unaligned data, for example,
 *	when we've stored a structure in a log record as a DBT, and now
 *	we want to look at it.  Unfortunately, if you have code like:
 *
 *		struct a {
 *			int x;
 *		} *p;
 *
 *		void *func_argument;
 *		int local;
 *
 *		p = (struct a *)func_argument;
 *		memcpy(&local, p->x, sizeof(local));
 *
 *	compilers optimize to use inline instructions requiring alignment,
 *	and records in the log don't have any particular alignment.  (This
 *	isn't a compiler bug, because it's a structure they're allowed to
 *	assume alignment.)
 *
 *	Casting the memcpy arguments to (u_int8_t *) appears to work most
 *	of the time, but we've seen examples where it wasn't sufficient
 *	and there's nothing in ANSI C that requires that work.
 *
 * PUBLIC: void *CDB___ua_memcpy __P((void *, const void *, size_t));
 */
void *
CDB___ua_memcpy(dst, src, len)
	void *dst;
	const void *src;
	size_t len;
{
	return (memcpy(dst, src, len));
}
