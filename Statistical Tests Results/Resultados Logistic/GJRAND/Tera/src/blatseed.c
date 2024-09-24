/* Test software for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../../src/gjrand.h"

/* This one writes a file for use by several tests in this directory. */

/* first optional arg is required file size in bytes (negative for no limit) */
/* second optional arg is random seed, */
/*	or "a" to repeatedly use gjrand_initrand() for autoseeding. */
/*	or "sNNNN" where NNNN is a number to use with gjrand_init() */
/*		and a 32 bit seed. */

/* This one is inspired by a comment on Donald Knuth's website. */
/* Knuth modified the seeding procedure for one of his generators after */
/* Pedro Gimeno suggested that in practice, random number generators are */
/* often used in this bizarre mode: */
/* Seed the generator, produce a few (or just one) random numbers; */
/* reseed the generator, often with previous seed + 1, and loop. */
/* Furthermore, a lot of generators apparently behave badly in this mode. */
/* Richard Brent contributed some further analysis of this problem. */

/* Needless to say, this comes as a shock to designers of random number */
/* generators because it is not at all the usage mode we are usually */
/* designing for. However, in retrospect, it seems obvious that a lot */
/* of applications will actually use this mode. Therefore design and */
/* testing should take it into account. */

/* With the "s" option, this should repeat after 32GB, because there */
/* are only 2 ** 32 different seeds possible with gjrand_init(). */

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static uint64_t buf[1024];

static void
blata(struct gjrand *g, int64_t s)
{
	while (s)
	{
		int c;
		int i = -1024;
		do
		{
			gjrand_initrand(g);
			buf[1024+i]=gjrand_rand64(g);
			i++;
		} while (i<0);

		if ((uint64_t)s>8192) c=8192; else c=s;
		if (fwrite(buf, 1, c, stdout)!=(size_t)c)
			{if (s>=0) crash("fwrite fails"); break;}

		if (s>=0) s-=c;
	}
}

static void
blat(struct gjrand *g, int64_t s, uint64_t seed)
{
	while (s)
	{
		int c, j= -1024;
		do
		{
			seed++;
			gjrand_init64(g, seed);
			buf[1024+j]=gjrand_rand64(g);
			j++;
		} while (j<0);

		if ((uint64_t)s>8192) c=8192; else c=s;
		if (fwrite(buf, 1, c, stdout)!=(size_t)c)
			{if (s>=0) crash("fwrite fails"); break;}
		if (s>=0) s-=c;
	}
}

static void
blatshort(struct gjrand *g, int64_t s, uint32_t seed)
{
	while (s)
	{
		int c, j= -1024;
		do
		{
			seed++;
			gjrand_init(g, seed);
			buf[1024+j]=gjrand_rand64(g);
			j++;
		} while (j<0);

		if ((uint64_t)s>8192) c=8192; else c=s;
		if (fwrite(buf, 1, c, stdout)!=(size_t)c)
			{if (s>=0) crash("fwrite fails"); break;}

		if (s>=0) s-=c;
	}
}

int
main(int argc, char **argv)
{
	int autoseed=0, shortseed=0;
	double ds;
	int64_t s= -1;
	unsigned long seed;
	struct gjrand g;

	if (argc>2)
	{
		if (argv[2][0]=='a') autoseed=1;
		else if (argv[2][0]=='s')
			{shortseed=1; sscanf(argv[2]+1, "%lu", &seed);}
		else sscanf(argv[2], "%lu", &seed);
	}
	else {gjrand_initrand(&g); seed=gjrand_rand32(&g);}

	if (argc>1 && sscanf(argv[1], "%lf", &ds)) s = (int64_t)ds;

	if (autoseed) blata(&g, s);
	else if (shortseed) blatshort(&g, s, (uint32_t)seed);
	else blat(&g, s, ((uint64_t)seed)<<32);

	return 0;
}
