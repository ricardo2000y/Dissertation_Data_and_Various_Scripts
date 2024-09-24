/* Miscellaneous stuff for gjrand random numbers version 3.2.2.0 or later. */
/* Copyright (C) 2004-2010 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

/* This one writes data for use by the tests in testunif */

/* optional arg is random seed. */
/* This tries to write forever. */

/* This is (a very loose interpretation of) the Blum, Blum, Shub generator. */

/* THIS VERSION IS NOT DONE PROPERLY ! */
/* DO NOT USE IT FOR SECURITY RELATED PURPOSES ! */
/* The most obvious problem is that the modulus is far too small. */

/* A further problem is that it tries for extra speed */
/* by revealing more bits of state than it's supposed to. */

/* This is slow, but not horribly so. It's comparable with glibc rand() */
/* function on my machine, and probably faster than inversive congruential */
/* generators, which have a following for some reason. */

/* Quality looks ok on pmcp --big . */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "../../src/my_int_types.h"

#define MOD1 (4000000559ul)
#define MOD2 (4200000419ul)

static uint32_t state[2];

static void
bbsseed(uint32_t s)
{
	state[0] = s%(MOD1-3) + 2;
	state[1] = ((uint64_t)s)*1234567ul%(MOD2-3) + 2;
}

static void
fill(uint32_t *buf)
{
	int i;
	uint32_t a=state[0], b=state[1];

	buf+=2048;

	for (i= -2048; i<0; i++)
	{
		a=(uint64_t)a*a%MOD1;
		b=(uint64_t)b*b%MOD2;
		/* The next line is totally bogus. */
		buf[i]=b*987653421UL+a;
	}

	state[0]=a; state[1]=b;
}

static void
blat(uint32_t seed)
{
	uint32_t buf[2048];

	bbsseed(seed);

	while (1)
	{
		fill(buf);
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
