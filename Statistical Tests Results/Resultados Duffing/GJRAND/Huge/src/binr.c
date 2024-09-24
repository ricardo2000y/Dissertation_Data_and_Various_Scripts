/* Test software for gjrand random numbers version 4.2.2.0 or later. */
/* Copyright (C) 2004-2017 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

/* Inspired by binary matrix rank tests in DIEHARD. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pscale.h"

#define WORDS 426
/* last element must be WORDS */
static const unsigned short wsizes[]=
{2, 4, 6, 10, 14, 20, 36, 52, 72, 104, 168, 232, 328, WORDS};

typedef uint64_t row_t;
#define BITS 64
#define WBITS (WORDS*BITS)

row_t bigmat[WBITS*WORDS];

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

/* How many Gaussian elimination pivots before obviously singular? */
/* m is trashed by the calculation. */
static int
binrnk(row_t m[], int words)
{
	row_t bit, t;
	int wbits = words*BITS;
	int i, j, k, l, r;

	r = 0;

	for (i=0; i<words; i++) for (bit=1; bit; bit<<=1)
	{
		/* find a pivot with non-zero in the column masked by bit. */
		for (j=r; j<wbits; j++) if (m[j*words+i]&bit) goto pfound;

		break;

		pfound:

		/* exchange pivot to top */
		for (k=i; k<words; k++)
		{
			t=m[r*words+k];
			m[r*words+k]=m[j*words+k]; m[j*words+k]=t;
		}

		/* subtract pivot row from lower rows to zero the column. */
		for (l=r+1; l<wbits; l++) if (m[l*words+i]&bit)
			for (k=i; k<words; k++) m[l*words+k] ^= m[r*words+k];

		r++;
	}

	return r;
}

static double
prob(int j)
{
	double p;

	if (j<=0) return 1.0;
	p = 2.0;
	do {p *= 0.5;} while (--j);

	return p;
}

static time_t tstart;

static void
doan(double pvl, int npv, int final)
{
	time_t tm = time(0);

	printf("processed in %.3g seconds. %s\n",
		(double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pco_scale(pvl, npv));
	if (!final) printf("------\n\n");
}

static void
readstuff(int64_t max, int cut)
{
	double pv1, pvl=2.0;
	int64_t p=0;
	int words, wd, npv=0;

	words = wd = wsizes[0];

	while (max<0 || p<max)
	{
		int w, b;
		if (wd<WORDS) wd = wsizes[npv];
		else if (cut && p<max) {cut = 0; max = ((max-p)>>2) + p;}
		w = wd*wd*BITS;
		if (p+w*(int)sizeof(row_t)>max) goto done;
		if ((int)fread(bigmat, sizeof(row_t), w, stdin)!=w) break;
		p += w*sizeof(row_t);
		b = binrnk(bigmat, wd);
		pv1 = prob(wd*BITS-b);
		npv++;
		words = wd;
		if (pv1<pvl)
		{
			pvl = pv1;
			printf("dim = %d ; rnk = %d ; p = %.3g\n",
				wd*BITS, b, pv1);
			if (pvl<=0.0) goto done;
		}
	}

	if (max>0 && p<max)
	{
		fprintf(stderr, "Warning, expected %.0f bytes, saw only %.0f\n",
			(double)max, (double)p);
		seterr(1);
	}

	done:
	printf("done %d matrices ; largest %d X %d\n\n",
		npv, words*BITS, words*BITS);

	doan(pvl, npv, 1);
}

int
main(int argc, char **argv)
{
	double dmax;
	int64_t max;
	int cut=0;

	tstart = time(0);

	if (argc>=2 && !strcmp(argv[1], "-c")) {cut++; argv++; argc--;}
	if (argc<2 || sscanf(argv[1], "%lf", &dmax)!=1 || dmax<1.0) max= -1;
	else max=dmax;

	readstuff(max, cut);

	return errorlevel;
}
