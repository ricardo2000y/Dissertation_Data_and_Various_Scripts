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

/* Multiply with carry. Introduced by Marsaglia. This might be the one */
/* in Numerical Recipes? */

/* This fails mcp --tiny , failing diff12. That indicates it has "lattice", */
/* just like a lcg, which it is really. Other than that it seems mostly ok, */
/* though i haven't done any really long test runs. */

/*
Multiplier a and modulus b are chosen so that both
a*b - 1 and (a*b - 1) / 2 are prime;
the period is then (a*b - 1) / 2
Initial value must not be zero.
*/

static void
blat(uint32_t seed)
{
	uint32_t buf[1024];
	uint64_t j=seed;

	j++;

	while (1)
	{
		int k=1023;
		do
		{
			j = (0xffffda61ul * (j&0xfffffffful)) + (j>>32);
			buf[k--] = j;
		} while (k>=0);

		if (fwrite(buf, 1, 4096, stdout)!=4096) break;
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
