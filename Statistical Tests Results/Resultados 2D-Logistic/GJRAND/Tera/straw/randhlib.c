/* Miscellaneous stuff for gjrand random numbers version 4.0.1.0 or later. */
/* Copyright (C) 2004-2013 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../src/my_int_types.h"

/* This one writes data for use by the tests in testunif */

/* optional arg is random seed. */
/* This attempts to write forever. */

/* This one is based on the most significant 8 bits of the rand() */
/* library function supplied with your system. Some older rand() */
/* gave only 15 bits result, so 8 bits is the most that can conveniently */
/* be used here. Most recent rand() functions give 31 bits. */
/* In some old rand() implementations, the least significant bits are */
/* extremely bad. */

/* Quality will vary enormously depending on your C library. */

static void
blat(void)
{
	char buf[4096];

	while (1)
	{
		int j=4095;
		do {buf[j--] = rand()*(256/(RAND_MAX+1.0));} while (j>=0);
		if (fwrite(buf, 1, 4096, stdout)!=4096) break;
	}
}

int
main(int argc, char **argv)
{
	unsigned int seed;

	if (argc>1) sscanf(argv[1], "%u", &seed);
	else seed = time(0);

	srand(seed);

	blat();

	return 0;
}
