/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * John B. Roll Jr.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1990 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)xargs.c	5.7 (Berkeley) 03/05/91";
#endif /* not lint */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "pathnames.h"

#define	DEF_ARGC	255

int tflag;
int fflag;

main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno, optind;
	extern char *optarg;
	register int ch;
	register char *p, *bp, *endbp, **bxp, **endxp, **xp;
	int cnt, indouble, insingle, nargs, nline;
	char *mark, *prog, **xargs;

	nargs = DEF_ARGC;
	nline = _POSIX2_LINE_MAX;

	while ((ch = getopt(argc, argv, "n:s:tf")) != EOF)
		switch(ch) {
		case 'n':
			if ((nargs = atoi(optarg)) <= 0) {
				(void)fprintf(stderr,
				    "xargs: bad argument count.\n");
				exit(1);
			}
			break;
		case 's':
			if ((nline = atoi(optarg)) <= 0) {
				(void)fprintf(stderr,
				    "xargs: bad command length.\n");
				exit(1);
			}
			break;
		case 't':
			tflag = 1;
			break;
		case 'f':
			fflag = 1;
			break;
		case '?':
		default:
			usage();
	}
	argc -= optind;
	argv += optind;

	/* room for the command, leftover arguments and trailing NULL */
	if (!(xargs =
	    (char **)malloc((u_int)(nargs + argc + 2) * sizeof(char **))))
		enomem();

	if (!(bp = malloc((u_int)nline + 1)))
		enomem();

	xp = xargs + 1;
	if (!*argv)
		prog = _PATH_ECHO;
	else {
		prog = *argv;
		while (*++argv)
			*xp++ = *argv;
	}

	if (xargs[0] = rindex(prog, '/'))
		++xargs[0];
	else
		xargs[0] = prog;

	/* set up the pointers into the buffer and the arguments */
	*(endxp = (bxp = xp) + nargs) = NULL;
	endbp = (mark = p = bp) + nline;

	insingle = indouble = 0;
	for (;;)
		switch(ch = getchar()) {
		case EOF:
			if (p == bp)		/* nothing to display */
				exit(0);
			if (mark == p) {	/* nothing since last arg end */
				*xp = NULL;
				run(prog, xargs);
				exit(0);
			}
			goto addarg;
		case ' ':
		case '\t':
			if (insingle || indouble)
				goto addch;
			goto addarg;
		case '\n':
			if (mark == p)			/* empty line */
				continue;
addarg:			*xp++ = mark;
			*p++ = '\0';
			if (xp == endxp || p >= endbp || ch == EOF) {
				if (insingle || indouble) {
					(void)fprintf(stderr,
					   "xargs: unterminated quote.\n");
					exit(1);
				}
				*xp = NULL;
				run(prog, xargs);
				if (ch == EOF)
					exit(0);
				p = bp;
				xp = bxp;
			}
			mark = p;
			break;
		case '\'':
			if (indouble)
				goto addch;
			insingle = !insingle;
			break;
		case '"':
			if (insingle)
				goto addch;
			indouble = !indouble;
			break;
		case '\\':
			if ((ch = getchar()) == EOF)
				ch = '\\';
			if (ch == '\n') {
				(void)fprintf(stderr,
				    "xargs: newline may not be escaped.\n");
				exit(1);
			}
			/* FALLTHROUGH */
		default:
addch:			if (p != endbp) {
				*p++ = ch;
				continue;
			}
			if (xp == bxp) {
				(void)fprintf(stderr,
				    "xargs: argument too large.\n");
				exit(1);
			}
			*xp = NULL;
			run(prog, xargs);
			cnt = endbp - mark;
			bcopy(mark, bp, cnt);
			p = (mark = bp) + cnt;
			*p++ = ch;
			xp = bxp;
			break;
		}
	/* NOTREACHED */
}

run(prog, argv)
	char *prog, **argv;
{
	union wait pstat;
	pid_t pid;
	char **p;

	if (tflag) {
		(void)fprintf(stderr, "%s", *argv);
		for (p = argv + 1; *p; ++p)
			(void)fprintf(stderr, " %s", *p);
		(void)fprintf(stderr, "\n");
		(void)fflush(stderr);
	}
	switch(pid = vfork()) {
	case -1:
		(void)fprintf(stderr,
		   "xargs: vfork: %s.\n", strerror(errno));
		exit(1);
	case 0:
		execvp(prog, argv);
		(void)fprintf(stderr,
		   "xargs: %s: %s.\n", prog, strerror(errno));
		_exit(1);
	}
	pid = waitpid(pid, (int *)&pstat, 0);
	if (pid == -1) {
		(void)fprintf(stderr,
		   "xargs: waitpid: %s.\n", strerror(errno));
		exit(1);
	}
	if (!fflag && pstat.w_status)
		exit(1);
}

enomem()
{
	(void)fprintf(stderr, "xargs: %s.\n", strerror(ENOMEM));
	exit(1);
}

usage()
{
	(void)fprintf(stderr,
	    "xargs: [-t] [-f] [-n number] [-s size] [utility [argument ...]]\n");
	exit(1);
}
