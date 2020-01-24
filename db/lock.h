/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 *
 *  @(#)lock.h  11.5 (Sleepycat) 10/27/99
 */

#ifndef DB_LOCK_DEFAULT_N
#define DB_LOCK_DEFAULT_N  1000 /* Default # of locks in region. */
#endif

/*
 * Out of band value for a lock.  Locks contain an offset into a lock region,
 * so we use an invalid region offset to indicate an invalid or unset lock.
 */
#define  LOCK_INVALID  INVALID_ROFF

/*
 * The locker id space is divided between the transaction manager and the lock
 * manager.  Lock IDs start at 0 and go to DB_LOCK_MAXID.  Txn IDs start at
 * DB_LOCK_MAXID + 1 and go up to TXN_INVALID.
 */
#define DB_LOCK_MAXID    0x7fffffff

/*
 * DB_LOCKREGION --
 *  The lock shared region.
 */
typedef struct __db_lockregion
{
  uint32_t id;                 /* unique id generator */
  uint32_t need_dd;            /* flag for deadlock detector */
  uint32_t detect;             /* run dd on every conflict */
  /* free lock header */
    SH_TAILQ_HEAD (__flock) free_locks;
  /* free obj header */
    SH_TAILQ_HEAD (__fobj) free_objs;
  /* free locker header */
    SH_TAILQ_HEAD (__flocker) free_lockers;
  uint32_t maxlocks;           /* maximum number of locks in table */
  uint32_t table_size;         /* size of hash table */
  uint32_t nmodes;             /* number of lock modes */
  uint32_t nlockers;           /* number of lockers */
  uint32_t maxnlockers;        /* maximum number of lockers */
  roff_t memlock_off;           /* offset of memory mutex */
  roff_t conf_off;              /* offset of conflicts array */
  roff_t obj_off;               /* offset of object hash table */
  roff_t osynch_off;            /* offset of the object mutex table */
  roff_t locker_off;            /* offset of locker hash table */
  roff_t lsynch_off;            /* offset of the locker mutex table */
  uint32_t nconflicts;         /* number of lock conflicts */
  uint32_t nrequests;          /* number of lock gets */
  uint32_t nreleases;          /* number of lock puts */
  uint32_t ndeadlocks;         /* number of deadlocks */
} DB_LOCKREGION;

/*
 * Since we will store DBTs in shared memory, we need the equivalent of a
 * DBT that will work in shared memory.
 */
typedef struct __sh_dbt
{
  uint32_t size;               /* Byte length. */
  ssize_t off;                  /* Region offset. */
} SH_DBT;

#define SH_DBT_PTR(p)  ((void *)(((uint8_t *)(p)) + (p)->off))

/*
 * Object structures;  these live in the object hash table.
 */
typedef struct __db_lockobj
{
  SH_DBT lockobj;               /* Identifies object locked. */
  SH_TAILQ_ENTRY links;         /* Links for free list. */
    SH_TAILQ_HEAD (__wait) waiters;     /* List of waiting locks. */
    SH_TAILQ_HEAD (__hold) holders;     /* List of held locks. */
  /* Declare room in the object to hold
   * typical DB lock structures so that
   * we do not have to allocate them from
   * shalloc at run-time. */
  uint8_t objdata[sizeof (struct __db_ilock)];
} DB_LOCKOBJ;

/*
 * Locker structures; these live in the locker hash table.
 */
typedef struct __db_locker
{
  uint32_t id;                 /* Locker id. */
  uint32_t dd_id;              /* Deadlock detector id. */
  size_t master_locker;         /* Locker of master transaction. */
  size_t parent_locker;         /* Parent of this child. */
    SH_LIST_HEAD (_child) child_locker; /* List of descendant txns;
                                           only used in a "master"
                                           txn. */
  SH_LIST_ENTRY child_link;     /* Links transactions in the family;
                                   elements of the child_locker
                                   list. */
  SH_TAILQ_ENTRY links;         /* Links for free list. */
    SH_LIST_HEAD (_held) heldby;        /* Locks held by this locker. */

#define DB_LOCKER_DELETED  0x0001
  uint32_t flags;
} DB_LOCKER;

/*
 * Lockers can be freed if they are not poart of a transaction
 * family.  Members of a family either point at the master
 * transaction or are the master transaction and have
 * children lockers.
 */
#define LOCKER_FREEABLE(lp)  ((lp)->master_locker       \
       == TXN_INVALID_ID &&       \
      SH_LIST_FIRST(&(lp)->child_locker, __db_locker) \
       == NULL)

/*
 * DB_LOCKTAB --
 *  The primary library lock data structure (i.e., the one referenced
 * by the environment, as opposed to the internal one laid out in the region.)
 */
typedef struct __db_locktab
{
  DB_ENV *dbenv;                /* Environment. */
  REGINFO reginfo;              /* Region information. */
  MUTEX *memlock;               /* Mutex to protect memory alloc. */
  uint8_t *conflicts;          /* Pointer to conflict matrix. */
  DB_HASHTAB *obj_tab;          /* Beginning of object hash table. */
  MUTEX *osynch_tab;            /* Object mutex table. */
  DB_HASHTAB *locker_tab;       /* Beginning of locker hash table. */
  MUTEX *lsynch_tab;            /* Locker mutex table. */
} DB_LOCKTAB;

/* Test for conflicts. */
#define CONFLICTS(T, R, HELD, WANTED) \
  (T)->conflicts[(HELD) * (R)->nmodes + (WANTED)]

#define OBJ_LINKS_VALID(L) ((L)->links.stqe_prev != -1)

