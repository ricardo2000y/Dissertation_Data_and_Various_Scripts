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

/* This is a lagged Fibonacci generator, or additive feedback shift register. */
/* This has 4 taps instead of the usual 2 (should be good), and a much */
/* smaller state than usual (should be bad). Result: very bad. */

/* This fails mcp --tiny , failing selfcor (reverse lsb) and slicerda. */

static void
update(uint32_t x[16])
{
	int i;

	for (i=0; i<16; i++) x[i] += x[(i-5)&15]+x[(i-3)&15]+x[(i-2)&15];
}

static void
blat(uint32_t seed)
{
	int i;
	uint32_t buf[16];

	buf[0]=seed;
	buf[1]= 0xffffffff;
	buf[2]=987654321;
	for (i=3; i<16; i++) buf[i]=buf[i-1]+123456789;
	update(buf);
	buf[0]^=buf[0]>>16;
	buf[1]^=buf[1]>>24;
	buf[2]^=buf[2]>>28;
	buf[3]^=buf[3]>>30;
	buf[4]^=buf[4]>>31;
	update(buf);

	while (1)
	{
		update(buf);
		if (fwrite(buf, 4, 16, stdout)!=16) break;
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
