/* Test software for gjrand random numbers version 4.1.1.0 or later. */
/* Copyright (C) 2004-2013 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

#define SHIFT 16
#define SIZE (1<<SHIFT)

#define MAXMEM (SIZE*9+400)
static double memory[MAXMEM+3];
static double *memptr=memory;

static short tmpacc[2*SIZE];
static int tmpsat=0;

static double *
memmark(int siz)
{
	double *r=memptr;
	memptr=r+siz; if (memptr>memory+MAXMEM) crash("out of memory");
	return r;
}

static void
memrelease(double *mark) {memptr=mark;}


struct poly {int order; double *coef;};

static void
polysub2(double *ca, double *cr, int ord)
{
	int j=ord-2;

	do {cr[1+j]-=ca[1+j]; cr[j]-=ca[j]; j-=2;} while (j>=0);
}

static void
polyadd2(double *ca, double *cr, int ord)
{
	int j=ord-2;

	do {cr[1+j]+=ca[1+j]; cr[j]+=ca[j]; j-=2;} while (j>=0);
}

static void
polyadd3(double *ca, double *cb, double *cr, int ord)
{
	int j=ord-2;

	do {cr[1+j]=ca[1+j]+cb[1+j]; cr[j]=ca[j]+cb[j]; j-=2;} while (j>=0);
}

static void
polymul4(const double *ca, const double *cb, double *cr)
{
	cr[0] += ca[0]*cb[0];
	cr[1] += ca[0]*cb[1]+ca[1]*cb[0];
	cr[2] += ca[0]*cb[2]+ca[1]*cb[1]+ca[2]*cb[0];
	cr[3] += ca[0]*cb[3]+ca[1]*cb[2]+ca[2]*cb[1]+ca[3]*cb[0];
	cr[4] += ca[1]*cb[3]+ca[2]*cb[2]+ca[3]*cb[1];
	cr[5] += ca[2]*cb[3]+ca[3]*cb[2];
	cr[6] += ca[3]*cb[3];
}

static void
polymul2(double *ca, double *cb, double *cr, int ord)
{
	const int small=8;

	{
		int j;
		for (j=0; j<2*ord; j++) cr[j]=0;
	}

	if (ord==4*small)
	{
		int j, k;

		for (j=4*(small-1); j>=0; j-=4) for (k=4*(small-1); k>=0; k-=4)
			polymul4(ca+j, cb+k, cr+j+k);

		return;
	}

	{
		struct poly t1, t3;

		t1.order=ord; t3.order=ord;
		t1.coef=memmark(ord);

		polymul2(ca, cb, t1.coef, ord/2);
		polyadd2(t1.coef, cr, ord);
		polysub2(t1.coef, cr+ord/2, ord);

		polymul2(ca+ord/2, cb+ord/2, t1.coef, ord/2);
		polyadd2(t1.coef, cr+ord, ord);
		polysub2(t1.coef, cr+ord/2, ord);

		polyadd3(ca, ca+ord/2, t1.coef, ord/2);
		polyadd3(cb, cb+ord/2, t1.coef+ord/2, ord/2);

		t3.coef=memmark(ord);
		polymul2(t1.coef, t1.coef+ord/2, t3.coef, ord/2);

		polyadd2(t3.coef, cr+ord/2, ord);

		memrelease(t1.coef);
	}
}

static struct poly
polymul(struct poly a, struct poly b)
{
	double *dummy;
	struct poly r;

	if (a.order!=b.order) crash("polymul order mismatch");
	r.order = 2*a.order;
	r.coef = memmark(r.order);
	dummy = memmark(1);
	polymul2(a.coef, b.coef, r.coef, a.order);
	memrelease(dummy);

	return r;
}

static struct poly
revpoly(struct poly p)
{
	struct poly r;
	int j;

	r.order=p.order;
	r.coef=memmark(p.order);
	for (j=0; j<p.order; j++) r.coef[j]=p.coef[p.order-j-1];

	return r;
}

static void
zeropoly(struct poly r)
{
	int j;
	for (j=0; j<SIZE; j++) r.coef[j]=0.0;
}

static double
printstats(struct poly p, int64_t bcount, const char *msg)
{
	int i, j, count, jext= -1;
	double *accstats;
	double x, ext=0.0, aext= -999.0, le, s=0.0, ss=0.0;
	double scale=4.5777764203336997e-05/(bcount*sqrt((double)SIZE));

	count = SIZE/2;
	accstats = memmark(count);

if (p.order!=2*SIZE) crash("printstats bad p.order");
	for (j=0; j<SIZE/2; j++) accstats[j] = p.coef[SIZE+j]+p.coef[j];

	for (i=0; i<count; i++)
	{
		j=i+1;
		x=accstats[i]*scale;
		if (fabs(x) > aext) {aext = fabs(x); ext = x; jext = j;}
		s+=x; ss+=x*x;
	}

	printf("%s : mean=%f ; sd=%f ; ext = %f (%d)\n", msg,
		s/count, sqrt(ss/count-s*s/((double)count*(double)count)),
		ext, jext);

	/* Somewhere here i suspect my probability calculations are wrong. */
	/* On the high side, they seem to be somewhere near right. On the */
	/* low side, very low numbers seem to be much rarer than these */
	/* figures suggest. */

	le = erfc(M_SQRT1_2*aext)*0.5;
	le = pco_scale(le, (double)SIZE);
	printf("==> p = %.3g\n", (double)le);

	return le;
}

