/* Test software for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include "../../src/gjrand.h"

/* This one writes a file for use by several tests in this directory. */

/* first optional arg is required file size in bytes (negative for no limit) */
/* second optional arg is random seed. */

void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

void
blat(struct gjrand *g, int64_t s)
{
	uint64_t buf[1024];

	while (s)
	{
		int c;
		int i;

		for (i=0; i<1024; i++) buf[i]=gjrand_rand64(g);

		if ((uint64_t)s>8192) c=8192; else c=s;
		if (fwrite(buf, 1, c, stdout)!=(size_t)c)
			{if (s>=0) crash("fwrite fails"); break;}

		if (s>=0) s-=c;
	}
}

int
main(int argc, char **argv)
{
	unsigned long seed;
	double ds;
	int64_t s= -1;
	struct gjrand g;

	if (argc>2) {sscanf(argv[2], "%lu", &seed); gjrand_init(&g, seed);}
	else gjrand_initrand(&g);
	if (argc>1 && sscanf(argv[1], "%lf", &ds)) s = (int64_t)ds;

	blat(&g, s);

	return 0;
}
