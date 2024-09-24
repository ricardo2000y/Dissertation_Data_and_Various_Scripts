/* Miscellaneous stuff for gjrand random numbers version 3.3.2.0 or later. */
/* Copyright (C) 2004-2010 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../src/my_int_types.h"

/* This one writes data for use by the tests in testunif */

/* second optional arg is random seed. */
/* This attempts to write forever. */

/* This is a linear congruential generator. */
/* These tend to be very weak in the least significant bits */
/* and reasonably good in the more significant bits. */
/* This one returns the most significant 32 bits out of 64. */
/* This is much better than 32 bit or 48 bit lcgs which still */
/* get used far too often by unsuspecting programmers. */
/* Unfortunately, even a 64 bit lcg isn't good enough by modern standards. */

/* This fails mcp --small , failing diff12 and selfcor (reverse lsb). */

static void
blat(uint32_t seed)
{
	uint32_t buf[2048];
	uint64_t j=seed;

	while (1)
	{
		int i= -2048;
		do
		{
			j=j*3098765421UL+1;
			(buf+2048)[i]=j>>32;
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
