/*
char id_signal[] = "@(#)signal_.c	1.1";
 *
 * change the action for a specified signal
 *
 * calling sequence:
 *	integer cursig, signal, savsig
 *	external proc
 *	cursig = signal(signum, proc, flag)
 * where:
 *	'cursig' will receive the current value of signal(2)
 *	'signum' must be in the range 0 <= signum <= 16
 *
 *	If 'flag' is negative, 'proc' must be an external proceedure name.
 *	
 *	If 'flag' is 0 or positive, it will be passed to signal(2) as the
 *	signal action flag. 0 resets the default action; 1 sets 'ignore'.
 *	'flag' may be the value returned from a previous call to signal.
 */

#include	"../libI77/f_errno.h"

/*** NOTE: the type casts for procp and signal are problematical but work ***/
int (*signal())();

long signal_(sigp, procp, flag)
long *sigp, *flag;
int (*procp)();
{
	if (*sigp < 0 || *sigp > 16)
		return(-((long)(errno=F_ERARG)));

	if (*flag < 0)	/* function address passed */
		return((long)signal((int)*sigp, procp) );

	else		/* integer value passed */
		return((long)signal((int)*sigp, (int)*flag) );
}
