/*
char id_kill[] = "@(#)kill_.c	1.1";
 *
 * send a signal to a process
 *
 * calling sequence:
 *	ierror = kill(pid, signum)
 * where:
 *	pid must be the process id of one of the user's processes
 *	signum must be a valid signal number (see signal(2))
 *	ierror will be 0 if successful; an error code otherwise.
 */

#include "../libI77/f_errno.h"

long kill_(pid, signum)
long *pid, *signum;
{
	if (*pid < 0 || *pid > 32767L || *signum < 1 || *signum > 16)
		return((long)(errno=F_ERARG));
	if (kill((int)*pid, (int)*signum) != 0)
		return((long)errno);
	return(0L);
}
