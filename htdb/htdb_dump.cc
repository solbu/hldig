//
// NAME
// 
// dump the content of an inverted index in Berkeley DB fashion
//
// SYNOPSIS
//
// htdb_dump [-klNpWz] [-C cachesize] [-d ahr] [-f file] [-h home] [-s subdb] db_file
//
// DESCRIPTION
//
// htdb_dump is a slightly modified version of the standard 
// Berkeley DB db_dump utility.
//
// The htdb_dump utility reads the database file <b>db_file</b> and
// writes it to the standard output using a portable flat-text format
// understood by the <i>htdb_load</i>
// utility. The argument <b>db_file</b> must be a file produced using
// the Berkeley DB library functions.
//
// OPTIONS
//
// <dl>
//
// <dt><b>-W</b>
// <dd>Initialize WordContext(3) before dumping. With the <b>-z</b>
// flag allows to dump inverted indexes using the mifluz(3) specific
// compression scheme. The MIFLUZ_CONFIG environment variable must be
// set to a file containing the mifluz(3) configuration.
//
// <dt><b>-z</b>
// <dd>The <b>db_file</b> is compressed. If <b>-W</b> is given the
// mifluz(3) specific compression scheme is used. Otherwise the default
// gzip compression scheme is used.
//
// <dt><b>-d</b>
// <dd>Dump the specified database in a format helpful for debugging
// the Berkeley DB library routines.
// <dl>
// <dt>
// a
// <dd>Display all information.
// <dt>
// h
// <dd>Display only page headers.
// <dt>
// r
// <dd>Do not display the free-list or pages on the free list.  This
// mode is used by the recovery tests.
// </dl>
// The output format of the <b>-d</b> option is not standard and may change,
// without notice, between releases of the Berkeley DB library.
//
// <dt><b>-f</b>
// <dd>Write to the specified <b>file</b> instead of to the standard output.
//
// <dt><b>-h</b>
// <dd>Specify a home directory for the database.
// As Berkeley DB versions before 2.0 did not support the concept of a
// <i>database home.</i>
//
// <dt><b>-k</b>
// <dd>Dump record numbers from Queue and Recno databases as keys.
//
// <dt><b>-l</b>
// <dd>List the subdatabases stored in the database.
//
// <dt><b>-N</b>
// <dd>Do not acquire shared region locks while running.  Other problems such
// as potentially fatal errors in Berkeley DB will be ignored as well.  This option
// is intended only for debugging errors and should not be used under any
// other circumstances.
// 
// <dt><b>-p</b>
// <dd>If characters in either the key or data items are printing characters
// (as defined by <b>isprint</b>(3)), use printing characters in
// <b>file</b> to represent them.  This option permits users to use standard
// text editors and tools to modify the contents of databases.
//
// Note, different systems may have different notions as to what characters
// are considered <i>printing characters</i>, and databases dumped in
// this manner may be less portable to external systems.
//
// <dt><b>-s</b>
// <dd>Specify a subdatabase to dump.  If no subdatabase is specified, all
// subdatabases found in the database are dumped.
//
// <dt><b>-V</b>
// <dd>Write the version number to the standard output and exit.
//
// </dl>
// 
// Dumping and reloading Hash databases that use user-defined hash functions
// will result in new databases that use the default hash function.
// While using the default hash function may not be optimal for the new database,
// it will continue to work correctly.
//
// Dumping and reloading Btree databases that use user-defined prefix or
// comparison functions will result in new databases that use the default
// prefix and comparison functions.
// <b>In this case, it is quite likely that the database will be damaged
// beyond repair permitting neither record storage or retrieval.</b>
//
// The only available workaround for either case is to modify the sources
// for the <i>htdb_load</i> utility to load the
// database using the correct hash, prefix and comparison functions.
//
// ENVIRONMENT
//
// <b>DB_HOME</b>
// If the <b>-h</b> option is not specified and the environment variable
// DB_HOME is set, it is used as the path of the database home.
// <br>
// <b>MIFLUZ_CONFIG</b>
// file name of configuration file read by WordContext(3). Defaults to
// <b>~/.mifluz.</b> 
//
// AUTHORS
//
// Sleepycat Software http://www.sleepycat.com/
//
//
// END
/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifndef lint
static const char copyright[] =
    "Copyright (c) 1996-2000\nSleepycat Software Inc.  All rights reserved.\n";
static const char revid[] =
    "$Id: htdb_dump.cc,v 1.1.2.4 2000/09/27 05:13:01 ghutchis Exp $";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

