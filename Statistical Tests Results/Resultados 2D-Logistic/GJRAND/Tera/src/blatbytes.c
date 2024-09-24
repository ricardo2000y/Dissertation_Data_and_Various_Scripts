/* Test software for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include "../../src/gjrand.h"

/* This one writes a file for use by several tests in this directory. */

/* if first arg is -u , use an unaligned data block. */
/* next optional arg is block size in bytes */
/* next optional arg is required file size in bytes (negative for no limit) */
/* next optional arg is random seed. */

void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

void
blat(struct gjrand *g, unsigned char *bp, unsigned bsize, int64_t s)
{

	while (s)
	{
		int c;

		gjrand_randbytes(g, bsize, bp);
		if ((uint64_t)s>(uint64_t)bsize) c=bsize; else c=s;
		if (fwrite(bp, 1, c, stdout)!=(size_t)c)
			{if (s>=0) crash("fwrite fails"); break;}
		if (s>=0) s-=c;
	}
}

int
main(int argc, char **argv)
{
	static unsigned char buf[8192];
	unsigned char *bp=buf;
	unsigned bsize;
	unsigned long seed;
	double ds;
	int64_t s= -1;
	struct gjrand g;

	if (argc>1 && argv[1][0]=='-' && argv[1][1]=='u')
		{bp++; argv++; argc--;}
	if (argc>3) {sscanf(argv[3], "%lu", &seed); gjrand_init(&g, seed);}
	else gjrand_initrand(&g);
	if (argc>2 && sscanf(argv[2], "%lf", &ds)) s = (int64_t)ds;
	if (argc<=1 || sscanf(argv[1], "%u", &bsize)!=1) bsize = 512;

	if (bsize>8192) crash("block size too big (max 8192)");

	blat(&g, bp, bsize, s);

	return 0;
}
