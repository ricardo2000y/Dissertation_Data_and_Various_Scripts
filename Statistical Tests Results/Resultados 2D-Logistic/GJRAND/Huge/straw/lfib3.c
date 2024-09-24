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

/* This is a lagged Fibonacci generator, or additive feedback shift register. */

/* A trinomial with fairly large lags is used. This particular one was */
/* apparently recommended by Knuth, but i haven't seen the context. */
/* It's a primitive polynomial, according to Schneier. */

/* This fails mcp --small , failing rda16 and rda. */

/* Unrolled loop in update assumes (SIZ-LAG1) is divisible by 4. */
#define SIZ 2048
#define LAG1 100
#define LAG2 37

static void
update(uint32_t x[SIZ])
{
	uint32_t *xi, *end;

	/* unroll the first two loops too, if you're desperate, but it's */
	/* maybe 10% on most machines. */

	xi=x; end=xi+LAG2;
	do {*xi = xi[SIZ-LAG2] + xi[SIZ-LAG1]; xi++;} while (xi<end);

	end=x+LAG1;
	do {*xi = xi[-LAG2] + xi[SIZ-LAG1]; xi++;} while (xi<end);

	end=x+SIZ;
	do
	{
		uint32_t a, b, c, d;
		a=xi[-LAG2]; b=xi[1-LAG2]; c=xi[2-LAG2]; d=xi[3-LAG2];
		a+=xi[-LAG1]; b+=xi[1-LAG1]; c+=xi[2-LAG1]; d+=xi[3-LAG1];
		xi+=4;
		xi[-4]=a; xi[-3]=b; xi[-2]=c; xi[-1]=d;
	} while (xi<end);
}

static void
writestuff(uint32_t seed)
{
	int j;
	uint32_t buf[SIZ];

	for (j=SIZ-1022; j<SIZ; j++)
	{
		seed += 123456789;
		seed *= 987654321;
		seed ^= (seed>>16);
		buf[j] = seed;
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
