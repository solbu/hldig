/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2000
 *	Sleepycat Software.  All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <signal.h>

extern "C" {
#include "db_int.h"
#include "common_ext.h"
}

static int interrupt;
static void onint(int);

/*
 * onint --
 *	Interrupt signal handler.
 */
static void onint(int signo)
{
    if ((interrupt = signo) == 0)
	interrupt = SIGINT;
}

void __db_util_siginit()
{
    /*
     * Initialize the set of signals for which we want to clean up.
     * Generally, we try not to leave the shared regions locked if
     * we can.
     */
#ifdef SIGHUP
    (void) signal(SIGHUP, onint);
#endif
    (void) signal(SIGINT, onint);
#ifdef SIGPIPE
    (void) signal(SIGPIPE, onint);
#endif
    (void) signal(SIGTERM, onint);
}

int __db_util_interrupted()
{
    return (interrupt != 0);
}

void __db_util_sigresend()
{
    /* Resend any caught signal. */
    if (__db_util_interrupted != 0) {
	(void) signal(interrupt, SIG_DFL);
	(void) raise(interrupt);
	/* NOTREACHED */
    }
}
