#	@(#)Makefile	8.1 (Berkeley) 07/19/93

CFLAGS+=-DKERBEROS
PROG=	login
SRCS=	login.c
#SRCS=	klogin.c login.c
#DPADD=	${LIBUTIL} ${LIBKRB} ${LIBDES}
#LDADD=	-lutil -lkrb -ldes
DPADD=	${LIBUTIL}
LDADD=	-lutil
BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg

.include <bsd.prog.mk>
