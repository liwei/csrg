#ifndef lint
static char sccsid[] = "@(#)checkuse.c	1.3 (Berkeley) 07/27/93";
#endif lint


/*
 * check which entries exist, also storage allocation
 */

#include "defs.h"
#include "ext.h"

checkuse()
{
	register int i, c, k;

	for(c = 0; c < ncol; c++){
		used[c] = lused[c] = rused[c] = 0;
		for(i = 0; i < nlin; i++){
			if(instead[i] || fullbot[i])
				continue;
			k = ctype(i, c);
			if(k == '-' || k == '=')
				continue;
			if((k == 'n' || k == 'a')){
				rused[c] |= real(table[i][c].rcol);
				if(!real(table[i][c].rcol))
					used[c] |= real(table[i][c].col);
				if(table[i][c].rcol)
					lused[c] |= real(table[i][c].col);
			} else
				used[c] |= real(table[i][c].col);
		}
	}
}

real(s)
char *s;
{
	if(s == 0)
		return(0);
	if(!point(s))
		return(1);
	if(*s == 0)
		return(0);
	return(1);
}

#define MAXVEC 128

static int spcount = 0;
extern char *calloc();
static char *spvecs[MAXVEC];

char *
chspace()
{
	register char *pp;

	if(spvecs[spcount])
		return(spvecs[spcount++]);
	if(spcount >= MAXVEC)
		error("Too many characters in table");
	spvecs[spcount++] = pp = calloc(MAXCHS + 200, 1);
	if(pp == (char *) -1 || pp ==  (char *) 0)
		error("no space for characters");
	return(pp);
}

#define MAXPC 256

static char *thisvec;
static int tpcount = -1;
static char *tpvecs[MAXPC];

int *
alocv(n)
{
	register int *tp, *q;
	if(tpcount < 0 || thisvec + n > tpvecs[tpcount] + MAXCHS){
		tpcount++;
		if(tpvecs[tpcount] == 0){
			tpvecs[tpcount] = calloc(MAXCHS, 1);
		}
		thisvec = tpvecs[tpcount];
		if(thisvec == (char *) -1)
			error("no space for vectors");
	}
	tp = (int *)thisvec;
	thisvec += n;
	for(q = tp; q < (int *)thisvec; q++)
		*q = 0;
	return(tp);
}

release(){
	extern char *exstore;

	/*
	 * give back unwanted space in some vectors
	 */
	spcount = 0;
	tpcount = -1;
	exstore = 0;
}
