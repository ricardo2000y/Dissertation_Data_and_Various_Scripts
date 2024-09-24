/* Test software for gjrand random numbers version 4.2.1.0 or later. */
/* Copyright (C) 2004-2014 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

/* This is tests related to the balance of 0 and 1 bits in the test data. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"
#include "../../testcommon/chi2p.h"

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

#define DIM 8
#define SIZE (3*3*3*3 * 3*3*3*3)

#define SZS 3

static long sat=0;
static struct nc
	{unsigned short nextbase; unsigned short scount; int32_t tots;}
	nc[SZS][SIZE];
static struct dc {double c, t;} ctz9[SZS][SIZE];

static char red32[33], red128[129], red512[513];

static void
initz9(void)
{
	int j, k;

	for (k=0; k<SZS; k++) for (j=0; j<SIZE; j++)
	{
		nc[k][j].nextbase = 3*j % SIZE; nc[k][j].scount = 0;
		nc[k][j].tots = 0;
		ctz9[k][j].c = 0.0; ctz9[k][j].t = 0.0;
	}

	for (j=0; j<15; j++) red32[j] = 0;
	for (j=15; j<18; j++) red32[j] = 1;
	for (j=18; j<33; j++) red32[j] = 2;
	for (j=0; j<61; j++) red128[j] = 0;
	for (j=61; j<68; j++) red128[j] = 1;
	for (j=68; j<129; j++) red128[j] = 2;
	for (j=0; j<250; j++) red512[j] = 0;
	for (j=250; j<262; j++) red512[j] = 1;
	for (j=262; j<513; j++) red512[j] = 2;
}

static uint32_t
dobufz9(const short *buf, uint32_t st, int n, int iter, const char reduce[])
{
	struct nc *p, *base;
	int j, bc;

	base = &(nc[iter][0]);

	for (j=0; j<n; j++)
	{
		p = base+st;
		bc = buf[j];
		st = p->nextbase + reduce[bc];
		p->scount++;
		p->tots += bc;
	}

	return st;
}

static uint64_t
mktrcounts(const uint64_t buf[512], uint64_t ts, short cts[1024])
{
	uint64_t t, u, us, mask;
	unsigned sh;
	int j;

	for (j=0; j<512; j+=2)
	{
		t = buf[j]; us = t>>63; u = buf[j+1];
		t = (t<<1) ^ t ^ ts; ts = u>>63; u = (u<<1) ^ u ^ us;

		sh = 1; mask = 0x55555555uL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 2; mask = 0x33333333uL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 4; mask = 0xf0f0f0fuL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 8; mask = 0xff00ffuL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 16; mask = 0xffuL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);

#if GJRAND_LITTLE_ENDIAN
		cts[2*j] = ((int)t)-16; cts[2*j+1] = ((int)(t>>32))-16;
		cts[2*j+2] = ((int)u)-16; cts[2*j+3] = ((int)(u>>32))-16;
#else
		cts[2*j+1] = ((int)t)-16; cts[2*j] = ((int)(t>>32))-16;
		cts[2*j+3] = ((int)u)-16; cts[2*j+2] = ((int)(u>>32))-16;
#endif
	}

	return ts;
}

static void
mkcounts(const uint64_t buf[512], short cts[1024])
{
	uint64_t t, u, mask;
	unsigned sh;
	int j;

	for (j=0; j<512; j+=2)
	{
		t = buf[j]; u = buf[j+1];

		sh = 1; mask = 0x55555555uL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 2; mask = 0x33333333uL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 4; mask = 0xf0f0f0fuL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 8; mask = 0xff00ffuL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);
		sh = 16; mask = 0xffuL; mask |= mask<<32;
		t = ((t>>sh)&mask) + (t&mask);
		u = ((u>>sh)&mask) + (u&mask);

#if GJRAND_LITTLE_ENDIAN
		cts[2*j] = ((int)t)-16; cts[2*j+1] = ((int)(t>>32))-16;
		cts[2*j+2] = ((int)u)-16; cts[2*j+3] = ((int)(u>>32))-16;
#else
		cts[2*j+1] = ((int)t)-16; cts[2*j] = ((int)(t>>32))-16;
		cts[2*j+3] = ((int)u)-16; cts[2*j+2] = ((int)(u>>32))-16;
#endif
	}
}

#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752438
#endif
#define CORRECT3 0.57735026918962576451
#define CORRECT6 0.40824829046386301636

static void
mix3(double ct[], int stride)
{
	double *p1=ct+stride, *p2=p1+stride;
	double a, b, c;
	int j;

	for (j=0; j<stride; j++)
	{
		a = ct[j]; b = p1[j]; c = p2[j];
		ct[j] = (a+b+c) * CORRECT3;
		p1[j] = (a-c) * M_SQRT1_2;
		p2[j] = (2*b-a-c) * CORRECT6;
	}

	stride /= 3;
	if (stride) {mix3(ct, stride); mix3(p1, stride); mix3(p2, stride);}
}

static void
desat(void)
{
	int j, k, t;

	for (k=0; k<SZS; k++)
	{
		long c=0;
		for (j=0; j<SIZE; j++)
		{
			t = nc[k][j].scount; ctz9[k][j].c += t; c += t;
			ctz9[k][j].t += nc[k][j].tots;
			nc[k][j].scount = 0; nc[k][j].tots = 0;
		}

		if (k==0 && c!=sat)
		{
fprintf(stderr, "Counters overflowed. Seriously non-random.\n");
printf("one sided P value (very small numbers are bad)\n\nP = 0\n");
exit(0);
		}
	}

	sat = 0;
}

static void
print3(int j) {do {putchar(j%3+'0'); j /= 3;} while (j);}

/* categorise j as low (0), medium (1), or high (2) */
/* weight based on ternary digits. */
static int
cat(unsigned j)
{
	int r=0;

	while (j) {r += j%3; j /= 3;}
	return (r>2) + (r>4);
}

