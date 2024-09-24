/* Test software for gjrand random numbers version 4.0.1.0 or later. */
/* Copyright (C) 2004-2013 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"
#include "../../testcommon/binom.h"
#include "../../testcommon/chi2p.h"

/* "Kendall and Babington-Smith Gap Test" on 8 bit things assembled from */
/* 2 bit slices of 4 consecutive 32 bit words. */
/* This is most likely to be useful on generators which produce 32 bit words */
/* with some bits much worse quality than others. I am thinking mostly */
/* of linear congruential generators, but perhaps it will catch others too. */

static void
crash(const char *s)
{
	fprintf(stderr, "crash [%s]\n", s); exit(1);
}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

static int64_t dc[16][751];
static unsigned short dc16[16][751];
static short pos16[16][256];

static int sat=0;

static void
desat(void)
{
	int tot;
	int j, k, t;

	for (j=0; j<16; j++)
	{
		tot = 0;
		for (k = 0; k<751; k++)
		{
			t = dc16[j][k]; tot += t;
			dc[j][k] += t; dc16[j][k] = 0;
		}
		if (tot != sat)
		{
			printf(
"Overflow in counters. This is seriously non-random\n\nP = 0\n");
			exit(0);
		}
	}
	sat = 0;
}

static void
mkexpect(double expect[751], int64_t pos)
{
	double c=(double)pos, e=c/256.0;
	int j;

	for (j=1; j<751; j++) {expect[j]=e; c-=e; e *= 255.0/256.0;}
	expect[0]=c;
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double expect[751];
	double p[16], extreme;
	time_t tm;
	int i, j, exi, exj, exd;

	if (sat) desat();

	mkexpect(expect, n);

	for (j=0; j<16; j++)
	{
		int64_t *d= &(dc[j][0]);
		double e, x, t=0.0;
		for (i=0; i<751; i++) {e=expect[i]; x=(double)d[i]-e; t+=x*x/e;}
		printf("%2d : %15.5f\n", 2*j, t);
		p[j] = chi2p1(t, 749);
	}

	p[0]=pcombo(p, 16);

	extreme=999.0; exi= -1; exj= -1; exd='?';
	for (j=0; j<16; j++)
	{
		int64_t tot=n;
		for (i=1; i<750; i++)
		{
			int64_t k=dc[j][i];
			double x=sumbino(tot, k, 1.0/256.0);
			if (x<extreme)
			{
				extreme=x; exi=i; exj=j;
				if (k*256>tot) exd='+'; else exd='-';
			}
			tot-=k;
		}
	}
	printf("extreme = %.3g (%d %d %c)\n", extreme, 2*exj, exi, exd);
	p[1]=pco_scale(extreme, 16*749);

	printf("pvals (%.3g %.3g)\n", p[0], p[1]);

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		n*16.0, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(p, 2));
	if (!final) printf("------\n\n");
}

static void
dobuf(const uint32_t * const buf, int n)
{
	int i, j, k;
	const uint32_t * const bend=buf+4*n;

	for (k=0; k<16; k++)
	{
		short *pp = &(pos16[k][0]);

		for (i=255; i>=0; i--)
		{
			j = *pp - n; if (j< -20000) j= -20000;
			*pp++ = j;
		}
	}

	i= -n;
	do
	{
		uint32_t a=bend[4*i], b=bend[4*i+1], c=bend[4*i+2], d=bend[4*i+3];
		uint32_t part;

#define DOPART(k) \
		j=i-pos16[k][part]; \
		if (j>=751) j=0; \
		pos16[k][part]=i; \
		dc16[k][j]++;

#define DOMORE(k) \
		part = (a&0xc0) | (b&0x30); a >>= 2; \
		part |= c&0xc; b >>= 2; \
		part |= d&0x3; c >>= 2; \
		DOPART(k); d >>= 2;

		part = ((a<<6)&0xc0) | ((b<<4)&0x30) | ((c<<2)&0xc) | (d&0x3);
		DOPART(0); d >>= 2;

		part = ((a<<4)&0xc0) | ((b<<2)&0x30) | (c&0xc) | (d&0x3);
		DOPART(1); c >>= 2; d >>= 2;

		part = ((a<<2)&0xc0) | (b&0x30) | (c&0xc) | (d&0x3);
		DOPART(2); b >>= 2; c >>= 2; d >>= 2;

		DOMORE(3);
		DOMORE(4); DOMORE(5); DOMORE(6); DOMORE(7);
		DOMORE(8); DOMORE(9); DOMORE(10); DOMORE(11);
		DOMORE(12); DOMORE(13); DOMORE(14); DOMORE(15);

		i++;
	}
	while (i<0);
}

static void
readstuff(int64_t n, int progress)
{
	static int64_t progsize[]=
	{
		100000000,
		150000000,
		200000000,
		300000000,
		500000000,
		700000000,
		0
	};
#define MYBUFSIZ 2048
	uint32_t buf[MYBUFSIZ];
	int64_t p=0, nextp=progsize[0];
	int j, progi=0;

	memset(dc, 0, sizeof(dc));
	memset(dc16, 0, sizeof(dc16));
	for (j=0; j<16; j++)
	{
		int i;
		for (i=0; i<256; i++) pos16[j][i] = -275;
	}

	while (n<=0 || p<n)
	{
		j=fread(buf, sizeof(uint32_t)*4, MYBUFSIZ/4, stdin);
		if (j<=0) break;
		if (n>0 && j>n-p) j=n-p;
		dobuf(buf, j);
		sat += j;
		if (sat>500000) desat();
		p+=j;
		if (progress && p*16>nextp)
		{
			doan(p, 0);
			progsize[progi++] *= 10;
			nextp = progsize[progi];
			if (!nextp) {progi = 0; nextp = progsize[0];}
		}
	}

	if (n>0 && p<n)
	{
		fprintf(stderr,
			"Warning, expected %.0f 128 bit samples, saw only %.0f\n",
			(double)n, (double)p);
		seterr(1);
	}

	if (p<50000)
	{
		fprintf(stderr, "Warning, < 50000 samples, don't trust result.\n");
		seterr(1);
	}

	doan(p, 1);
}

int
main(int argc, char **argv)
{
	double dn;
	int64_t n= -1;
	int progress=0, j;

	tstart = time(0);

	for (j=1; j<argc; j++)
	{
		if (strcmp(argv[j], "--progress")==0) progress = 1;
		else if (sscanf(argv[j], "%lf", &dn)==1) n = (int64_t)(dn/16.0);
		else crash("optional arg must be --progress or numeric");
	}

	printf("expected range [ 633 662 699 799 841 875 ]\n");

	readstuff(n, progress);

	return errorlevel;
}
