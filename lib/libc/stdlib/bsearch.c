/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)bsearch.c	5.1 (Berkeley) 10/14/89";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <stdio.h>

char *
bsearch(key, base, nmemb, size, compar)
	register char *key, *base;
	size_t nmemb;
	register size_t size;
	register int (*compar)();
{
	register int bottom, middle, result, top;
	register char *p;

	for (bottom = 0, top = nmemb - 1; bottom <= top;) {
		middle = (bottom + top) >> 1;
		p = base + middle * size;
		if (!(result = (*compar)(key, p)))
			return(p);
		if (result > 0)
			bottom = middle + 1;
		else
			top = middle - 1;
	}
	return(NULL);
}
