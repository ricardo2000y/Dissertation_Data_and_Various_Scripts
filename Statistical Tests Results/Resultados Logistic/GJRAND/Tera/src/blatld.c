/* Test software for gjrand random numbers version 4.0.3.0 or later. */
/* Copyright (C) 2004-2013 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include "../../src/gjrand.h"

/* This one writes a file for use by several tests in this directory. */

/* first optional arg is required file size in bytes (negative for no limit) */
/* second optional arg is random seed. */

/* On some platforms, type long double has less than 64 bits of precision. */
/* If you have one of those, this program will generate a very non-random */
/* stream, and tests will fail horribly. Intel 387 and compatibles are ok, */
/* provided extended precision is used, as is the default on linux and */
/* probably MS-Windows. */

void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

#define B16 (65536.0)
#define B64 (B16*B16*B16*B16)

void
blat(struct gjrand *g, int64_t s)
{
	uint64_t buf[512];

	while (s)
	{
		int c, j;

		for (j=0; j<512; j++) buf[j] = (uint64_t)(gjrand_ldrand(g)*B64);
		if ((uint64_t)s>4096) c=4096; else c=s;
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
