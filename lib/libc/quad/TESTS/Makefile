#	@(#)Makefile	8.1 (Berkeley) 06/04/93

all: mul divrem

MUL=	mul.c ../muldi3.c
mul: ${MUL}
	gcc -g -DSPARC_XXX ${MUL} -o $@

DIVREM=	divrem.c ../qdivrem.c
divrem: ${DIVREM}
	gcc -g -DSPARC_XXX ${DIVREM} -o $@