static double
doanz9(int szs, int bits)
{
	double tct[SIZE], sigma[3]={-1, -1, -1};
	double pv, t, x, db=bits*0.25;
	int j, lags[3]={-1, -1, -1}, wcount[3]={0, 0, 0};

	if (sat) desat();

	for (j=0; j<SIZE; j++)
	{
		if (ctz9[szs][j].c == 0.0) tct[j] = 0.0;
		else tct[j] = ctz9[szs][j].t / sqrt(ctz9[szs][j].c*db);
	}

	mix3(tct, SIZE/3);

	pv = 2.0;
	for (j=1; j<SIZE; j++)
	{
		int c = cat(j);
		wcount[c]++;
		x = fabs(tct[j]);
		if (x>sigma[c]) {lags[c] = j; sigma[c] = x;}
	}

	for (j=0; j<3; j++)
	{
		static const char *catstr[3]={"low", "medium", "high"};
		printf("mix3 extreme = %.5f (lags = ", sigma[j]);
		print3(lags[j]);
		printf(") %s weight %d\n", catstr[j], wcount[j]);
		t = erfc(M_SQRT1_2*sigma[j]);
		t = pco_scale(t, (double)(wcount[j]));
		if (t<pv) pv = t;
	}
	pv = pco_scale(pv, 3.0);

	printf("bits per word =%4d : sub-test P value = %.3g\n\n", bits, pv);
	return pv;
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double pv[SZS];
	int64_t ns=n;
	time_t tm;
	int szs = 0, bits=32;

	do
	{
		pv[szs] = doanz9(szs, bits);
		szs++;
		ns >>= 2;
		bits <<= 2;
	} while (szs<SZS && ns>1900000);

	tm = time(0);
	printf("processed %.2g bytes in %.3g seconds. %s\n",
		(double)n, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(pv, szs));
	if (!final) printf("------\n\n");
}

static void
redcounts(short cts[], int n)
{
	int j;
	for (j=0; j<n; j++)
		cts[j] = cts[4*j] + cts[4*j+1] + cts[4*j+2] + cts[4*j+3];
}

static void
readstuff(int64_t n, int trans, int progress)
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
	static uint32_t z9st[SZS] = {1544, 1544, 1544};
	uint64_t buf[512], ts=0;
	short cts[1024];
	int64_t pos=0, nextp=progsize[0];
	int j, progi=0;

	while (n<0 || pos<n)
	{
		j = fread(buf, 4096, 1, stdin);
		if (j!=1) break;
		if (trans) ts = mktrcounts(buf, ts, cts);
		else mkcounts(buf, cts);
		z9st[0] = dobufz9(cts, z9st[0], 1024, 0, red32+16);
		redcounts(cts, 256);
		z9st[1] = dobufz9(cts, z9st[1], 256, 1, red128+64);
		redcounts(cts, 64);
		z9st[2] = dobufz9(cts, z9st[2], 64, 2, red512+256);
		sat += 1024; if (sat>10000000) desat();
		pos += 4096;
		if (progress && pos>nextp)
		{
			doan(pos, 0);
			progsize[progi++] *= 10;
			nextp = progsize[progi];
			if (!nextp) {progi = 0; nextp = progsize[0];}
		}
	}

	if (n>0 && pos<n)
	{
		fprintf(stderr, "Warning, expected %.0f bytes, saw only %.0f\n",
			(double)n, (double)pos);
		seterr(1);
	}

	if (pos<10100000)
	{
		fprintf(stderr,
			"Warning, < 10100000 bytes, don't trust result.\n");
		seterr(1);
	}

	if (trans) printf("transitions\n");

	doan(pos, 1);
}

int
main(int argc, char **argv)
{
	double dn;
	int64_t n= -1;
	int trans=0, progress=0, j;

	tstart = time(0);

	for (j=1; j<argc; j++)
	{
		if (strcmp(argv[j], "--progress")==0) progress = 1;
		else if (strcmp(argv[j], "-t")==0) trans = 1;
		else if (sscanf(argv[j], "%lf", &dn)==1) n = (int64_t)dn;
		else crash("optional arg must be --progress or -t or numeric");
	}

	if (n>0) n &= ~((int64_t)4095);
	initz9();
	readstuff(n, trans, progress);

	return errorlevel;
}
