/* 
 * Copyright (C) Dirk Husemann, Computer Science Department IV, 
 * 		 University of Erlangen-Nuremberg, Germany, 1990, 1991, 1992
 * Copyright (c) 1992   Regents of the University of California.
 * All rights reserved.
 * 
 * This code is derived from software contributed to Berkeley by
 * Dirk Husemann and the Computer Science Department (IV) of
 * the University of Erlangen-Nuremberg, Germany.
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
 *	@(#)pk_llcsubr.c	7.1 (Berkeley) 12/08/92
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/domain.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/kernel.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_llc.h>
#include <net/if_types.h>
#include <net/route.h>

#include <netccitt/dll.h>
#include <netccitt/x25.h>
#include <netccitt/pk.h>
#include <netccitt/pk_var.h>
#include <netccitt/llc_var.h>


/*
 * Routing support for X.25
 *
 * We distinguish between two cases:
 * RTF_HOST:
 * 	rt_key(rt)	X.25 address of host
 *	rt_gateway	SNPA (MAC+DLSAP) address of host
 *	rt_llinfo	pkcb for rt_key(rt)
 *
 * RTF_GATEWAY
 *	rt_key(rt)	X.25 address of host or suitably masked network
 *	rt_gateway	X.25 address of next X.25 gateway (switch)
 *	rt_llinfo	rtentry for rt_gateway address
 *			ought to be of type RTF_HOST
 *
 *
 * Mapping of X.121 to pkcbs:
 *
 * HDLC uses the DTE-DCE model of X.25, therefore we need a many-to-one
 * relationship, i.e.:
 *	
 * 	{X.121_a, X.121_b, X.121_c, ..., X.121_i} -> pkcb_0
 *
 * LLC2 utilizes the DTE-DTE model of X.25, resulting effectively in a
 * one-to-one relationship, i.e.:
 *
 *	{X.121_j} 	->	pkcb_1a
 *	{X.121_k}	->	pkcb_1b
 *	...
 *	{X.121_q}	->	pkcb_1q
 * 
 * It might make sense to allow a many-to-one relation for LLC2 also,
 * 
 *	{X.121_r, X.121_s, X.121_t, X.121_u} -> pkcb_2a
 *
 * This would make addresses X.121_[r-u] essentially aliases of one
 * address ({X.121_[r-u]} would constitute a representative set).
 *
 * Each one-to-one relation must obviously be entered individually with
 * a route add command, whereas a many-to-one relationship can be 
 * either entered individually or generated by using a netmask.
 * 
 * To facilitate dealings the many-to-one case for LLC2 can only be
 * established via a netmask.
 *
 */

#define XTRACTPKP(rt)	((rt)->rt_flags & RTF_GATEWAY ? \
			 ((rt)->rt_llinfo ? \
			  (struct pkcb *) ((struct rtentry *)((rt)->rt_llinfo))->rt_llinfo : \
			  (struct pkcb *) NULL) : \
			 (struct pkcb *)((rt)->rt_llinfo))

#define equal(a1, a2) (bcmp((caddr_t)(a1), \
			       (caddr_t)(a2), \
			       (a1)->sa_len) == 0)
#define XIFA(rt) ((struct x25_ifaddr *)((rt)->rt_ifa))

int
cons_rtrequest(int cmd, struct rtentry *rt, struct sockaddr *dst)
{
	register struct pkcb *pkp;
	register int i;
	register char one_to_one;
	struct pkcb *pk_newlink();
	struct rtentry *npaidb_enter();

	pkp = XTRACTPKP(rt);

	switch(cmd) {
	case RTM_RESOLVE:
	case RTM_ADD:
		if (pkp) 
			return(EEXIST);

		if (rt->rt_flags & RTF_GATEWAY) {
			if (rt->rt_llinfo)
				RTFREE((struct rtentry *)rt->rt_llinfo);
			rt->rt_llinfo = (caddr_t) rtalloc1(rt->rt_gateway, 1);
			return(0);
		}
		/*
		 * Assumptions:	(1) ifnet structure is filled in
		 *		(2) at least the pkcb created via 
		 *		    x25config (ifconfig?) has been 
		 *		    set up already.
		 *		(3) HDLC interfaces have an if_type of 
		 *		    IFT_X25{,DDN}, LLC2 interfaces 
		 *		    anything else (any better way to 
		 *		    do this?)
		 *
		 */
		if (!rt->rt_ifa)
			return (ENETDOWN);
	
		/*	
		 * We differentiate between dealing with a many-to-one
		 * (HDLC: DTE-DCE) and a one-to-one (LLC2: DTE-DTE) 
		 * relationship (by looking at the if type).
		 *
		 * Only in case of the many-to-one relationship (HDLC)
		 * we set the ia->ia_pkcb pointer to the pkcb allocated
		 * via pk_newlink() as we will use just that one pkcb for
		 * future route additions (the rtentry->rt_llinfo pointer
		 * points to the pkcb allocated for that route).
		 *
		 * In case of the one-to-one relationship (LLC2) we 
		 * create a new pkcb (via pk_newlink()) for each new rtentry.
		 * 
		 * NOTE: Only in case of HDLC does ia->ia_pkcb point
		 * to a pkcb, in the LLC2 case it doesn't (as we don't 
		 * need it here)!
		 */
		one_to_one = ISISO8802(rt->rt_ifp);

		if (!(pkp = XIFA(rt)->ia_pkcb) && !one_to_one) 
			XIFA(rt)->ia_pkcb = pkp = 
				pk_newlink(XIFA(rt), (caddr_t) 0);
		else if (one_to_one && 
			 !equal(rt->rt_gateway, rt->rt_ifa->ifa_addr)) {
			pkp = pk_newlink(XIFA(rt), (caddr_t) 0);
			/*
			 * We also need another route entry for mapping
			 * MAC+LSAP->X.25 address
			 */
			pkp->pk_llrt = npaidb_enter(rt->rt_gateway, rt_key(rt), rt, 0);
		}
		if (pkp) {
			if (!pkp->pk_rt)
				pkp->pk_rt = rt;
			pkp->pk_refcount++;
		}
		rt->rt_llinfo = (caddr_t) pkp;

		return(0);

	case RTM_DELETE:
	{
		/*
		 * The pkp might be empty if we are dealing
		 * with an interface route entry for LLC2, in this 
		 * case we don't need to do anything ...
		 */
		if (pkp) {
			if ( rt->rt_flags & RTF_GATEWAY ) {
				if (rt->rt_llinfo)
					RTFREE((struct rtentry *)rt->rt_llinfo);
				return(0);
			}
			
			if (pkp->pk_llrt)
				npaidb_destroy(pkp->pk_llrt);

			pk_dellink (pkp);
			
			return(0);
		}
	}
	}
}


/*
 * Glue between X.25 and LLC2
 */
int
x25_llcglue(int prc, struct sockaddr *addr)
{
	register struct sockaddr_x25 *sx25 = (struct sockaddr_x25 *)addr;
	register struct x25_ifaddr *x25ifa;
	struct dll_ctlinfo ctlinfo;
	
	if((x25ifa = (struct x25_ifaddr *)ifa_ifwithaddr(addr)) == 0)
		return 0;

	ctlinfo.dlcti_cfg  =
	    (struct dllconfig *)(((struct sockaddr_x25 *)(&x25ifa->ia_xc))+1);
	ctlinfo.dlcti_lsap = LLC_X25_LSAP;

	return ((int)llc_ctlinput(prc, addr, (caddr_t)&ctlinfo));
}
