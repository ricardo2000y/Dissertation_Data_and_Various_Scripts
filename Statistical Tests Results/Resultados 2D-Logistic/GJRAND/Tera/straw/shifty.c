/* Miscellaneous stuff for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../src/my_int_types.h"

/* A small linear feadback shift register with some massaging of the output. */

/* The polynomial is x**57 + x**32 + 1 . It is irreducable and therefore has */
/* a fixed period 20587884010836553 (according to Carlo Wood), but it is not */
/* primitive. There are apparently 7 different cycles of this length, plus */
/* the trivial cycle of length 1. */

/* It fails mcp --small (lownda). */

/* Versions 3.4.0.0 and earlier had a stronger version of this generator. */

static uint32_t state[2];

static uint32_t
xrand(void)
{
	uint32_t d = state[0], a=state[1], b, c;

	state[0] = a;
	b = (d<<7) ^ (a>>25) ^ a;
	state[1] = b;

	c = a+b; d = 2*a+b+1987654321;
	c *= 9; d = (d<<20)|(d>>12);

	return c+d;
}

static void
fillxrand(uint32_t *buf)
{
	int j = -2048;

	buf+=2048;
	do {buf[j++] = xrand();} while (j<0);
}

static void
blat(uint32_t seed)
{
	uint32_t buf[2048];
	int i;

	state[0] = 0xffffffff;
	state[1] = seed;
	for (i=99; i>=0; i--) (void)xrand();

	while (1)
	{
		fillxrand(buf);
		if (fwrite(buf, 4, 2048, stdout)!=2048) break;
	}
}

int
main(int argc, char **argv)
{
	unsigned long seed;

	if (argc>1) sscanf(argv[1], "%lu", &seed);
	else seed = time(0);

	blat(seed);

	return 0;
}
