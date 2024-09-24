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

/* "Kendall and Babington-Smith Gap Test" on 16-bit values. */

/* For each word read, calculate the distance since the same word */
/* value was last seen and collect a histogram of these distances. */
/* Finally do a chi-square test on the histogram. */

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

static int64_t dc[2][70000][2];
static int pos32[65536];

static double expzer[70000];

static void
mkexpzer(double fracz)
{
	double p, pi;
	int j;

	expzer[0] = expzer[1] = 0.0;
	pi = fracz; p = 0.0;
	fracz = 1.0-fracz;

	for (j=2; j<70000; j++) {p += pi; pi *= fracz; expzer[j] = p;}
};

static double expect[70000];

static void
mkexpect(int64_t pos)
{
	double c=(double)pos, e=c/65536.0, f;
	int i;

	for (i=1; i<70000; i++)
	{
		f = e; if (f<0.5) f = 0.5;
		expect[i]=f;
		c-=e;
		e *= 65535.0/65536.0;
	}

	if (c<0.5) c = 0.5;
	expect[0]=c;
}


static void
doan1(const char *note, int64_t n, int j, double pvs[2])
{
	double e, t=0.0, x, extreme;
	int64_t tot, ext=0, exk=0;
	int i, exi, exd, tr;

	puts(note);

	if (j==0)
	{
		for (i=0; i<70000; i++)
			{e=expect[i]; x=(double)dc[j][i][0]-e; t+=x*x/e;}

		pvs[1] = chi2p1(t, 69999);

		printf("chis = %.0f (p = %.3g)\n", t, pvs[1]);

		extreme=999.0; exi= -1; exd='?';
		tot=n;
		for (i=1; i<70000; i++)
		{
			int64_t k=dc[j][i][0];
			if (tot<=4) break;
			x = sumbino(tot, k, 1.0/65536.0);
			if (x<extreme)
			{
				extreme=x; exi=i;
				if (k*65536>tot) exd='+'; else exd='-';
				if (extreme<=0.0) break;
			}
			tot-=k;
		}
		if (i>1) pvs[2]=pco_scale(extreme, i-1); else pvs[2] = 0.5;
		printf("extreme = %.3g (%d %c) (p = %.3g)\n",
			extreme, exi, exd, pvs[2]);
	}

	extreme=999.0; exi= -1; exd='?'; tr=0;
	for (i=2; i<70000; i++)
	{
		int64_t k=dc[j][i][1];
		tot = dc[j][i][0];
		if (tot>4)
		{
			x=sumbino(tot, k, expzer[i]);
			tr++;
			if (x<extreme)
			{
				extreme=x; exi=i;
				if (k>tot*expzer[i]) exd='+'; else exd='-';
				exk = k; ext = tot;
				if (extreme<=0.0) break;
			}
		}
	}
	if (tr) pvs[0]=pco_scale(extreme, tr); else pvs[0] = 0.5;
	printf("zero1st = %.3g (%d %c) (p = %.3g)\n", extreme, exi, exd, pvs[0]);
	printf("ext = %.0f ; exk = %.0f ; expect = %.3g\n\n",
		(double)ext, (double)exk, ext*expzer[exi]);
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double pvs[6];
	int64_t nz;
	time_t tm;
	int j;

	if (n>((int64_t)1)<<47) printf(
"rda16: with such huge amounts of data, counter overflow might happen\n"
"rda16: perhaps you should rebuild with 64 bit counters\n");

	nz = 0;
	for (j=69999; j>=0; j--) nz += dc[0][j][0];

	mkexpect(nz);
	mkexpzer(1.0/65535.0);

	doan1("simple", nz, 0, pvs);
	doan1("zero1st", nz, 1, pvs+3);

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		(double)(2*n), (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(pvs, 4));
	if (!final) printf("------\n\n");
}

static void
dobuf(const unsigned short *buf, int n)
{
	int i, j, ch, k, pz;
	const unsigned short * const bend=buf+n;

	for (k=65535; k>=0; k--)
	{
		j = pos32[k]-n;
		if (j < -2000000) j = -2000000;
		pos32[k] = j;
	}

	pz = pos32[0];

	i = -n;
	do
	{
		ch = bend[i];

		if (ch)
		{
			j = i-pos32[ch]; if (j>=70000) j = 0;
			dc[0][j][0]++;
			k = i-pz; if (k>=70000) k = 0;
			dc[1][k][0]++;
			if (pos32[ch]<pz) dc[0][j][1]++; else dc[1][k][1]++;
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
#define MYBUFSIZ (1<<17)
#define LEADIN (70002)
	unsigned short buf[MYBUFSIZ];
	int64_t p=0, nextp=progsize[0];
	int j, progi=0, leadindone=0;

	memset(dc, 0, sizeof(dc));
	memset(pos32, 0, sizeof(pos32));

	while (n<=0 || p<n)
	{
		j=fread(buf, 2, MYBUFSIZ, stdin);
		if (j<=0) break;
		if (n>0 && j>n-p) j = n-p;
		dobuf(buf, j);
		p += j;
		if (progress && 2*p>nextp)
		{
			doan(p-leadindone, 0);
			progsize[progi++] *= 10;
			nextp = progsize[progi];
			if (!nextp) {progi = 0; nextp = progsize[0];}
		}
		if (p>=LEADIN && !leadindone)
			{memset(dc, 0, sizeof(dc)); leadindone = p;}
	}

	if (n>0 && p<n)
	{
		fprintf(stderr, "Warning, expected %.0f words, saw only %.0f\n",
			(double)n, (double)p);
		seterr(1);
	}

	if (p<350000)
	{
		fprintf(stderr, "Warning, < 700 kB, don't trust result.\n");
		seterr(1);
	}

	doan(p-leadindone, 1);
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

	if (n>((int64_t)1)<<47) printf(
"rda16: with such huge amounts of data, counter overflow might happen\n"
"rda16: perhaps you should rebuild with 64 bit counters\n");

	readstuff(n>>1, progress);

	return errorlevel;
}
