/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1982, 1986, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
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
 *
 * from: Utah $Hdr: pcb.h 1.13 89/04/23$
 *
 *	@(#)pcb.h	7.1 (Berkeley) 05/08/90
 */

#ifdef KERNEL
#include "frame.h"
#else
#include <hp300/frame.h>
#endif

/*
 * HP300 process control block
 */
struct pcb
{
	short	pcb_flags;	/* misc. process flags (+0) */
	short	pcb_ps; 	/* processor status word (+2) */
	int	pcb_ustp;	/* user segment table pointer (+4) */
	int	pcb_usp;	/* user stack pointer (+8) */
	int	pcb_regs[12];	/* D0-D7, A0-A7 (+C) */
	struct pte *pcb_p0br;	/* P0 base register (+3C) */
	int	pcb_p0lr;	/* P0 length register (+40) */
	struct pte *pcb_p1br;	/* P1 base register (+44) */
	int	pcb_p1lr;	/* P1 length register (+48) */
	int	pcb_szpt; 	/* number of pages of user page table (+4C) */
	int	pcb_cmap2;	/* temporary copy PTE (+50) */
	int	*pcb_sswap;	/* saved context for swap return (+54) */
	short	pcb_sigc[12];	/* signal trampoline code (+58) */
	caddr_t	pcb_onfault;	/* for copyin/out faults (+70) */
	struct fpframe pcb_fpregs; /* 68881/2 context save area (+74) */
	int	pcb_exec[16];	/* exec structure for core dumps (+1B8) */
	int	pcb_res[2];	/* reserved for future expansion (+1F8) */
};

/* flags */

#define	PCB_AST		0x0001	/* async trap pending */
#define PCB_HPUXMMAP	0x0010	/* VA space is multiple mapped */
#define PCB_HPUXTRACE	0x0020	/* being traced by an HPUX process */
#define PCB_HPUXBIN	0x0040	/* loaded from an HPUX format binary */
				/* note: does NOT imply SHPUX */

#define aston() \
	{ \
		u.u_pcb.pcb_flags |= PCB_AST; \
	}

#define astoff() \
	{ \
		u.u_pcb.pcb_flags &= ~PCB_AST; \
	}

#define astpend() \
	(u.u_pcb.pcb_flags & PCB_AST)
