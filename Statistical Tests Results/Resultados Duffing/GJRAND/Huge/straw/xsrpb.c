/* Miscellaneous stuff for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../src/my_int_types.h"

/* This one writes data for use by the tests in testunif */

/* optional arg is random seed. */
/* This attempts to write forever. */

/* This is one of R. P. Brent's variations on George Marsaglia's xorshift */
/* generators. Note that Brent does not recommend this one, preferring at */
/* least 256 bits. Also the "Weyl generator" is omitted. Without the Weyl, */
/* this is linear with respect to XOR. */

/* This fails mcp --tiny , failing the binr test because it is */
/* totally linear. At mcp --standard it also fails several others, so */
/* even if you don't care about linearity, this is bad. */
/* However, some of R. P. Brent's larger xorshift generators appear to be */
/* quite good, especially if used with the "Weyl generator" as recommended. */

static void
blat(uint32_t seed)
{
	uint32_t buf[2048], j=seed, k, t=123456789;

	while (1)
	{
		int i= -2048;
		do
		{
			k = t;
			j ^= j<<17; t ^= t<<12;
			j ^= j>>14; t ^= t>>19;
			t ^= j;
			(buf+2048)[i] = t;
			j = k;
			i++;
		} while (i<0);

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
