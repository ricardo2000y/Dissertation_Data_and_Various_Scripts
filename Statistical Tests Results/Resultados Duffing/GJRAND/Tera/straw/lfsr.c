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

/* Linear feadback shift register. */
/* Polynomial x**903 + x**640 + 1 is primitive according to Carlo Wood. */
/* Cycle should be (2 ** 903) - 1 bits. */

/* Quality is poor. I think there is a problem with slow diffusion here. */
/* Ie, if an unusual pattern of bits gets into the shift register, */
/* related unusual patterns are likely to be stuck in there through several */
/* thousand bits of output. Even larger shift registers would help to hide */
/* this effect, but wouldn't make it go away. Fixing it really needs */
/* some method of faster diffusion: eg. more tap points, or the "twist" and */
/* "temper" used in Mersenne Twister. */

/* This fails mcp --small . */

#define SIZ 2048

static void
update(uint32_t x[SIZ])
{
	int j;

	for (j=0; j<20; j++)
		x[j] = ((x[SIZ+j-29]>>25) | (x[SIZ+j-28]<<7)) ^ x[SIZ+j-20];

	for (j=20; j<28; j++)
		x[j] = ((x[SIZ+j-29]>>25) | (x[SIZ+j-28]<<7)) ^ x[j-20];

	x[28] = ((x[SIZ+28-29]>>25) | (x[28-28]<<7)) ^ x[j-20];

	for (j=29; j<SIZ; j++) x[j] = ((x[j-29]>>25) | (x[j-28]<<7)) ^ x[j-20];
}

static void
writestuff(uint32_t seed)
{
	int i;
	uint32_t buf[SIZ];

	for (i=SIZ-1022; i<SIZ; i++)
	{
		seed+=123456789;
		seed*=987654321;
		seed^=(seed>>16);
		buf[i]=seed;
	}

	while (1)
	{
		update(buf);
		if (fwrite(buf, 4, SIZ, stdout)!=SIZ) break;
	}
}

int
main(int argc, char **argv)
{
	unsigned long seed;

	if (argc>1) sscanf(argv[1], "%lu", &seed);
	else seed = time(0);

	writestuff(seed);

	return 0;
}
