/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)RESET.c 1.3 05/07/81";

#include "h00vars.h"
#include "h01errs.h"

RESET(filep, name, maxnamlen, datasize)

	register struct iorec	*filep;
	char			*name;
	long			maxnamlen;
	long			datasize;
{
	if (name == NULL && filep == INPUT && filep->fname[0] == '\0') {
		if (fseek(filep->fbuf, (long)0, 0)) {
			ERROR(ESEEK, filep->pfname);
			return;
		}
		filep->funit &= ~(EOFF | EOLN);
		filep->funit |= SYNC;
		return;
	}
	filep = GETNAME(filep, name, maxnamlen, datasize);
	filep->fbuf = fopen(filep->fname, "r");
	if (filep->fbuf == NULL) {
		if (filep->funit & TEMP) {
			filep->funit |= (EOFF | SYNC | FREAD);
			return;
		}
		ERROR(EOPEN, filep->pfname);
		return;
	}
	filep->funit |= (SYNC | FREAD);
	if (filep->fblk > PREDEF) {
		setbuf(filep->fbuf, &filep->buf[0]);
	}
}
