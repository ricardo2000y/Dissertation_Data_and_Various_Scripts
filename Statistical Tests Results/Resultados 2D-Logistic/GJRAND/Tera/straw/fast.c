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

/* An old attempt of mine at a very fast very good generator. */
/* Unfortunately by modern standards it's neither. */
/* sh5da becomes suspicious somewhere near 10GB. */

static uint32_t buf[1026];

static void
doseed(uint32_t seed)
{
	uint32_t s=seed;
	int i;

	buf[1025]=987654321;
	for (i=1024; i>=0; i--)
	{
		buf[i]=s;
		s=s*987653421+1;
	}
}

static void
dobuffirst(void)
{
	uint32_t a;

	a=buf[1024]+buf[1025];
	if (a==0) buf[1025]+=1234567890;
	buf[1024]=a;
	buf[0]^=a;
}

static void
dobuf(void)
{
	uint32_t a, b, c, d, *p;

	a=buf[1020]; b=buf[1021]; c=buf[1022]; d=buf[1023];

	p=buf;

	do
	{
		b=(b>>3)|(b<<29); a^=p[0]; d+=c;
		b^=p[1]; a+=d;
		d=(d>>7)|(d<<25); c^=p[2]; b+=a;
		d^=p[3]; c+=b;
		p[0]=a; p[1]=b; p[2]=c; p[3]=d;

		b=(b>>3)|(b<<29); a^=p[4]; d+=c;
		b^=p[5]; a+=d;
		d=(d>>7)|(d<<25); c^=p[6]; b+=a;
		d^=p[7]; c+=b;
		p[4]=a; p[5]=b; p[6]=c; p[7]=d;

		b=(b>>3)|(b<<29); a^=p[8]; d+=c;
		b^=p[9]; a+=d;
		d=(d>>7)|(d<<25); c^=p[10]; b+=a;
		d^=p[11]; c+=b;
		p[8]=a; p[9]=b; p[10]=c; p[11]=d;

		b=(b>>3)|(b<<29); a^=p[12]; d+=c;
		b^=p[13]; a+=d;
		d=(d>>7)|(d<<25); c^=p[14]; b+=a;
		d^=p[15]; c+=b;

		p+=16;
		p[-4]=a; p[-3]=b; p[-2]=c; p[-1]=d;
	} while (p<buf+1024);
}

static void
blat(uint32_t seed)
{
	doseed(seed);

	while (1)
	{
		dobuffirst();
		dobuf();
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
