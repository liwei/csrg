/*	cp.h	1.2	86/12/06	*/

/*
 * Tahoe console processor interface
 */

/*
 * Reduced DCB layout for byte communication.
 */
#define	CPBUFLEN 200		/* Output buffer length */
#ifndef	LOCORE
struct	cphdr {
	char	cp_unit;	/* Done bit & unit # */
	char	cp_comm;	/* Command */
	short	cp_count;	/* Counter (when relevant) */
};

struct	cpdcb_o {		/* Output structure */
	struct	cphdr	cp_hdr;
	char	cp_buf[CPBUFLEN]; /* Buffer for output or 'stty' */
};

struct	cpdcb_i {		/* Structure for input */
	struct	cphdr	cp_hdr;
	char	cpi_buf[4]; 	/* Buffer for input */
};
#endif

#define	CPDONE	0x80		/* 'Done' bit in cp_unit */
#define	CPTAKE	0x40		/* CP 'ack' to this cpdcb */

/* unit values */
#define	CPUNIT	0		/* the CP itself */
#define	CPCONS	1		/* console line */
#define	CPREMOT	2		/* remote line */
#define	CPCLOCK	4		/* realtime clock */

/* commands */
#define	CPRESET 0
#define	CPWRITE 1		/* write device or register */
#define	CPREAD	2		/* read device or register */
#define	CPSTTY	3		/* set terminal configuration */
#define	CPBOOT	4		/* reboot system */

#if !defined(LOCORE) && defined(KERNEL)
struct	cphdr *cnlast;		/* last command sent to cp */
#endif