extern "C" {
#include "db_int.h"
#include "db_page.h"
#include "db_shash.h"
#include "btree.h"
#include "hash.h"
#include "lock.h"
}

#include "util_sig.h"

#include "WordContext.h"
#include "WordDBCompress.h"

void	 configure __P((char *));
int	 db_init __P((char *));
int	 dump __P((DB *, int, int));
int	 dump_sub __P((DB *, char *, int, int));
int	 is_sub __P((DB *, int *));
int	 main __P((int, char *[]));
int	 show_subs __P((DB *));
void	 usage __P((void));

DB_ENV	*dbenv;
const char
	*progname = "htdb_dump";				/* Program name. */

int
main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	DB *dbp;
	int ch, d_close;
	int e_close, exitval;
	int lflag, pflag, ret, rflag, Rflag, subs, keyflag;
	char *dopt, *home, *subname;
	int compress = 0;
	int wordlist = 0;
	u_int32_t cachesize = 0;
	WordContext *context = 0;

	dbp = NULL;
	d_close = e_close = exitval = lflag = pflag = rflag = Rflag = 0;
	keyflag = 0;
	dopt = home = subname = NULL;
	while ((ch = getopt(argc, argv, "d:f:h:klNprRs:VC:zW")) != EOF)
		switch (ch) {
		case 'd':
			dopt = optarg;
			break;
		case 'f':
			if (freopen(optarg, "w", stdout) == NULL) {
				fprintf(stderr, "%s: %s: reopen: %s\n",
				    progname, optarg, strerror(errno));
				exit (1);
			}
			break;
		case 'h':
			home = optarg;
			break;
		case 'k':
			keyflag = 1;
			break;
		case 'l':
			lflag = 1;
			break;
		case 'N':
			if ((ret = CDB_db_env_set_mutexlocks(0)) != 0) {
				fprintf(stderr,
				    "%s: db_env_set_mutexlocks: %s\n",
				    progname, CDB_db_strerror(ret));
				return (1);
			}
			if ((ret = CDB_db_env_set_panicstate(0)) != 0) {
				fprintf(stderr,
				    "%s: db_env_set_panicstate: %s\n",
				    progname, CDB_db_strerror(ret));
				return (1);
			}
			break;
		case 'p':
			pflag = 1;
			break;
		case 's':
			subname = optarg;
			break;
		case 'R':
			Rflag = 1;
			/* DB_AGGRESSIVE requires DB_SALVAGE */
			/* FALLTHROUGH */
		case 'r':
			rflag = 1;
			break;
		case 'V':
			printf("%s\n", CDB_db_version(NULL, NULL, NULL));
			exit(0);
		case 'C':
			cachesize = atoi(optarg);
			break;
		case 'z':
			compress = DB_COMPRESS;
			break;
		case 'W':
			wordlist = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	if (dopt != NULL && pflag) {
		fprintf(stderr,
		    "%s: the -d and -p options may not both be specified\n",
		    progname);
		exit (1);
	}
	if (lflag && subname != NULL) {
		fprintf(stderr,
		    "%s: the -l and -s options may not both be specified\n",
		    progname);
		exit (1);
	}

	if (keyflag && rflag) {
		fprintf(stderr, "%s: %s",
		    "the -k and -r or -R options may not both be specified\n",
		    progname);
		exit(1);
	}

	if (subname != NULL && rflag) {
		fprintf(stderr, "%s: %s",
		    "the -s and -r or R options may not both be specified\n",
		    progname);
		exit(1);
	}

	/* Handle possible interruptions. */
	__db_util_siginit();


	if(wordlist && compress) {
	  static ConfigDefaults defaults[] = {
	    { "wordlist_wordkey_description", "Word 24/DocID 32/Flag 8/Location 16"},
	    { "wordlist_env_skip", "true"},
	    { 0, 0, 0 }
	  };
	  context = new WordContext(defaults);
	}
	/*
	   * Create an environment object and initialize it for error
	   * reporting.
	   */
	if ((ret = CDB_db_env_create(&dbenv, 0)) != 0) {
	  fprintf(stderr,
		  "%s: CDB_db_env_create: %s\n", progname, CDB_db_strerror(ret));
	  goto err;
	}
	e_close = 1;

	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, progname);
	if(compress && wordlist) dbenv->mp_cmpr_info = (new WordDBCompress(context))->CmprInfo();

	/* Initialize the environment. */
	if (db_init(home) != 0)
		goto err;

	/* Create the DB object and open the file. */
	if ((ret = CDB_db_create(&dbp, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "CDB_db_create");
		goto err;
	}
	if(cachesize > 0) dbp->set_cachesize(dbp, 0, cachesize, 1);
	d_close = 1;

	/*
	 * If we're salvaging, don't do an open;  it might not be safe.
	 * Dispatch now into the salvager.
	 */
	if (rflag) {
		if ((ret = dbp->verify(dbp, argv[0], NULL, stdout,
		    DB_SALVAGE | (Rflag ? DB_AGGRESSIVE : 0))) != 0)
			goto err;
		exitval = 0;
		goto done;
	}

	if ((ret = dbp->open(dbp,
	    argv[0], subname, DB_UNKNOWN, (DB_RDONLY | compress), 0)) != 0) {
		dbp->err(dbp, ret, "open: %s", argv[0]);
		goto err;
	}

	if (dopt != NULL) {
		if (CDB___db_dump(dbp, dopt, NULL)) {
			dbp->err(dbp, ret, "CDB___db_dump: %s", argv[0]);
			goto err;
		}
	} else if (lflag) {
		if (is_sub(dbp, &subs))
			goto err;
		if (subs == 0) {
			dbp->errx(dbp,
			    "%s: does not contain multiple databases", argv[0]);
			goto err;
		}
		if (show_subs(dbp))
			goto err;
	} else {
		subs = 0;
		if (subname == NULL && is_sub(dbp, &subs))
			goto err;
		if (subs) {
			if (dump_sub(dbp, argv[0], pflag, keyflag))
				goto err;
		} else
			if (CDB___db_prheader(dbp, NULL, pflag, keyflag, stdout,
			    CDB___db_verify_callback, NULL, 0) ||
			    dump(dbp, pflag, keyflag))
				goto err;
	}

	if (0) {
err:		exitval = 1;
	}
done:	if (d_close && (ret = dbp->close(dbp, 0)) != 0) {
		exitval = 1;
		dbp->err(dbp, ret, "close");
	}
	if(wordlist && compress) {
	  delete (WordDBCompress*)dbenv->mp_cmpr_info->user_data;
	  delete dbenv->mp_cmpr_info;
	}
	if (e_close && (ret = dbenv->close(dbenv, 0)) != 0) {
		exitval = 1;
		fprintf(stderr,
		    "%s: dbenv->close: %s\n", progname, CDB_db_strerror(ret));
	}

	if(context) delete context;

	/* Resend any caught signal. */
	__db_util_sigresend();

	return (exitval);
}

