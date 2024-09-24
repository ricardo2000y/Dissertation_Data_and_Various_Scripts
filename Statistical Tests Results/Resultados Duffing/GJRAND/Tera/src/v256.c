/* Test software for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"
#include "../../testcommon/chi2p.h"

/* Each test generates a block of 256 bytes and counts how many distinct */
/* byte values exist in it. */
/* This is repeated for each block and the counts accumulate. */
/* When finished, do chi-squared test, and print extrema. */

/* Only whole blocks are tested. If a partial block remains at the end, */
/* it is ignored. */

/* This is Geronimo's invention dating from September 2005. */
/* If this is actually an older known method, and especially if it has */
/* a name, i would like to hear. */

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

#define LO 145
#define HI 178

static int64_t count[257];

static void
mkexpect(double expect[257], int64_t pos)
{
	double t[257];
	int i, j;

	for (i=0; i<257; i++) expect[i]=0.0;
	expect[1]=(double)(pos>>8);

	for (i=2; i<=256; i++)
	{
		for (j=0; j<257; j++) t[j]=expect[j];
		for (j=0; j<257; j++) expect[j]=0.0;

		for (j=1; j<i; j++)
		{
			expect[j]+=t[j]*((double)j/256.0);
			expect[j+1]+=t[j]*((double)(256-j)/256.0);
		}
	}
}

static void
compact(double expect[], int64_t tc[])
{
	int64_t ag;
	double xag;
	int i;

	ag=0; xag=0.0;
	for (i=0; i<=LO; i++) {ag+=count[i]; xag+=expect[i];}
	tc[LO]=ag; expect[LO]=xag;
	for (i=0; i<LO; i++) {tc[i]=0; expect[i]=0.0;}

	ag=0; xag=0.0;
	for (i=256; i>=HI; i--) {ag+=count[i]; xag+=expect[i];}
	tc[HI]=ag; expect[HI]=xag;
	for (i=256; i>HI; i--) {tc[i]=0; expect[i]=0.0;}

	for (i=LO+1; i<HI; i++) tc[i] = count[i];
}

static double
oneside(double x, double y)
{
	if (x<0.5) {if (y>0.5) y = 0.5; return 2.0*y;}
	return 2.0*(1.0-x);
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double p[3];
	double expect[257];
	int64_t tc[257];
	double e, t=0.0, x, y;
	time_t tm;
	int j, m;

	mkexpect(expect, n);

	m=256;
	for (j=255; j>0; j--) if (count[j]) m=j;
	x=0.0;
	for (j=1; j<m; j++) x+=expm1(-expect[j])*(x-1.0);
	y=expm1(-expect[m])*(x-1.0)+x;
	printf("minimum = %d  (prob = %15.10f %15.10f)\n", m, x, y);

	p[0]=oneside(x, y);

	m=1;
	for (j=2; j<=256; j++) if (count[j]) m=j;
	x=0.0;
	for (j=256; j>m; j--) x+=expm1(-expect[j])*(x-1.0);
	y=expm1(-expect[m])*(x-1.0)+x;
	printf("maximum = %d  (prob = %15.10f %15.10f)\n\n", m, x, y);

	p[1]=oneside(x, y);

	compact(expect, tc);

	for (j=LO; j<=HI; j++)
	{
		e=expect[j];
		x=(double)(tc[j])-e;
		t+=x*x/(e+0.0001);
	}

	printf("expected range [ 14.0 17.8 23.9 44.9 56.1 65.3 ]\n"
		"chi-squared = %15.5f\n", t);

	p[2] = chi2p1(t, HI-LO);

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		(double)n, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(p, 3));
	if (!final) printf("------\n\n");
}

static int
dobuf(const unsigned char * const buf)
{
	uint64_t w[32], s;
	const unsigned char *j;
	static unsigned char *c;
	int k, k1, k2, k3;

	c = (unsigned char *)w;
	memset(c, 0, sizeof(w));

	j=buf+252;
	do
	{
		k = j[0]; k1 = j[1]; k2 = j[2]; k3 = j[3];
		c[k] = 1; c[k1] = 1; c[k2] = 1; c[k3] = 1;
		j-=4;
	} while (j>=buf);

	s = 0;
	for (k=31; k>=0; k--) s += w[k];
	s = (s>>32) + (s&0xffffffff);
	s = ((s>>8)&0xff00ff) + (s&0xff00ff);
	s = ((s>>16)&0x3ff) + (s&0x3ff);

	return (int)s;
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
#define MYBUFSIZ 8192
	unsigned char buf[MYBUFSIZ];
	int64_t p=0, nextp=progsize[0];
	int j, progi=0;

	memset(count, 0, sizeof(count));

	while (n<0 || p<n)
	{
		j = fread(buf, 1, MYBUFSIZ, stdin);
		if (j<=0) break;
		if (n>0 && j>n-p) j = n-p;
		j &= (~255);
		p+=j;
		do {j-=256; count[dobuf(buf+j)]++;}
		while (j>0);
		if (progress && p>nextp)
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

	if (p<1000000)
	{
		fprintf(stderr,
			"Warning, < 1000000 samples, don't trust result.\n");
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
		else if (sscanf(argv[j], "%lf", &dn)==1) n = (int64_t)dn;
		else crash("optional arg must be --progress or numeric");
	}

	if (n>0) n &= ~((int64_t)255);

	readstuff(n, progress);

	return errorlevel;
}
