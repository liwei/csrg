/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This module is believed to contain source code proprietary to AT&T.
 * Use and redistribution is subject to the Berkeley Software License
 * Agreement and your Software Agreement with AT&T (Western Electric).
 */

#ifndef lint
static char sccsid[] = "@(#)move.c	8.1 (Berkeley) 06/04/93";
#endif /* not lint */

move(xi,yi){
		movep(xconv(xsc(xi)),yconv(ysc(yi)));
		return;
}
