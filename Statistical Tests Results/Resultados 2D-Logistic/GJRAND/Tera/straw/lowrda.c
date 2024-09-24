/* Miscellaneous stuff for gjrand random numbers version 3.3.2.0 or later. */
/* Copyright (C) 2004-2010 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../src/my_int_types.h"

/* This one writes data for use by the tests in testunif */

/* optional arg is random seed. */
/* This attempts to write forever. */

/* This one is an experiment to see if i can make it fail the rda test */
/* on the low side, preferably while doing fairly well on other tests. */
/* The thing is that bad generators usually fail rda on the high side. */
/* I can't think why this would be important theoretically or practically, */
/* I was just wondering. */

#define MAX 4101
static double expect[MAX];
static double count[MAX];
static int64_t last[256];

static void
init(void)
{
	double c, e;
	int i;

	for (i=0; i<MAX; i++) count[i]=0;
	for (i=0; i<256; i++) last[i]= -275;
	c=1.0; e=1.0/256.0;
	for (i=1; i<MAX; i++) {expect[i]=e; c-=e; e *= 255.0/256.0;}
	expect[0]=c;
}

static int64_t pos=0;
static uint64_t seed;

static void
fillbuf(unsigned char *buf)
{
	int i;

	for (i=0; i<4096; i++)
	{
		int j, j2, k, k2;

		seed = seed*30987653421U+1;
		j = seed>>56; j2 = (seed>>48)&255;

		pos++;
		k=pos-last[j]; if (k>=MAX) k=0;
		k2=pos-last[j2]; if (k2>=MAX) k2=0;

		if (count[k]*expect[k2] > count[k2]*expect[k])
			{j = j2; k = k2;}

		buf[i] = j;
		last[j]=pos;
		count[k]++;
	}
}

static void
blat(void)
{
	unsigned char buf[4096];

	init();

	while (1)
	{
		fillbuf(buf);
		if (fwrite(buf, 1, 4096, stdout)!=4096) break;
	}
}

int
main(int argc, char **argv)
{
	unsigned long s;

	if (argc<=1 || sscanf(argv[1], "%lu", &s)!=1) s = time(0);
	seed = s;
	seed = seed*30987653421U+1;
	seed = seed*30987653421U+1;

	blat();

	return 0;
}