static time_t tstart;

static void
doan(int64_t n, int final, struct poly accf0, struct poly accr1)
{
	struct poly q, m;
	double pvs[2], *mark;
	time_t tm;

	mark = memmark(1);
	q=revpoly(accf0);
	m=polymul(accf0, q);
	pvs[0] = printstats(m, n, "forward (msb)");
	memrelease(mark);

	mark = memmark(1);
	q=revpoly(accr1);
	m=polymul(accr1, q);
	pvs[1] = printstats(m, n, "reverse (lsb)");
	memrelease(mark);

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		(double)n*2*SIZE, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(pvs, 2));
	if (!final) printf("------\n\n");
}

static short transf[256], transr[256];

static void
mktrans(void)
{
	int i, j;

	for (i=0; i<256; i++)
	{
		j=i;
		transf[i]=2*j-255;
		j = ((i>>4)&0xf) | ((i&0xf)<<4);
		j = ((j>>2)&0x33) | ((j&0x33)<<2);
		j = ((j>>1)&0x55) | ((j&0x55)<<1);
		transr[i]=2*j-255;
	}
}

static void
dotmpacc(unsigned short *buf)
{
	short *tp=tmpacc+2*SIZE;
	unsigned j, k;
	int i;

	buf += SIZE;
	for (i= -SIZE; i<0; i++)
	{
		j = buf[i]; k = j>>8; j &= 255;
		k = transf[k]; j = transr[j];
		tp[2*i] += k; tp[2*i+1] += j;
	}
}

static void
desat(double *f0, double *r1)
{
	int i;

	for (i=0; i<SIZE; i++)
	{
		f0[i] += tmpacc[2*i]; r1[i] += tmpacc[2*i+1];
		tmpacc[2*i] = 0; tmpacc[2*i+1] = 0;
	}

	tmpsat=0;
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
	unsigned short buf[SIZE];
	struct poly accf0, accr1;
	int64_t p=0, nextp=progsize[0];
	int progi=0;

	memset(tmpacc, 0, sizeof(tmpacc));

	accf0.order=SIZE; accf0.coef=memmark(SIZE);
	zeropoly(accf0);
	accr1.order=SIZE; accr1.coef=memmark(SIZE);
	zeropoly(accr1);

	while (fread(buf, 2, SIZE, stdin)==SIZE)
	{
		dotmpacc(buf);
		tmpsat++;
		if (tmpsat>=120) desat(accf0.coef, accr1.coef);
		p++;
		if (n>=0 && p>=n) break;
		if (progress && p*2*SIZE>nextp)
		{
			desat(accf0.coef, accr1.coef);
			doan(p, 0, accf0, accr1);
			progsize[progi++] *= 10;
			nextp = progsize[progi];
			if (!nextp) {progi = 0; nextp = progsize[0];}
		}
	}

	desat(accf0.coef, accr1.coef);

	if (n>0 && p<n)
	{
fprintf(stderr, "Warning, selfcor expected %.0f blocks, saw only %.0f\n",
			(double)n, (double)p);
		seterr(1);
	}

	doan(p, 1, accf0, accr1);
}

int
main(int argc, char **argv)
{
	double dn;
	int64_t n= -1;
	int progress=0, j;

	tstart = time(0);

	mktrans();

	for (j=1; j<argc; j++)
	{
		if (strcmp(argv[j], "--progress")==0) progress = 1;
		else if (sscanf(argv[j], "%lf", &dn)==1)
			{if (dn>0) n = (int64_t)(dn*(1.0/(2*SIZE)));}
		else crash("optional arg must be --progress or numeric");
	}

	readstuff(n, progress);

	return errorlevel;
}
