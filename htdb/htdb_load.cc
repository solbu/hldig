//
// NAME
// 
// displays statistics for Berkeley DB environments.
//
// SYNOPSIS
//
// htdb_load [-nTzW] [-c name=value] [-f file] [-h home] [-C cachesize] [-t btree | hash | recno] db_file
//
// DESCRIPTION
//
// The htdb_load utility reads from the standard input and loads it into
// the database <b>db_file</b>.
// The database <b>db_file</b> is created if it does not already exist.
//
// The input to htdb_load must be in the output format specified by the
// htdb_dump utility, or as specified for the <b>-T</b> below.
//
// OPTIONS
// 
// <dl>
//
//
// <dt><b>-W</b>
// <dd>Initialize WordContext(3) before loading. With the <b>-z</b>
// flag allows to load inverted indexes using the mifluz(3) specific
// compression scheme. The MIFLUZ_CONFIG environment variable must be
// set to a file containing the mifluz(3) configuration.
//
// <dt><b>-z</b>
// <dd>The <b>db_file</b> is compressed. If <b>-W</b> is given the
// mifluz(3) specific compression scheme is used. Otherwise the default
// gzip compression scheme is used.
//
// <dt><b>-c</b>
// <dd>Specify configuration options for the DB structure 
// ignoring any value they may have based on the input.
// The command-line format is <b>name=value</b>.
// See <i>Supported Keywords</i> for
// a list of supported words for the <b>-c</b> option.
//
// <dt><b>-f</b>
// <dd>Read from the specified <b>input</b> file instead of from
// the standard input.
//
// <dt><b>-h</b>
// <dd>Specify a home directory for the database.
// If a home directory is specified, the database environment is opened using
// the <i>DB_INIT_LOCK</i>, <i>DB_INIT_LOG</i>, <i>DB_INIT_MPOOL</i>,
// <i>DB_INIT_TXN</i> and <i>DB_USE_ENVIRON</i> flags to
// DBENV-&gt;open. This means that htdb_load can be used to load
// data into databases while they are in use by other processes. If the
// DBENV-&gt;open call fails, or if no home directory is specified, the
// database is still updated, but the environment is ignored, e.g., no
// locking is done.
//
// <dt><b>-n</b>
// <dd>Do not overwrite existing keys in the database when loading into an
// already existing database.
// If a key/data pair cannot be loaded into the database for this reason,
// a warning message is displayed on the standard error output and the
// key/data pair are skipped.
// 
// <dt><b>-T</b>
// <dd>The <b>-T</b>
// option allows non-Berkeley DB applications to easily load text files 
// into databases.
//
// If the database to be created is of type Btree or Hash, or the keyword
// <b>keys</b> is specified as set, the input must be paired lines of text,
// where the first line of the pair is the key item, and the second line of
// the pair is its corresponding data item.  If the database to be created
// is of type Queue or Recno and the keywork <b>keys</b> is not set, the
// input must be lines of text, where each line is a new data item for the
// database.
// 
// A simple escape mechanism, where newline and backslash (\)
// characters are special, is applied to the text input.
// Newline characters are interpreted as record separators.
// Backslash characters in the text will be interpreted in one of two ways:
// if the backslash character precedes another backslash character, the pair
// will be interpreted as a literal backslash.
// If the backslash character precedes any other character, the two characters
// following the backslash will be interpreted as hexadecimal specification of
// a single character, e.g., \0a is a newline character in the ASCII
// character set.
// 
// For this reason, any backslash or newline characters that naturally
// occur in the text input must be escaped to avoid misinterpretation by
// htdb_load
// 
// If the <b>-T</b> option is specified, the underlying access method type
// must be specified using the <b>-t</b> option.
//
// <dt><b>-t</b>
// <dd>Specify the underlying access method.
// If no <b>-t</b> option is specified, the database will be loaded into a
// database of the same type as was dumped, e.g., a Hash database will be
// created if a Hash database was dumped.
// 
// Btree and Hash databases may be converted from one to the other.  Queue
// and Recno databases may be converted from one to the other.  If the
// <b>-k</b> option was specified on the call to htdb_dump then Queue
// and Recno databases may be converted to Btree or Hash, with the key being
// the integer record number.
//
// <dt><b>-V</b>
// <dd>Write the version number to the standard output and exit.
//
// </dl>
//
// The htdb_load utility attaches to one or more of the Berkeley DB
// shared memory regions.  In order to avoid region corruption, it 
// should always be given
// the chance to detach and exit gracefully.  To cause htdb_load to clean up
// after itself and exit, send it an interrupt signal (SIGINT).
//
// The htdb_load utility exits 0 on success, 1 if one or more key/data
// pairs were not loaded into the database because the key already existed,
// and &gt;1 if an error occurs.
// 
// KEYWORDS
//
// The following keywords are supported for the <b>-c</b> command-line option
// to the htdb_load utility. See DB-&gt;open for further discussion of
// these keywords and what values should be specified.
//
// The parenthetical listing specifies how the value part of the
// <b>name=value</b> pair is interpreted.
// Items listed as (boolean) expect value to be <b>1</b> (set) or <b>0</b>
// (unset).
// Items listed as (number) convert value to a number.
// Items listed as (string) use the string value without modification.
//
// <dl>
// <dt>bt_minkey (number)
// <dd>The minimum number of keys per page.
// <dt>db_lorder (number)
// <dd>The byte order for integers in the stored database metadata.
// <dt>db_pagesize (number)
// <dd>The size of pages used for nodes in the tree, in bytes.
// <dt>duplicates (boolean)
// <dd>The value of the DB_DUP flag.
// <dt>h_ffactor (number)
// <dd>The density within the Hash database.
// <dt>h_nelem (number)
// <dd>The size of the Hash database.
// <dt>keys (boolean)
// <dd>Specify if keys are present for Queue or Recno databases.
// <dt>re_len (number)
// <dd>Specify fixed-length records of the specified length.
// <dt>re_pad (string)
// <dd>Specify the fixed-length record pad character.
// <dt>recnum (boolean)
// <dd>The value of the DB_RECNUM flag.
// <dt>renumber (boolean)
// <dd>The value of the DB_RENUMBER flag.
// <dt>subdatabase (string)
// <dd>The subdatabase to load.
// </dl>
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
// 
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
    "$Id: htdb_load.cc,v 1.1.2.4 2000/09/27 05:13:01 ghutchis Exp $";
