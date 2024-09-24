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

/* "Kendall and Babington-Smith Gap Test" on byte values. */

/* For each byte read, calculate the distance since the same byte */
/* value was last seen and collect a histogram of these distances. */
/* Finally do a chi-square test on the histogram. */

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

static int64_t dc[752][2];
static int pos32[256];

static double expzer[752];

static void
mkexpzer(double fracz)
{
	double p, pi;
	int j;

	expzer[0] = expzer[1] = 0.0;
	pi = fracz; p = 0.0;
	fracz = 1.0-fracz;

	for (j=2; j<752; j++) {p += pi; pi *= fracz; expzer[j] = p;}
};

static double expect[752];

static void
mkexpect(double expect[752], int64_t pos)
{
	double c=(double)pos, e=c/256.0;
	int j;

	for (j=1; j<751; j++) {expect[j]=e; c-=e; e *= 255.0/256.0;}
	expect[751]=c;
}


static void
doan1(int64_t n, double pvs[3])
{
	double e, t=0.0, x, extreme;
	int64_t tot, ext=0, exk=0;
	int j, exi, exd;

	for (j=1; j<752; j++)
	{
		e=expect[j];
		x=(double)dc[j][0]-e;
		t+=x*x/e;
	}

	pvs[0] = chi2p1(t, 749);

	printf("chisquare (df=749)  %.0f (p = %.3g)\n", t, pvs[0]);

	extreme=999.0; exi= -1; exd='?';
	tot=n;
	for (j=1; j<751; j++)
	{
		int64_t k=dc[j][0];
		if (tot<=0) tot = 1;
		x=sumbino(tot, k, 1.0/256.0);
		if (x<extreme)
		{
			extreme=x; exi=j;
			if (k*256>tot) exd='+'; else exd='-';
			if (extreme<=0.0) break;
		}
		tot-=k;
	}
	pvs[1]=pco_scale(extreme, 750);
	printf("extreme = %.3g (%d %c) (p = %.3g)\n",
		extreme, exi, exd, pvs[1]);

	extreme=999.0; exi= -1; exd='?';
	for (j=2; j<751; j++)
	{
		int64_t k=dc[j][1];
		tot = dc[j][0];
		if (tot<=0) tot = 1;
		x=sumbino(tot, k, expzer[j]);
		if (x<extreme)
		{
			extreme=x; exi=j;
			if (k>tot*expzer[j]) exd='+'; else exd='-';
			exk = k; ext = tot;
			if (extreme<=0.0) break;
		}
		tot-=k;
	}
	pvs[2]=pco_scale(extreme, 749);
	printf("zero1st = %.3g (%d %c) (p = %.3g)\n",
		extreme, exi, exd, pvs[2]);
	printf("ext = %.0f ; exk = %.0f ; expect = %.0f\n\n",
		(double)ext, (double)exk, ext*expzer[exi]);
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double pvs[3];
	int64_t nz;
	time_t tm;
	int j;

	nz = 0;
	for (j=751; j>=0; j--) nz += dc[j][0];

	mkexpect(expect, nz);
	mkexpzer(1.0/255.0);

	doan1(nz, pvs);

	tm = time(0);
	printf("processed %.2g bytes in %.3g seconds. %s\n",
		(double)n, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(pvs, 3));
	if (!final) printf("------\n\n");
}

static void
dobuf(const unsigned char *buf, int n)
{
	int i, j, ch, k, pz;
	const unsigned char * const bend=buf+n;

	for (k=255; k>=0; k--)
		{j = pos32[k]-n; if (j < -20000) j = -20000; pos32[k] = j;}

	pz = pos32[0];

	i = -n;
	do
	{
		ch = bend[i];

		if (ch)
		{
			j = i-pos32[ch]; if (j>751) j = 751;
			dc[j][0]++; if (pos32[ch]<pz) dc[j][1]++;
		}
		else pz = i;

		pos32[ch] = i;
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
#define MYBUFSIZ 8192
	unsigned char buf[MYBUFSIZ];
	int64_t p=0, nextp=progsize[0];
	int j, progi=0;

	for (j=0; j<256; j++) pos32[j]= -9-j;
	memset(dc, 0, sizeof(dc));

	while (n<=0 || p<n)
	{
		j=fread(buf, 1, MYBUFSIZ, stdin);
		if (j<=0) break;
		if (n>0 && j>n-p) j = n-p;
		dobuf(buf, j);
		p += j;
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
			"Warning, expected %.0f bytes, saw only %.0f\n",
			(double)n, (double)p);
		seterr(1);
	}

	if (p<500000)
	{
		fprintf(stderr,
			"Warning, < 500000 bytes, don't trust result.\n");
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

	readstuff(n, progress);

	return errorlevel;
}
