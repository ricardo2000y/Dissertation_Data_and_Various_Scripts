/* Test software for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include "../../src/gjrand.h"

/* This one writes a file for use by several tests in this directory. */

/* first optional arg is block size in 64 bit words. */
/* second optional arg is required file size in bytes (negative for no limit) */
/* third optional arg is random seed. */

void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

void
blat(struct gjrand *g, unsigned bsize, int64_t s)
{
	int b8=8*bsize;
	uint64_t buf[4096];

	while (s)
	{
		int c;

		gjrand_rand64v(g, bsize, buf);

		if ((uint64_t)s>(uint64_t)b8) c=b8; else c=s;
		if (fwrite(buf, 1, c, stdout)!=(size_t)c)
			{if (s>=0) crash("fwrite fails"); break;}

		if (s>=0) s-=c;
	}
}

int
main(int argc, char **argv)
{
	unsigned bsize;
	unsigned long seed;
	double ds;
	int64_t s= -1;
	struct gjrand g;

	if (argc>3) {sscanf(argv[3], "%lu", &seed); gjrand_init(&g, seed);}
	else gjrand_initrand(&g);
	if (argc>2 && sscanf(argv[2], "%lf", &ds)) s = (int64_t)ds;
	if (argc<=1 || sscanf(argv[1], "%u", &bsize)!=1) bsize = 512;

	if (bsize>4096) crash("block size too big (max 4096)");

	blat(&g, bsize, s);

	return 0;
}
