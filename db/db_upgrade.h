/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *  Sleepycat Software.  All rights reserved.
 *
 * $Id: db_upgrade.h,v 1.2 2002/02/02 18:18:05 ghutchis Exp $
 */

#ifndef _DB_UPGRADE_H_
#define  _DB_UPGRADE_H_

/*
 * This file defines the metadata pages from the previous release.
 * These structures are only used to upgrade old versions of databases.
 */

/* Structures from the 3.0 release */

typedef struct _dbmeta30
{
  DB_LSN lsn;                   /* 00-07: LSN. */
  db_pgno_t pgno;               /* 08-11: Current page number. */
  uint32_t magic;              /* 12-15: Magic number. */
  uint32_t version;            /* 16-19: Version. */
  uint32_t pagesize;           /* 20-23: Pagesize. */
  uint8_t unused1[1];          /*    24: Unused. */
  uint8_t type;                /*    25: Page type. */
  uint8_t unused2[2];          /* 26-27: Unused. */
  uint32_t free;               /* 28-31: Free list page number. */
  uint32_t flags;              /* 32-35: Flags: unique to each AM. */
  /* 36-55: Unique file ID. */
  uint8_t uid[DB_FILE_ID_LEN];
} DBMETA30;

/************************************************************************
 BTREE METADATA PAGE LAYOUT
 ************************************************************************/
typedef struct _btmeta30
{
  DBMETA30 dbmeta;              /* 00-55: Generic meta-data header. */

  uint32_t maxkey;             /* 56-59: Btree: Maxkey. */
  uint32_t minkey;             /* 60-63: Btree: Minkey. */
  uint32_t re_len;             /* 64-67: Recno: fixed-length record length. */
  uint32_t re_pad;             /* 68-71: Recno: fixed-length record pad. */
  uint32_t root;               /* 72-75: Root page. */

  /*
   * Minimum page size is 128.
   */
} BTMETA30;

/************************************************************************
 HASH METADATA PAGE LAYOUT
 ************************************************************************/
typedef struct _hashmeta30
{
  DBMETA30 dbmeta;              /* 00-55: Generic meta-data page header. */

  uint32_t max_bucket;         /* 56-59: ID of Maximum bucket in use */
  uint32_t high_mask;          /* 60-63: Modulo mask into table */
  uint32_t low_mask;           /* 64-67: Modulo mask into table lower half */
  uint32_t ffactor;            /* 68-71: Fill factor */
  uint32_t nelem;              /* 72-75: Number of keys in hash table */
  uint32_t h_charkey;          /* 76-79: Value of hash(CHARKEY) */
#define  NCACHED30  32          /* number of spare points */
  /* 80-207: Spare pages for overflow */
  uint32_t spares[NCACHED30];

  /*
   * Minimum page size is 256.
   */
} HMETA30;

/************************************************************************
 QUEUE METADATA PAGE LAYOUT
 ************************************************************************/
/*
 * QAM Meta data page structure
 *
 */
typedef struct _qmeta30
{
  DBMETA30 dbmeta;              /* 00-55: Generic meta-data header. */

  uint32_t start;              /* 56-59: Start offset. */
  uint32_t first_recno;        /* 60-63: First not deleted record. */
  uint32_t cur_recno;          /* 64-67: Last recno allocated. */
  uint32_t re_len;             /* 68-71: Fixed-length record length. */
  uint32_t re_pad;             /* 72-75: Fixed-length record pad. */
  uint32_t rec_page;           /* 76-79: Records Per Page. */

  /*
   * Minimum page size is 128.
   */
} QMETA30;

/* Structures from Release 2.x */

/************************************************************************
 BTREE METADATA PAGE LAYOUT
 ************************************************************************/

/*
 * Btree metadata page layout:
 */
typedef struct _btmeta2X
{
  DB_LSN lsn;                   /* 00-07: LSN. */
  db_pgno_t pgno;               /* 08-11: Current page number. */
  uint32_t magic;              /* 12-15: Magic number. */
  uint32_t version;            /* 16-19: Version. */
  uint32_t pagesize;           /* 20-23: Pagesize. */
  uint32_t maxkey;             /* 24-27: Btree: Maxkey. */
  uint32_t minkey;             /* 28-31: Btree: Minkey. */
  uint32_t free;               /* 32-35: Free list page number. */
  uint32_t flags;              /* 36-39: Flags. */
  uint32_t re_len;             /* 40-43: Recno: fixed-length record length. */
  uint32_t re_pad;             /* 44-47: Recno: fixed-length record pad. */
  /* 48-67: Unique file ID. */
  uint8_t uid[DB_FILE_ID_LEN];
} BTMETA2X;

/************************************************************************
 HASH METADATA PAGE LAYOUT
 ************************************************************************/

/*
 * Hash metadata page layout:
 */
/* Hash Table Information */
typedef struct hashhdr
{                               /* Disk resident portion */
  DB_LSN lsn;                   /* 00-07: LSN of the header page */
  db_pgno_t pgno;               /* 08-11: Page number (btree compatibility). */
  uint32_t magic;              /* 12-15: Magic NO for hash tables */
  uint32_t version;            /* 16-19: Version ID */
  uint32_t pagesize;           /* 20-23: Bucket/Page Size */
  uint32_t ovfl_point;         /* 24-27: Overflow page allocation location */
  uint32_t last_freed;         /* 28-31: Last freed overflow page pgno */
  uint32_t max_bucket;         /* 32-35: ID of Maximum bucket in use */
  uint32_t high_mask;          /* 36-39: Modulo mask into table */
  uint32_t low_mask;           /* 40-43: Modulo mask into table lower half */
  uint32_t ffactor;            /* 44-47: Fill factor */
  uint32_t nelem;              /* 48-51: Number of keys in hash table */
  uint32_t h_charkey;          /* 52-55: Value of hash(CHARKEY) */
  uint32_t flags;              /* 56-59: Allow duplicates. */
#define  NCACHED2X  32          /* number of spare points */
  /* 60-187: Spare pages for overflow */
  uint32_t spares[NCACHED2X];
  /* 188-207: Unique file ID. */
  uint8_t uid[DB_FILE_ID_LEN];

  /*
   * Minimum page size is 256.
   */
} HASHHDR;

#endif