/*
 * db_init --
 *	Initialize the environment.
 */
int
db_init(char *home)
{
	u_int32_t flags;
	int ret;

	/*
	 * Try and use the shared memory pool region when dumping a database,
	 * so our information is as up-to-date as possible, even if the mpool
	 * cache hasn't been flushed.
	 */
	flags = DB_USE_ENVIRON | DB_INIT_MPOOL | DB_INIT_LOCK;
	if (dbenv->open(dbenv, home, flags, 0) == 0)
		return (0);

	/*
	 * An environment is required because we may be trying to look at
	 * databases in directories other than the current one.  We could
	 * avoid using an environment iff the -h option wasn't specified,
	 * but that seems like more work than it's worth.
	 *
	 * No environment exists (or, at least no environment that includes
	 * an mpool region exists).  Create one, but make it private so that
	 * no files are actually created.
	 */
	LF_SET(DB_CREATE | DB_PRIVATE);
	if ((ret = dbenv->open(dbenv, home, flags, 0)) == 0)
		return (0);

	/* An environment is required. */
	dbenv->err(dbenv, ret, "open");
	return (1);
}

/*
 * is_sub --
 *	Return if the database contains subdatabases.
 */
int
is_sub(DB *dbp, int *yesno)
{
	DB_BTREE_STAT *btsp = 0;
	DB_HASH_STAT *hsp = 0;
	int ret;

	switch (dbp->type) {
	case DB_BTREE:
	case DB_RECNO:
		if ((ret = dbp->stat(dbp, &btsp, NULL, 0)) != 0) {
			dbp->err(dbp, ret, "DB->stat");
			return (ret);
		}
		*yesno = btsp->bt_metaflags & BTM_SUBDB ? 1 : 0;
		break;
	case DB_HASH:
		if ((ret = dbp->stat(dbp, &hsp, NULL, 0)) != 0) {
			dbp->err(dbp, ret, "DB->stat");
			return (ret);
		}
		*yesno = hsp->hash_metaflags & DB_HASH_SUBDB ? 1 : 0;
		break;
	case DB_QUEUE:
		break;
	default:
		dbp->errx(dbp, "unknown database type");
		return (1);
	}
	if(btsp) free(btsp);
	if(hsp) free(hsp);
	return (0);
}

