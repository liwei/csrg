/*-
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rodney Ruddock of the University of Guelph.
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
static char sccsid[] = "@(#)z.c	5.1 (Berkeley) 01/23/93";
#endif /* not lint */

#include "ed.h"

/*
 * This prints out the next group of lines (as spec'd by the skicky
 * number; 22 by default). It's really useful for scrolling in chunks
 * through the buffer (better than l, n, or p). Shame on POSIX for
 * not including it; yaaa! BSD for keeping it! :-)
 */

void
z(inputt, errnum)

FILE *inputt;
int *errnum;

{
  register int l_cnt;

  if (current == NULL)
    {
      *errnum = -1;
      strcpy(help_msg, "no lines in buffer");
      return;
    }
  /* set zsnum if need be here */
  ss = getc(inputt);
  if ((ss > 48) && (ss < 57))
    zsnum = dig_num_conv(inputt, errnum);  /* default set in main */
  else if ((ss != '\n') && (ss != EOF))
    {
      ungetc(ss, inputt);
      if (rol(inputt, errnum))
        return;
    }

  if (sigint_flag)
    SIGINT_ACTION;
  if (End_default)
    {
      if ((current->below) != NULL)
        start = current->below;
      else
        {
          strcpy(help_msg, "at end of buffer");
          *errnum = -1;
          ungetc('\n', inputt);
          return;
        }
    }
  else
    start = End;
  if (start == NULL)
    {
      strcpy(help_msg, "bad address");
      *errnum = -1;
      ungetc('\n', inputt);
      return;
    }
  start_default = End_default = 0;

  current = start;
  l_cnt = 1;  /* yes, set to = 1 */
  while (1)
     {
       /* scroll-out the next 'zsnum' of lines or until bottom */
       if (sigint_flag)
         SIGINT_ACTION;
       if (current == NULL)
         break;
       get_line(current->handle, current->len);
       printf("%s\n", text);
       if (current == bottom)
         break;
       l_cnt++;
       if (zsnum < l_cnt)
         break;
       current = current->below;
     } /* end-while */
  *errnum = 1;
} /* end-z */
