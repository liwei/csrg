/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
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
static char sccsid[] = "@(#)tty.c	5.12 (Berkeley) 05/16/93";
#endif /* not lint */

/*
 * Terminal initialization routines.
 */
#include <sys/ioctl.h>

#include <curses.h>
#include <termios.h>
#include <unistd.h>

struct termios __orig_termios;
static struct termios norawt, rawt;
static int useraw;

/*
 * gettmode --
 *	Do terminal type initialization.
 */
int
gettmode()
{
	useraw = 0;
	
	if (tcgetattr(STDIN_FILENO, &__orig_termios))
		return (ERR);

	GT = (__orig_termios.c_oflag & OXTABS) == 0;
	NONL = (__orig_termios.c_oflag & ONLCR) == 0;

	norawt = __orig_termios;
	norawt.c_oflag &= ~OXTABS;
	rawt = norawt;
	cfmakeraw(&rawt);

	return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt) ? ERR : OK);
}

int
raw()
{
	useraw = __pfast = __rawmode = 1;
	return (tcsetattr(STDIN_FILENO, TCSADRAIN, &rawt));
}

int
noraw()
{
	useraw = __pfast = __rawmode = 0;
	return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt));
}

int
cbreak()
{
	rawt.c_lflag &= ~ICANON;
	norawt.c_lflag &= ~ICANON;

	__rawmode = 1;
	if (useraw)
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &rawt));
	else
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt));
}

int
nocbreak()
{
	rawt.c_lflag |= ICANON;
	norawt.c_lflag |= ICANON;

	__rawmode = 0;
	if (useraw) 
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &rawt));
	else
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt));
}
	
int
echo()
{
	rawt.c_lflag |= ECHO;
	norawt.c_lflag |= ECHO;
	
	__echoit = 1;
	if (useraw) 
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &rawt));
	else
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt));
}

int
noecho()
{
	rawt.c_lflag &= ~ECHO;
	norawt.c_lflag &= ~ECHO;
	
	__echoit = 0;
	if (useraw) 
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &rawt));
	else
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt));
}

int
nl()
{
	rawt.c_iflag |= ICRNL;
	rawt.c_oflag |= ONLCR;
	norawt.c_iflag |= ICRNL;
	norawt.c_oflag |= ONLCR;

	__pfast = __rawmode;
	if (useraw) 
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &rawt));
	else
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt));
}

int
nonl()
{
	rawt.c_iflag &= ~ICRNL;
	rawt.c_oflag &= ~ONLCR;
	norawt.c_iflag &= ~ICRNL;
	norawt.c_oflag &= ~ONLCR;

	__pfast = 1;
	if (useraw) 
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &rawt));
	else
		return (tcsetattr(STDIN_FILENO, TCSADRAIN, &norawt));
}

void
__startwin()
{
	(void)fflush(stdout);
	(void)setvbuf(stdout, NULL, _IOFBF, 0);

	tputs(TI, 0, __cputchar);
	tputs(VS, 0, __cputchar);
}

int
endwin()
{
	if (curscr != NULL) {
		if (curscr->flags & __WSTANDOUT) {
			tputs(SE, 0, __cputchar);
			curscr->flags &= ~__WSTANDOUT;
		}
		__mvcur(curscr->cury, curscr->cury, curscr->maxy - 1, 0);
	}

	(void)tputs(VE, 0, __cputchar);
	(void)tputs(TE, 0, __cputchar);
	(void)fflush(stdout);
	(void)setvbuf(stdout, NULL, _IOLBF, 0);

	return (tcsetattr(STDIN_FILENO, TCSADRAIN, &__orig_termios));
}

/*
 * The following routines, savetty and resetty are completely useless and
 * are left in only as stubs.  If people actually use them they will almost
 * certainly screw up the state of the world.
 */
static struct termios savedtty;
int
savetty()
{
	return (tcgetattr(STDIN_FILENO, &savedtty));
}

int
resetty()
{
	return (tcsetattr(STDIN_FILENO, TCSADRAIN, &savedtty));
}