#endif

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

extern "C" {
#include "db_int.h"
#include "db_page.h"
#include "db_am.h"
#include "clib.h"
}

#include "util_sig.h"

#include "WordDBCompress.h"
#include "WordContext.h"
#include "WordKey.h"

void	badend __P((void));
void	badnum __P((void));
int	configure __P((DB *, char **, char **, int *));
int	db_init __P((char *));
int	dbt_rdump __P((DBT *));
int	dbt_rprint __P((DBT *));
int	dbt_rrecno __P((DBT *));
int	digitize __P((int, int *));
int	load __P((char *, DBTYPE, char **, int, u_int32_t, int, WordContext *));
int	main __P((int, char *[]));
int	rheader __P((DB *, DBTYPE *, char **, int *, int*));
void	usage __P((void));

int	endodata;			/* Reached the end of a database. */
int	endofile;			/* Reached the end of the input. */
int	existed;			/* Tried to load existing key. */
u_long	lineno;				/* Input file line number. */
int	version = 1;			/* Input version. */

DB_ENV	*dbenv;
const char
	*progname = "db_load";		/* Program name. */

int
main(int argc, char* argv[])
{
	extern char *optarg;
	extern int optind;
	DBTYPE dbtype;
	u_int32_t db_nooverwrite;
	int ch, exitval, no_header, ret;
	char **clist, **clp, *home;
	u_int32_t cachesize = 0;
	int compress = 0;
	int wordlist = 0;
	WordContext *context = 0;

	home = NULL;
	db_nooverwrite = 0;
	exitval = no_header = 0;
	dbtype = DB_UNKNOWN;

	/* Allocate enough room for configuration arguments. */
	if ((clp = clist = (char **)calloc(argc + 1, sizeof(char *))) == NULL) {
		fprintf(stderr, "%s: %s\n", progname, strerror(ENOMEM));
		exit(1);
	}

	while ((ch = getopt(argc, argv, "c:f:h:nTt:C:S:zWV")) != EOF)
		switch (ch) {
		case 'c':
			*clp++ = optarg;
			break;
		case 'f':
			if (freopen(optarg, "r", stdin) == NULL) {
				fprintf(stderr, "%s: %s: reopen: %s\n",
				    progname, optarg, strerror(errno));
				exit(1);
			}
			break;
		case 'h':
			home = optarg;
			break;
		case 'n':
			db_nooverwrite = DB_NOOVERWRITE;
			break;
		case 'T':
			no_header = 1;
			break;
		case 't':
			if (strcmp(optarg, "btree") == 0) {
				dbtype = DB_BTREE;
				break;
			}
			if (strcmp(optarg, "hash") == 0) {
				dbtype = DB_HASH;
				break;
			}
			if (strcmp(optarg, "recno") == 0) {
				dbtype = DB_RECNO;
				break;
			}
			if (strcmp(optarg, "queue") == 0) {
				dbtype = DB_QUEUE;
				break;
			}
			usage();
			/* NOTREACHED */
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
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();

	/* Handle possible interruptions. */
	__db_util_siginit();

	if(wordlist) {
	  static ConfigDefaults defaults[] = {
	    { "wordlist_wordkey_description", "Word 24/DocID 32/Flag 8/Location 16"},
	    { "wordlist_env_skip", "true"},
	    { 0, 0, 0 }
	  };
	  context = new WordContext(defaults);
	} 

	/*
	 * Create an environment object initialized for error reporting, and
	 * then open it.
	 */
	if ((ret = CDB_db_env_create(&dbenv, 0)) != 0) {
	  fprintf(stderr,
		  "%s: CDB_db_env_create: %s\n", progname, CDB_db_strerror(ret));
	  goto shutdown;
	}
	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, progname);
	if(cachesize > 0) dbenv->set_cachesize(dbenv, 0, cachesize, 1);
	if(compress && wordlist) dbenv->mp_cmpr_info = (new WordDBCompress(context))->CmprInfo();

	if (db_init(home) != 0)
		goto shutdown;

	while (!endofile)
		if (load(argv[0],
		    dbtype, clist, no_header, db_nooverwrite, compress, context) != 0)
			goto shutdown;

	if (0) {
shutdown:	exitval = 1;
	}
	if(wordlist && compress) {
	  delete (WordDBCompress*)dbenv->mp_cmpr_info->user_data;
	  delete dbenv->mp_cmpr_info;
	}
	if ((ret = dbenv->close(dbenv, 0)) != 0) {
		exitval = 1;
		fprintf(stderr,
		    "%s: dbenv->close: %s\n", progname, CDB_db_strerror(ret));
	}

	if(context) delete context;
	free(clist);
	/* Resend any caught signal. */
	__db_util_sigresend();

	/* Return 0 on success, 1 if keys existed already, and 2 on failure. */
	return (exitval == 0 ? (existed == 0 ? 0 : 1) : 2);
}

