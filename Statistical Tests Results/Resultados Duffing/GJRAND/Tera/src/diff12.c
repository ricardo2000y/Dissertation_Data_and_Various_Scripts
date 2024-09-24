/* Test software for gjrand random numbers version 4.2.1.0 or later. */
/* Copyright (C) 2004-2014 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"
#include "../../testcommon/chi2p.h"

static int errorlevel=0;
static void
seterr(int e) {if (errorlevel<e) errorlevel=e;}

static void
crash(const char *s)
{
	fflush(stdout);
	seterr(1); fprintf(stderr, "crash [%s]\n", s); exit(errorlevel);
}

#define DIM 12
#define SHIFT 11
#define SIZ (1<<SHIFT)

struct res {uint16_t x[DIM]; uint64_t n;};
static struct res res[DIM][SIZ];

static void
init(void)
{
	int i, j, k;

	for (j=0; j<DIM; j++) for (i=0; i<SIZ; i++)
	{
		struct res *p = &(res[j][i]);
		for (k=DIM-1; k>=0; k--) p->x[k] = SIZ;
		p->n = 0;
	}
}

static void
dopoint(uint16_t x[DIM], int shift)
{
	uint16_t y[DIM];
	int i, j, k;
	struct res *ptr;

	if (shift) for (j=DIM-1; j>=0; j--) x[j] = (x[j]>>shift) & (SIZ-1);
	else for (j=DIM-1; j>=0; j--) x[j] &= (SIZ-1);

	for (i=DIM-1; i>=0; i--)
	{
		for (j=i; j>=0; j--) y[j] = x[j];
		ptr = &(res[i][y[i]]);
		ptr->n++;
		for (j=i; j>=0; j--) x[j] = (x[j] - ptr->x[j]) & (SIZ-1);
		for (j=i; j>=0; j--)
		{
			k = ((int)(y[j])) - ((int)(ptr->x[j]));
			if (k>0) goto done;
			if (k<0) goto doit;
		}

		doit:
		for (j=i; j>=0; j--) ptr->x[j] = y[j];

		done: ;
	}
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double pval[DIM], p, e, t;
	uint64_t tot;
	time_t tm;
	struct res *ptr;
	int j, k;

	for (j=DIM-1; j>=0; j--)
	{
		tot = 0;
		ptr = &(res[j][0]);
		for (k=SIZ-1; k>=0; k--) tot += ptr[k].n;
		if (tot < 10*SIZ) break;

		e = tot/((double)SIZ);
		p = 0.0;
		for (k=SIZ-1; k>=0; k--) {t = ptr[k].n-e; p += t*t;}
		p /= e;
		printf("order = %2d : chis = %10.0f", DIM-1-j, p);
		p = chi2p2(p, SIZ-1);
		printf(" ;  p = %.6g\n", p);
		pval[j]=p;
	}

	j = DIM-1-j;
	if (j<=0)
	{
		fprintf(stderr, "blocks = %15.0f ; wanted >= %ld\n",
			(double)tot, 10L*SIZ);
		crash("not enough data, try with more");
	}

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		(double)n*(2*DIM), (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");

	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(pval, j));
	if (!final) printf("------\n\n");
}

static void
readstuff(int64_t n, int shift, int progress)
{
	static int64_t progsize[]=
	{
		70000000,
		100000000,
		150000000,
		200000000,
		300000000,
		500000000,
		0
	};
	uint16_t v[DIM*512];
	int64_t p=0, nextp=progsize[0];
	int progi=0, j, k;

	init();

	do
	{
		if ((j = fread(v, sizeof(uint16_t)*DIM, 512, stdin)) <= 0)
			break;
		for (k=0; k<j; k++) dopoint(v+k*DIM, shift);
		p += j;
		if (progress && p*(2*DIM)>nextp)
		{
			doan(p, 0);
			progsize[progi++] *= 10;
			nextp = progsize[progi];
			if (!nextp) {progi = 0; nextp = progsize[0];}
		}
	} while (n<0 || p<n);

	if (n>=0 && p<n)
	{
		fprintf(stderr, "warning expected %.0f points, saw only %.0f\n",
			(double)n, (double)p);
		seterr(1);
	}

	doan(p, 1);
}

int
main(int argc, char **argv)
{
	double dn;
	int64_t n= -1;
	int progress=0, shift=0, j;

	tstart = time(0);

	for (j=1; j<argc; j++)
	{
		if (strcmp(argv[j], "--progress")==0) progress = 1;
		else if (strncmp(argv[j], "--shift=", 8)==0)
			sscanf(argv[j]+8, "%d", &shift);
		else if (sscanf(argv[j], "%lf", &dn)==1)
			n = (int64_t)dn/(DIM*sizeof(uint16_t));
		else crash("optional args : --progress --shift=n or numeric");
	}

	if (shift<0 || shift>5) crash("shift count must be 0 to 5");

	readstuff(n, shift, progress);

	return errorlevel;
}