/*
 * dump_sub --
 *	Dump out the records for a DB containing subdatabases.
 */
int
dump_sub(DB *parent_dbp, char *parent_name, int pflag, int keyflag)
{
	DB *dbp;
	DBC *dbcp;
	DBT key, data;
	int ret;
	char *subdb;

	/*
	 * Get a cursor and step through the database, dumping out each
	 * subdatabase.
	 */
	if ((ret = parent_dbp->cursor(parent_dbp, NULL, &dbcp, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB->cursor");
		return (1);
	}

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	while ((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
		/* Nul terminate the subdatabase name. */
		if ((subdb = (char*)malloc(key.size + 1)) == NULL) {
			dbenv->err(dbenv, ENOMEM, NULL);
			return (1);
		}
		memcpy(subdb, key.data, key.size);
		subdb[key.size] = '\0';

		/* Create the DB object and open the file. */
		if ((ret = CDB_db_create(&dbp, dbenv, 0)) != 0) {
			dbenv->err(dbenv, ret, "CDB_db_create");
			free(subdb);
			return (1);
		}
		if ((ret = dbp->open(dbp,
		    parent_name, subdb, DB_UNKNOWN, (DB_RDONLY | ((parent_dbp->flags & DB_AM_CMPR) ? DB_COMPRESS : 0)), 0)) != 0)
			dbp->err(dbp, ret,
			    "DB->open: %s:%s", parent_name, subdb);
		if (ret == 0 &&
		    (CDB___db_prheader(dbp, subdb, pflag, keyflag, stdout,
		    CDB___db_verify_callback, NULL, 0) ||
		     dump(dbp, pflag, keyflag)))
			ret = 1;
		(void)dbp->close(dbp, 0);
		free(subdb);
		if (ret != 0)
			return (1);
	}
	if (ret != DB_NOTFOUND) {
		dbp->err(dbp, ret, "DBcursor->get");
		return (1);
	}

	if ((ret = dbcp->c_close(dbcp)) != 0) {
		dbp->err(dbp, ret, "DBcursor->close");
		return (1);
	}

	return (0);
}

/*
 * show_subs --
 *	Display the subdatabases for a database.
 */
int
show_subs(DB *dbp)
{
	DBC *dbcp;
	DBT key, data;
	int ret;

	/*
	 * Get a cursor and step through the database, printing out the key
	 * of each key/data pair.
	 */
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->cursor");
		return (1);
	}

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	while ((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0) {
		if ((ret = CDB___db_prdbt(&key, 1, NULL, stdout,
		    CDB___db_verify_callback, 0, NULL)) != 0) {
			dbp->errx(dbp, NULL);
			return (1);
		}
	}
	if (ret != DB_NOTFOUND) {
		dbp->err(dbp, ret, "DBcursor->get");
		return (1);
	}

	if ((ret = dbcp->c_close(dbcp)) != 0) {
		dbp->err(dbp, ret, "DBcursor->close");
		return (1);
	}
	return (0);
}

/*
 * dump --
 *	Dump out the records for a DB.
 */
int
dump(DB *dbp, int pflag, int keyflag)
{
	DBC *dbcp;
	DBT key, data;
	int ret, is_recno;

	/*
	 * Get a cursor and step through the database, printing out each
	 * key/data pair.
	 */
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->cursor");
		return (1);
	}

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	is_recno = (dbp->type == DB_RECNO || dbp->type == DB_QUEUE);
	keyflag = is_recno ? keyflag : 1;
	while ((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0)
		if ((keyflag && (ret = CDB___db_prdbt(&key,
		    pflag, " ", stdout, CDB___db_verify_callback,
		    is_recno, NULL)) != 0) || (ret =
		    CDB___db_prdbt(&data, pflag, " ", stdout,
			CDB___db_verify_callback, 0, NULL)) != 0) {
			dbp->errx(dbp, NULL);
			return (1);
		}
	if (ret != DB_NOTFOUND) {
		dbp->err(dbp, ret, "DBcursor->get");
		return (1);
	}

	if ((ret = dbcp->c_close(dbcp)) != 0) {
		dbp->err(dbp, ret, "DBcursor->close");
		return (1);
	}

	(void)CDB___db_prfooter(stdout, CDB___db_verify_callback);
	return (0);
}

/*
 * usage --
 *	Display the usage message.
 */
void
usage()
{
	(void)fprintf(stderr, "usage: %s\n",
"htdb_dump [-klNprRVWz] [-C cachesize] [-d ahr] [-f output] [-h home] [-s database] db_file\n");
	exit(1);
}
