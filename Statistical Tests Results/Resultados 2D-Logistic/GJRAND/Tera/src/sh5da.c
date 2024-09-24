/* Test software for gjrand random numbers version 4.1.1.0 or later. */
/* Copyright (C) 2004-2013 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"
#include "../../testcommon/binom.h"
#include "../../testcommon/chi2p.h"

/* Take 5 32 bit words at a time and look at their ordering */
/* There are 5! possibilities. Treat results as a stream of data 0-119 */
/* and do a Kendall and Babington-Smith Gap Test on this stream. */

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

#define FAC (5*4*3*2)
static int32_t pos32f[FAC], pos32r[FAC];

#define MAXGAP 366
static int64_t dcf[MAXGAP], dcr[MAXGAP];

static void
mkexpect(int64_t pos, double expect[MAXGAP])
{
	double x, y;
	int j;

	x = pos;
	for (j=1; j<MAXGAP; j++) {y = x*(1.0/FAC); expect[j] = y; x -= y;}
	expect[0] = x;
}

static double expect[MAXGAP];

static void
doan1(int64_t n, int64_t dc[MAXGAP], double pvs[2])
{
	double t=0.0, x, extreme;
	int64_t tot;
	int j, exj, exd;

	for (j=0; j<MAXGAP; j++)
	{
		double e=expect[j];
		double x=((double)(dc[j]))-e;
		t+=x*x/e;
	}

	pvs[0] = chi2p1(t, MAXGAP-2);

	printf(" (df=%d) chis=%.2f  p = %.3g\n", MAXGAP-2, t, pvs[0]);

	extreme = 999.0; exj = -1; exd = '?';
	tot = n;
	for (j=1; j<MAXGAP; j++)
	{
		int64_t k=dc[j];
		if (tot<=0) tot = 1;
		x=sumbino(tot, k, 1.0/FAC);
		if (x<extreme)
		{
			extreme=x; exj=j;
			if (k*FAC>tot) exd='+'; else exd='-';
			if (extreme<=0.0) break;
		}
		tot-=k;
	}

	pvs[1] = pco_scale(extreme, MAXGAP-1);

	printf("extreme = %.3g (%d %c) (p = %.3g)\n", extreme, exj, exd, pvs[1]);
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double pvs[4];
	time_t tm;

	mkexpect(n, expect);

	printf("native byte order :");
	doan1(n, dcf, pvs);
	printf("reverse byte order:");
	doan1(n, dcr, pvs+2);

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		n*20.0, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(pvs, 4));
	if (!final) printf("------\n\n");
}

static int
order(const uint32_t b[5])
{
	uint32_t x;
	unsigned r=b[1]<b[0];

	r*=3;
	x=b[2]; r+=x<b[1]; r+=x<b[0];
	r*=4;
	x=b[3]; r+=x<b[2]; r+=x<b[1]; r+=x<b[0];
	r*=5;
	x=b[4]; r+=x<b[3]; r+=x<b[2]; r+=x<b[1]; r+=x<b[0];

	return r;
}

static void
dobuf(const uint32_t * const buf, int n, int32_t pos32[FAC], int64_t dc[MAXGAP])
{
	const uint32_t *b=buf;
	int i, j, ch;

	for (i=0; i<FAC; i++)
	{
		j=pos32[i]-n;
		if (j< -20000) j= -20000;
		pos32[i]=j;
	}

	i= -n;
	do
	{
		ch = order(b);
		b += 5;
		j = i-pos32[ch]; if (j>=MAXGAP) j = 0;
		pos32[ch] = i;
		dc[j]++;
		i++;
	}
	while (i<0);
}

static void
dobyteswap(uint32_t *b, int n)
{
	uint32_t x, y;

	if (n&1)
	{
		x= *b;
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		x = (x<<16)|(x>>16);
		*b = x; b++; n--;
	}
	n >>= 1;
	if (!n) return;
	do
	{
		x = *b; y = b[1];
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		y = ((y&0xff00ff)<<8) | ((y>>8)&0xff00ff);
		x = (x<<16)|(x>>16); y = (y<<16)|(y>>16);
		*b = x; b[1] = y; b += 2; n--;
	} while (n>0);
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
#define MYBUFBLOCKS (1024)
	uint32_t buf[MYBUFBLOCKS*5];
	int64_t p=0, nextp=progsize[0];
	int j, progi=0;

	memset(dcf, 0, sizeof(dcf));
	memset(dcr, 0, sizeof(dcr));
	for (j=0; j<FAC; j++) pos32f[j] = pos32r[j] = -76;

	while (n<=0 || p<n)
	{
		j=fread(buf, sizeof(uint32_t)*5, MYBUFBLOCKS, stdin);
		if (n>=0 && j>n-p) j=n-p;
		if (j<=0) break;
		dobuf(buf, j, pos32f, dcf);
		dobyteswap(buf, 5*j);
		dobuf(buf, j, pos32r, dcr);
		p += j;
		if (progress && p*20>nextp)
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
			"Warning, expected %.0f samples, saw only %.0f\n",
			(double)n, (double)p);
		seterr(1);
	}

	if (p<25000)
	{
		fprintf(stderr,
			"Warning, < 25000 samples, don't trust result.\n");
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
		else if (sscanf(argv[j], "%lf", &dn)==1)
			{if (dn>0) n = (int64_t)(dn*(1.0/20.0));}
		else crash("optional arg must be --progress or numeric");
	}

	readstuff(n, progress);

	return errorlevel;
}