struct __db_lock
{
  /*
   * Wait on mutex to wait on lock.  You reference your own mutex with
   * ID 0 and others reference your mutex with ID 1.
   */
  MUTEX mutex;

  uint32_t holder;             /* Who holds this lock. */
  uint32_t gen;                /* Generation count. */
  SH_TAILQ_ENTRY links;         /* Free or holder/waiter list. */
  SH_LIST_ENTRY locker_links;   /* List of locks held by a locker. */
  uint32_t refcount;           /* Reference count the lock. */
  db_lockmode_t mode;           /* What sort of lock. */
  ssize_t obj;                  /* Relative offset of object struct. */
  roff_t txnoff;                /* Offset of holding transaction. */
  db_status_t status;           /* Status of this lock. */
};

/*
 * This is a serious layering violation.  To support nested transactions, we
 * need to be able to tell that a lock is held by a transaction (as opposed to
 * some other locker) and to be able to traverse the parent/descendent chain.
 * In order to do this, each lock held by a transaction maintains a reference
 * to the shared memory transaction structure so it can be accessed during lock
 * promotion.  As the structure is in shared memory, we cannot store a pointer
 * to it, so we use the offset within the region.  An invalid region offset is
 * used to indicate that there is no transaction associated with the current
 * lock.
 */
#define TXN_IS_HOLDING(L)  ((L)->txnoff != INVALID_ROFF)

/*
 * Flag values for CDB___lock_put_internal:
 * DB_LOCK_DOALL:     Unlock all references in this lock (instead of only 1).
 * DB_LOCK_FREE:      Free the lock (used in checklocker).
 * DB_LOCK_IGNOREDEL: Remove from the locker hash table even if already
          deleted (used in checklocker).
 * DB_LOCK_NOPROMOTE: Don't bother running promotion when releasing locks
 *          (used by CDB___lock_put_internal).
 * DB_LOCK_UNLINK:    Remove from the locker links (used in checklocker).
 */
#define  DB_LOCK_DOALL    0x001
#define  DB_LOCK_FREE    0x002
#define  DB_LOCK_IGNOREDEL  0x004
#define  DB_LOCK_NOPROMOTE  0x008
#define  DB_LOCK_UNLINK    0x010

/*
 * Macros to get/release different types of mutexes.
 */
#define OBJECT_LOOKUP(lt, ndx, dbt, sh_obj)        \
  HASHLOOKUP((lt)->objtab,          \
      ndx, __db_lockobj, links, dbt, sh_obj, CDB___lock_cmp);

#ifdef FINE_GRAIN
#define OBJECT_LOCK_NDX(lt, ndx)          \
  MUTEX_LOCK(&(lt)->osynch_tab[ndx], (lt)->dbenv->lockfhp)
#define OBJECT_LOCK(lt, reg, obj, ndx)           \
  HASHACCESS((lt)->osynch_tab, obj,        \
      (reg)->table_size, CDB___lock_ohash, ndx, (lt)->dbenv->lockfhp)
#define SHOBJECT_LOCK(lt, reg, shobj, ndx)        \
  HASHACCESS((lt)->osynch_tab, shobj,        \
      (reg)->table_size, CDB___lock_lhash, ndx, (lt)->dbenv->lockfhp)
#define  OBJECT_UNLOCK(lt, ndx)            \
  MUTEX_UNLOCK(&(lt)->osynch_tab[ndx])
#else
#define  OBJECT_LOCK_NDX(lt, ndx)
#define OBJECT_LOCK(lt, reg, obj, ndx)           \
  ndx = CDB___lock_ohash(obj) % (reg)->table_size
#define SHOBJECT_LOCK(lt, reg, shobj, ndx)        \
  ndx = CDB___lock_lhash(shobj) % (reg)->table_size
#define  OBJECT_UNLOCK(lt, ndx)
#endif

#define LOCKER_LOOKUP(lt, ndx, locker, sh_locker)      \
  HASHLOOKUP((lt)->lockertab,          \
      ndx, __db_locker, links, locker, sh_locker, CDB___lock_locker_cmp);

#ifdef FINE_GRAIN
#define  LOCKER_LOCK_NDX(lt, ndx)          \
  MUTEX_LOCK(&(lt)->lsynch_tab[ndx], (lt)->dbenv->lockfhp)
#define  LOCKER_LOCK(lt, reg, locker, ndx)        \
  HASHACCESS((lt)->lsynch_tab, locker,        \
      reg->table_size, CDB___lock_locker_hash, ndx, (lt)->dbenv->lockfhp)
#define  LOCKER_UNLOCK(lt, ndx)            \
  MUTEX_UNLOCK(&(lt)->lsynch_tab[ndx])

#define MEMORY_LOCK(lt)              \
  MUTEX_LOCK((lt)->memlock, (lt)->dbenv->lockfhp)
#define MEMORY_UNLOCK(lt)            \
  MUTEX_UNLOCK((lt)->memlock)
#else
#define  LOCKER_LOCK_NDX(lt, ndx)
#define  LOCKER_LOCK(lt, reg, locker, ndx)        \
  ndx = CDB___lock_locker_hash(locker) % (reg)->table_size
#define  LOCKER_UNLOCK(lt, ndx)
#define MEMORY_LOCK(lt)
#define MEMORY_UNLOCK(lt)
#endif

#ifdef FINE_GRAIN
#define  LOCKREGION(dbenv, lt)
#define  UNLOCKREGION(dbenv, lt)
#else
#define  LOCKREGION(dbenv, lt)  R_LOCK((dbenv), &(lt)->reginfo)
#define  UNLOCKREGION(dbenv, lt)  R_UNLOCK((dbenv), &(lt)->reginfo)
#endif
#include "lock_ext.h"