/*
 * load --
 *	Load a database.
 */
int
load(char *name, DBTYPE argtype, char **clist, int no_header, u_int32_t db_nooverwrite, int compress, WordContext* context)
{
	DB *dbp;
	DBT key, rkey, data, *readp, *writep;
	DBTYPE dbtype;
	db_recno_t recno, datarecno;
	int checkprint, ret, rval, keys;
	int keyflag, ascii_recno;
	char *subdb;

	endodata = 0;
	subdb = NULL;
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	/* Create the DB object. */
	if ((ret = CDB_db_create(&dbp, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "CDB_db_create");
		return (1);
	}

	dbtype = DB_UNKNOWN;
	keys = -1;
	keyflag = -1;
	/* Read the header -- if there's no header, we expect flat text. */
	if (no_header) {
		checkprint = 1;
		dbtype = argtype;
	} else {
		if (rheader(dbp, &dbtype, &subdb, &checkprint, &keys) != 0)
			goto err;
		if (endofile)
			goto done;
	}

	/*
	 * Apply command-line configuration changes.  (We apply command-line
	 * configuration changes to all databases that are loaded, e.g., all
	 * subdatabases.)
	 */
	if (configure(dbp, clist, &subdb, &keyflag))
		goto err;

#if 0
	if(subdb && !strcmp(subdb, "index") && context) dbp->set_bt_compare(dbp, word_db_cmp);
#endif

	if (keys != 1) {
		if (keyflag == 1) {
			dbp->err(dbp, EINVAL, "No keys specified in file");
			goto err;
		}
	}
	else if (keyflag == 0) {
		dbp->err(dbp, EINVAL, "Keys specified in file");
		goto err;
	}
	else
		keyflag = 1;

	if (dbtype == DB_BTREE || dbtype == DB_HASH) {
		if (keyflag == 0)
			dbp->err(dbp,
			    EINVAL, "Btree and Hash must specify keys");
		else
			keyflag = 1;
	}

	if (argtype != DB_UNKNOWN) {

		if (dbtype == DB_RECNO || dbtype == DB_QUEUE)
			if (keyflag != 1 && argtype != DB_RECNO
			     && argtype != DB_QUEUE){
				dbenv->errx(dbenv,
			   "improper database type conversion specified");
				goto err;
			}
		dbtype = argtype;
	}

	if (dbtype == DB_UNKNOWN) {
		dbenv->errx(dbenv, "no database type specified");
		goto err;
	}

	if (keyflag == -1)
		keyflag = 0;

	if (keyflag == 1 && (dbtype == DB_RECNO || dbtype == DB_QUEUE))
		ascii_recno = 1;
	else
		ascii_recno = 0;

	/* Open the DB file. */
	if ((ret = dbp->open(dbp,
	    name, subdb, dbtype, (DB_CREATE | compress), CDB___db_omode("rwrwrw"))) != 0) {
		dbp->err(dbp, ret, "DB->open: %s", name);
		goto err;
	}

	/* Initialize the key/data pair. */
	readp = &key;
	writep = &key;
	if (dbtype == DB_RECNO || dbtype == DB_QUEUE) {
		key.size = sizeof(recno);
		if (keyflag) {
			key.data = &datarecno;
			if (checkprint) {
				readp = &rkey;
				goto key_data;
			}
		}
		else
			key.data = &recno;
	} else
key_data:	if ((readp->data =
		    (void *)malloc(readp->ulen = 1024)) == NULL) {
			dbenv->err(dbenv, ENOMEM, NULL);
			goto err;
		}
	if ((data.data = (void *)malloc(data.ulen = 1024)) == NULL) {
		dbenv->err(dbenv, ENOMEM, NULL);
		goto err;
	}

	/* Get each key/data pair and add them to the database. */
	for (recno = 1; !__db_util_interrupted(); ++recno) {
		if (!keyflag)
			if (checkprint) {
				if (dbt_rprint(&data))
					goto err;
			} else {
				if (dbt_rdump(&data))
					goto err;
			}
		else
			if (checkprint) {
				if (dbt_rprint(readp))
					goto err;
				if (!endodata && dbt_rprint(&data))
					goto fmt;
			} else {
				if (ascii_recno) {
					if (dbt_rrecno(readp))
						goto err;
				} else
					if (dbt_rdump(readp))
						goto err;
				if (!endodata && dbt_rdump(&data)) {
fmt:					dbenv->errx(dbenv,
					    "odd number of key/data pairs");
					goto err;
				}
			}
		if (endodata)
			break;
		if (readp != writep) {
			if (sscanf((char*)readp->data, "%ud", &datarecno) != 1)
				dbenv->errx(dbenv,
				    "%s: non-integer key at line: %d",
				    name, !keyflag ? recno : recno * 2 - 1);
			if (datarecno == 0)
				dbenv->errx(dbenv, "%s: zero key at line: %d",
				    name,
				    !keyflag ? recno : recno * 2 - 1);
		}
		switch (ret =
		    dbp->put(dbp, NULL, writep, &data, db_nooverwrite)) {
		case 0:
			break;
		case DB_KEYEXIST:
			existed = 1;
			dbenv->errx(dbenv,
			    "%s: line %d: key already exists, not loaded:",
			    name,
			    !keyflag ? recno : recno * 2 - 1);

			(void)CDB___db_prdbt(&key, checkprint, 0, stderr,
			    CDB___db_verify_callback, 0, NULL);
			break;
		default:
			dbenv->err(dbenv, ret, NULL);
			goto err;
		}
	}
done:	rval = 0;

	if (0) {
err:		rval = 1;
	}

	/* Close the database. */
	if ((ret = dbp->close(dbp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->close");
		rval = 1;
	}

	/* Free allocated memory. */
	if (subdb != NULL)
		free(subdb);
	if (dbtype != DB_RECNO && dbtype != DB_QUEUE)
		free(key.data);
	free(data.data);

	return (rval);
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

	/* We may be loading into a live environment.  Try and join. */
	flags = DB_USE_ENVIRON |
	    DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
	if (dbenv->open(dbenv, home, flags, 0) == 0)
		return (0);

	/*
	 * We're trying to load a database.
	 *
	 * An environment is required because we may be trying to look at
	 * databases in directories other than the current one.  We could
	 * avoid using an environment iff the -h option wasn't specified,
	 * but that seems like more work than it's worth.
	 *
	 * No environment exists (or, at least no environment that includes
	 * an mpool region exists).  Create one, but make it private so that
	 * no files are actually created.
	 */
	LF_CLR(DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_TXN);
	LF_SET(DB_CREATE | DB_PRIVATE);
	if ((ret = dbenv->open(dbenv, home, flags, 0)) == 0)
		return (0);

	/* An environment is required. */
	dbenv->err(dbenv, ret, "DBENV->open");
	return (1);
}

#define	FLAG(name, value, keyword, flag)				\
	if (strcmp(name, keyword) == 0) {				\
		switch (*value) {					\
		case '1':						\
			if ((ret = dbp->set_flags(dbp, flag)) != 0) {	\
				dbp->err(dbp, ret, "%s: set_flags: %s",	\
				    progname, name);			\
				return (1);				\
			}						\
			break;						\
		case '0':						\
			break;						\
		default:						\
			badnum();					\
			return (1);					\
		}							\
		continue;						\
	}
#define	NUMBER(name, value, keyword, func)				\
	if (strcmp(name, keyword) == 0) {				\
		if (CDB___db_getlong(dbp,					\
		    NULL, value, 1, LONG_MAX, &val) != 0)		\
			return (1);					\
		if ((ret = dbp->func(dbp, val)) != 0)			\
			goto nameerr;					\
		continue;						\
	}
#define	STRING(name, value, keyword, func)				\
	if (strcmp(name, keyword) == 0) {				\
		if ((ret = dbp->func(dbp, value[0])) != 0)		\
			goto nameerr;					\
		continue;						\
	}

/*
 * configure --
 *	Handle command-line configuration options.
 */
int
configure(DB *dbp, char **clp, char **subdbp, int *keysp)
{
	long val;
	int ret, savech;
	char *name, *value;

	for (; (name = *clp) != NULL; *--value = savech, ++clp) {
		if ((value = strchr(name, '=')) == NULL) {
			dbp->errx(dbp,
		    "command-line configuration uses name=value format");
			return (1);
		}
		savech = *value;
		*value++ = '\0';

		if (strcmp(name, "database") == 0 ||
		    strcmp(name, "subdatabase") == 0) {
			if ((*subdbp = strdup(value)) == NULL) {
				dbp->err(dbp, ENOMEM, NULL);
				return (1);
			}
			continue;
		}
		if (strcmp(name, "keys") == 0) {
			if (strcmp(value, "1") == 0)
				*keysp = 1;
			else if (strcmp(value, "0") == 0)
				*keysp = 0;
			else {
				badnum();
				return (1);
			}
			continue;
		}

#ifdef notyet
		NUMBER(name, value, "bt_maxkey", set_bt_maxkey);
#endif
		NUMBER(name, value, "bt_minkey", set_bt_minkey);
		NUMBER(name, value, "db_lorder", set_lorder);
		NUMBER(name, value, "db_pagesize", set_pagesize);
		FLAG(name, value, "duplicates", DB_DUP);
		FLAG(name, value, "dupsort", DB_DUPSORT);
		NUMBER(name, value, "h_ffactor", set_h_ffactor);
		NUMBER(name, value, "h_nelem", set_h_nelem);
		NUMBER(name, value, "re_len", set_re_len);
		STRING(name, value, "re_pad", set_re_pad);
		FLAG(name, value, "recnum", DB_RECNUM);
		FLAG(name, value, "renumber", DB_RENUMBER);

		dbp->errx(dbp,
		    "unknown command-line configuration keyword");
		return (1);
	}
	return (0);

nameerr:
	dbp->err(dbp, ret, "%s: %s=%s", progname, name, value);
	return (1);
}

/*
 * rheader --
 *	Read the header message.
 */
int
rheader(DB *dbp, DBTYPE *dbtypep, char **subdbp, int *checkprintp, int *keysp)
{
	long val;
	int first, ret;
	char *name, *value, *p, buf[128];

	*dbtypep = DB_UNKNOWN;
	*checkprintp = 0;

	for (first = 1;; first = 0) {
		++lineno;

		/* If we don't see the expected information, it's an error. */
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			if (!first || ferror(stdin))
				goto badfmt;
			endofile = 1;
			break;
		}
		if ((p = strchr(name = buf, '=')) == NULL)
			goto badfmt;
		*p++ = '\0';
		if ((p = strchr(value = p, '\n')) == NULL)
			goto badfmt;
		*p = '\0';
		if (name[0] == '\0' || value[0] == '\0')
			goto badfmt;

		if (strcmp(name, "HEADER") == 0)
			break;
		if (strcmp(name, "VERSION") == 0) {
			/*
			 * Version 1 didn't have a "VERSION" header line, we
			 * only support versions 1 and 2 of the dump format.
			 */
			version = atoi(value);

			if (version != 2) {
				dbp->errx(dbp,
				    "line %lu: VERSION %d is unsupported",
				    lineno, version);
				return (1);
			}
			continue;
		}
		if (strcmp(name, "format") == 0) {
			if (strcmp(value, "bytevalue") == 0) {
				*checkprintp = 0;
				continue;
			}
			if (strcmp(value, "print") == 0) {
				*checkprintp = 1;
				continue;
			}
			goto badfmt;
		}
		if (strcmp(name, "type") == 0) {
			if (strcmp(value, "btree") == 0) {
				*dbtypep = DB_BTREE;
				continue;
			}
			if (strcmp(value, "hash") == 0) {
				*dbtypep = DB_HASH;
				continue;
			}
			if (strcmp(value, "recno") == 0) {
				*dbtypep = DB_RECNO;
				continue;
			}
			if (strcmp(value, "queue") == 0) {
				*dbtypep = DB_QUEUE;
				continue;
			}
			dbp->errx(dbp, "line %lu: unknown type", lineno);
			return (1);
		}
		if (strcmp(name, "database") == 0 ||
		    strcmp(name, "subdatabase") == 0) {
			if ((*subdbp = strdup(value)) == NULL) {
				dbp->err(dbp, ENOMEM, NULL);
				return (1);
			}
			continue;
		}
		if (strcmp(name, "keys") == 0) {
			if (strcmp(value, "1") == 0)
				*keysp = 1;
			else if (strcmp(value, "0") == 0)
				*keysp = 0;
			else {
				badnum();
				return (1);
			}
			continue;
		}

#ifdef notyet
		NUMBER(name, value, "bt_maxkey", set_bt_maxkey);
#endif
		NUMBER(name, value, "bt_minkey", set_bt_minkey);
		NUMBER(name, value, "db_lorder", set_lorder);
		NUMBER(name, value, "db_pagesize", set_pagesize);
		FLAG(name, value, "duplicates", DB_DUP);
		FLAG(name, value, "dupsort", DB_DUPSORT);
		NUMBER(name, value, "h_ffactor", set_h_ffactor);
		NUMBER(name, value, "h_nelem", set_h_nelem);
		NUMBER(name, value, "re_len", set_re_len);
		STRING(name, value, "re_pad", set_re_pad);
		FLAG(name, value, "recnum", DB_RECNUM);
		FLAG(name, value, "renumber", DB_RENUMBER);

		dbp->errx(dbp,
		    "unknown input-file header configuration keyword");
		return (1);
	}
	return (0);

nameerr:
	dbp->err(dbp, ret, "%s: %s=%s", progname, name, value);
	return (1);

badfmt:
	dbp->errx(dbp, "line %lu: unexpected format", lineno);
	return (1);
}

/*
 * dbt_rprint --
 *	Read a printable line into a DBT structure.
 */
int
dbt_rprint(DBT *dbtp)
{
	u_int32_t len;
	u_int8_t *p;
	int c1, c2, e, escape, first;
	char buf[32];

	++lineno;

	first = 1;
	e = escape = 0;
	for (p = (u_int8_t*)dbtp->data, len = 0; (c1 = getchar()) != '\n';) {
		if (c1 == EOF) {
			if (len == 0) {
				endofile = endodata = 1;
				return (0);
			}
			badend();
			return (1);
		}
		if (first) {
			first = 0;
			if (version > 1) {
				if (c1 != ' ') {
					buf[0] = c1;
					if (fgets(buf + 1,
					    sizeof(buf) - 1, stdin) == NULL ||
					    strcmp(buf, "DATA=END\n") != 0) {
						badend();
						return (1);
					}
					endodata = 1;
					return (0);
				}
				continue;
			}
		}
		if (escape) {
			if (c1 != '\\') {
				if ((c2 = getchar()) == EOF) {
					badend();
					return (1);
				}
				c1 = digitize(c1, &e) << 4 | digitize(c2, &e);
				if (e)
					return (1);
			}
			escape = 0;
		} else
			if (c1 == '\\') {
				escape = 1;
				continue;
			}
		if (len >= dbtp->ulen - 10) {
			dbtp->ulen *= 2;
			if ((dbtp->data =
			    (void *)realloc(dbtp->data, dbtp->ulen)) == NULL) {
				dbenv->err(dbenv, ENOMEM, NULL);
				return (1);
			}
			p = (u_int8_t *)dbtp->data + len;
		}
		++len;
		*p++ = c1;
	}
	dbtp->size = len;

	return (0);
}

/*
 * dbt_rdump --
 *	Read a byte dump line into a DBT structure.
 */
int
dbt_rdump(DBT *dbtp)
{
	u_int32_t len;
	u_int8_t *p;
	int c1, c2, e, first;
	char buf[32];

	++lineno;

	first = 1;
	e = 0;
	for (p = (u_int8_t*)dbtp->data, len = 0; (c1 = getchar()) != '\n';) {
		if (c1 == EOF) {
			if (len == 0) {
				endofile = endodata = 1;
				return (0);
			}
			badend();
			return (1);
		}
		if (first) {
			first = 0;
			if (version > 1) {
				if (c1 != ' ') {
					buf[0] = c1;
					if (fgets(buf + 1,
					    sizeof(buf) - 1, stdin) == NULL ||
					    strcmp(buf, "DATA=END\n") != 0) {
						badend();
						return (1);
					}
					endodata = 1;
					return (0);
				}
				continue;
			}
		}
		if ((c2 = getchar()) == EOF) {
			badend();
			return (1);
		}
		if (len >= dbtp->ulen - 10) {
			dbtp->ulen *= 2;
			if ((dbtp->data =
			    (void *)realloc(dbtp->data, dbtp->ulen)) == NULL) {
				dbenv->err(dbenv, ENOMEM, NULL);
				return (1);
			}
			p = (u_int8_t *)dbtp->data + len;
		}
		++len;
		*p++ = digitize(c1, &e) << 4 | digitize(c2, &e);
		if (e)
			return (1);
	}
	dbtp->size = len;

	return (0);
}

/*
 * dbt_rrecno --
 *	Read a record number dump line into a DBT structure.
 */
int
dbt_rrecno(DBT *dbtp)
{
	char buf[32];

	++lineno;

	if (fgets(buf, sizeof(buf), stdin) == NULL) {
		endofile = endodata = 1;
		return (0);
	}

	if (strcmp(buf, "DATA=END\n") == 0) {
		endodata = 1;
		return (0);
	}

	if (buf[0] != ' ' || CDB___db_getulong(NULL,
	    progname, buf + 1, 0, 0, (u_long *)dbtp->data)) {
		badend();
		return (1);
	}

	dbtp->size = sizeof(db_recno_t);
	return (0);
}

/*
 * digitize --
 *	Convert a character to an integer.
 */
int
digitize(int c, int *errorp)
{
	switch (c) {			/* Don't depend on ASCII ordering. */
	case '0': return (0);
	case '1': return (1);
	case '2': return (2);
	case '3': return (3);
	case '4': return (4);
	case '5': return (5);
	case '6': return (6);
	case '7': return (7);
	case '8': return (8);
	case '9': return (9);
	case 'a': return (10);
	case 'b': return (11);
	case 'c': return (12);
	case 'd': return (13);
	case 'e': return (14);
	case 'f': return (15);
	}

	dbenv->errx(dbenv, "unexpected hexadecimal value");
	*errorp = 1;

	return (0);
}

/*
 * badnum --
 *	Display the bad number message.
 */
void
badnum()
{
	dbenv->errx(dbenv,
	    "boolean name=value pairs require a value of 0 or 1");
}

/*
 * badend --
 *	Display the bad end to input message.
 */
void
badend()
{
	dbenv->errx(dbenv, "unexpected end of input data or key/data pair");
}

/*
 * usage --
 *	Display the usage message.
 */
void
usage()
{
	(void)fprintf(stderr, "%s\n\t%s\n",
	    "usage: db_load [-nTzWV]",
    "[-c name=value] [-f file] [-h home] [-C cachesize] [-t btree | hash | recno] db_file");
	exit(1);
}
