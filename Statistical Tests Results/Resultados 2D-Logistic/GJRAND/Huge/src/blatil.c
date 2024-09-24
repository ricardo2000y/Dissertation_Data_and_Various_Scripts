/* Test software for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

/* This one writes a file for use by several tests in this directory. */

/* first optional arg is required file size in bytes (negative for no limit) */
/* second optional arg is random seed or Xn where n is 0 to 3. */
/* Xn uses gjrand_init4() and varies only element n of the seed array. */

/* 512 different generator objects are used and we output 32 bytes from */
/* each in turn. */

/* The idea of this one is to test independence of the streams generated */
/* by different seeds. This idea is inspired by a quick look through */
/* SPRNG and its tests. The SPRNG tests i think do a lot more than this. */
/* I saw version 2, latest is version 4 but i think the tests are the same. */
/* http://sprng.fsu.edu */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../../src/gjrand.h"

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static void
blat(struct gjrand g[512], int64_t s)
{
	uint64_t buf[2048];

	while (s)
	{
		int c, i=2044, j=511;
		do {gjrand_rand64v(g+j, 4, buf+i); j--; i-=4;} while (i>=0);
		if ((uint64_t)s>sizeof(buf)) c=sizeof(buf); else c=s;
		if (fwrite(buf, 1, c, stdout)!=(size_t)c)
			{if (s>=0) crash("fwrite fails"); break;}
		if (s>=0) s-=c;
	}
}

int
main(int argc, char **argv)
{
	struct gjrand g[512];
	unsigned long seed;
	double ds;
	int64_t s= -1;
	int i=0;

	gjrand_initrand(g);
	if (argc>2)
	{
		uint32_t s4[4];
		int j;
		if (argv[2][0] == 'X')
		{
			j = argv[2][1] - '0'; if (j<0 || j>3) j=0;
			gjrand_rand32v(g, 4, s4);
			for (i=0; i<512; i++) {gjrand_init4(g+i, s4); s4[j]++;}
		}
		else sscanf(argv[2], "%lu", &seed);
	}
	else seed = gjrand_rand32(g);
	if (i==0) for (i=0; i<512; i++) gjrand_init(g+i, seed+i);

	if (argc>1 && sscanf(argv[1], "%lf", &ds)) s = (int64_t)ds;

	blat(g, s);

	return 0;
}
