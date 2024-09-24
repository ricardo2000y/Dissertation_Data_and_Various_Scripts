/* Test software for gjrand random numbers version 4.2.1.0 or later. */
/* Copyright (C) 2004-2014 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

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

#define DIM 9
#define SIZE (3*3*3 * 3*3*3 * 3*3*3)

static long sat=0;
static unsigned char reduce[1024];
static struct nc {unsigned short nextbase; unsigned short scount;} nc[SIZE];
static double count[SIZE];

static void
init(void)
{
	int j;

	for (j=0; j<SIZE; j++)
		{nc[j].nextbase = 3*j % SIZE; nc[j].scount = 0; count[j] = 0.0;}
	for (j=0; j<1024; j++) reduce[j] = j%3;
}

static void
desat(void)
{
	long c=0;
	int j, k;

	for (j=0; j<SIZE; j++)
		{k = nc[j].scount; count[j] += k; c += k; nc[j].scount = 0;}

	if (c!=sat)
	{
		fprintf(stderr, "Counters overflowed. Seriously non-random.\n");
		printf("one sided P value (very small numbers are bad)\n\n"
			"P = 0\n");
		exit(0);
	}

	sat = 0;
}

static uint32_t
dobuf(const uint32_t *buf, int i, uint32_t st)
{
	struct nc *p;
	const unsigned char *b;
	int j;

	buf += i;
	j = -i;
	do
	{
		b = (const unsigned char *)(buf+j);
		p = nc+st;
		st = p->nextbase + reduce[(b[0]+b[1])+(b[2]+b[3])];
		p->scount++;
		j++;
	} while (j<0);

	sat += i;
	if (sat>10000000) desat();

	return st;
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
		p1[j] = (2*a-b-c) * CORRECT6;
		p2[j] = (b-c) * M_SQRT1_2;
	}

	if (stride>1)
	{
		stride /= 3;
		mix3(ct, stride); mix3(p1, stride); mix3(p2, stride);
	}
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double tct[SIZE];
	double pv[2];
	double c, t, x;
	time_t tm;
	int j;

	if (sat) desat();

	memcpy(tct, count, sizeof(tct));

	if (n<200000)
	{
		fprintf(stderr, "Warning, < 800 K, don't trust result.\n");
		seterr(1);
	}

	x = n/((double)SIZE);
	c = 0.0;
	for (j=0; j<SIZE; j++) {t = tct[j]-x; c += t*t;}
	c /= x;
	c -= (c-(SIZE-1))*0.38; /* approx correction for covariance. */

	pv[0] = chi2p1(c, (SIZE-1));
	printf("chis = %15g ; df = %d ; p = %.5g\n", c, (SIZE-1), pv[0]);

	t = 1.0/sqrt(x);
	for (j=0; j<SIZE; j++) tct[j] = (tct[j]-x)*t;
	mix3(tct, SIZE/3);

	t = -1.0;
	for (j=(SIZE/3); j<SIZE; j++) {x = fabs(tct[j]); if (x>t) t = x;}

	x = erfc(M_SQRT1_2*t);
	pv[1] = pco_scale(x, (double)(SIZE-(SIZE/3)));
	printf("mix3 extreme = %.5f ; p = %.5g\n", t, pv[1]);

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		n*4.0, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(pv, 2));
	if (!final) printf("------\n\n");
}

static void
readstuff(int64_t n, int progress)
{
	static int64_t progsize[]=
	{
		10000000,
		15000000,
		20000000,
		30000000,
		50000000,
		70000000,
		0
	};

	uint32_t buf[2048];
	int64_t pos=0, nextp=progsize[0];
	uint32_t st=538;
	int i, progi=0;

	while (n<0 || pos<n)
	{
		i = fread(buf, 4, 2048, stdin);
		if (i<=0) break;
		if (n>0 && i>n-pos) i = n-pos;
		st = dobuf(buf, i, st);
		pos+=i;
		if (i<2048) break;
		if (progress && pos*4>nextp)
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
			n*4.0, pos*4.0);
		seterr(1);
	}

	doan(pos, 1);
}

int
main(int argc, char **argv)
{
	double dn;
	int64_t n= -1;
	int progress=0, j;

	tstart = time(0);

	init();

	for (j=1; j<argc; j++)
	{
		if (strcmp(argv[j], "--progress")==0) progress = 1;
		else if (sscanf(argv[j], "%lf", &dn)==1) n = (int64_t)(dn*0.25);
		else crash("optional arg must be --progress or numeric");
	}

	readstuff(n, progress);

	return errorlevel;
}
