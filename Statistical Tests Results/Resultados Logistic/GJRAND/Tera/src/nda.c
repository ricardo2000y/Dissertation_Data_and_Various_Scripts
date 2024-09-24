/* Test software for gjrand random numbers version 3.4.1.0 or later. */
/* Copyright (C) 2004-2011 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"
#include "../../testcommon/binom.h"

/* Random bytes are read from standard input. There is an optional */
/* size arg in bytes, otherwise it just reads until end of file. */

static void
crash(const char *s) {fprintf(stderr, "crash [%s]\n", s); exit(1);}

static int errorlevel=0;
static void seterr(int e) {if (errorlevel<e) errorlevel=e;}

static double dc[16][16][50];
static int64_t count[16][16][50];
static int32_t c32[16][16][50];
static int32_t last[16];

static void
init(void)
{
	int i;
	memset(count, 0, sizeof(count));
	memset(c32, 0, sizeof(c32));
	for (i=0; i<16; i++) last[i]= -11;
}

static void
dobyte(const int b, int pos)
{
	int j, n;
	int32_t *c;

	n = b&15;
	c = &(c32[n][0][0]);

	for (j= -16; j<0; j++)
	{
		int d=pos-(last+16)[j];
		if (d>49) d=49;
		c[d]++;
		c+=50;
	}

	last[n]=pos;

	pos++;
	n = (b>>4)&15;
	c = &(c32[n][0][0]);

	for (j= -16; j<0; j++)
	{
		int d=pos-(last+16)[j];
		if (d>49) d=49;
		c[d]++;
		c+=50;
	}

	last[n]=pos;
}

static void
dobuf(const unsigned char *buf, int pos, int i)
{
	int j;

	buf+=i;

	for (j= -i; j<0; j++)
	{
		dobyte(buf[j], pos);
		pos += 2;
	}
}

static void
butterfly(double *x, double *y)
{
	double a, b;
	int siz=y-x;

	do
	{
		a = *x; b = *y;
		*x = a+b; *y = a-b;
		x++; y++;
		siz--;
	} while (siz>0);

	siz = y-x;
	if (siz>50)
	{
		x -= siz; y -= siz;
		siz >>= 1;
		butterfly(x, x+siz);
		butterfly(y, y+siz);
	}
}

static time_t tstart;

static void
doan(int64_t n, int final)
{
	double p[2];
	double t=0.0, extreme;
	time_t tm;
	int exi, exj, exk, exd='?';
	int i, j, k;

	t=0.0; extreme= 999.0; exi=exj=exk= -1;
	for (i=0; i<16; i++) for (j=0; j<16; j++)
	{
		uint64_t tot=0;
		for (k=49; k>0; k--) tot+=count[i][j][k];
		for (k=1; k<=48; k++)
		{
			double y, z;
			uint64_t x=count[i][j][k];

			if (tot<=0) tot = 1;
			y=sumbino(tot, x, 1.0/16.0);
			z = tot*(1.0/16.0);

			if (y<extreme)
			{
				extreme=y; exi=i; exj=j; exk=k;
				if (x>z) exd='+'; else exd='-';
			}

			dc[i][j][k] = (x-z) / sqrt(z);
			tot-=x;
		}
	}

	printf("extreme = %.3g   (%d %d %d %c)\n", extreme, exi, exj, exk, exd);
	p[0] = pco_scale(extreme, 16*16*48);

	butterfly(&(dc[0][0][0]), &(dc[8][0][0]));
	extreme = -1.0;
	for (i=0; i<16; i++) for (j=0; j<16; j++) for (k=1; k<=48; k++)
	{
		t = fabs(dc[i][j][k]);
		if (t>extreme) {extreme = t; exi = i; exj = j; exk = k;}
	}

	extreme /= 16.0;
	printf("transform = %.3g   (%x %x %d)\n", extreme, exi, exj, exk);

	extreme = erfc(M_SQRT1_2*extreme);
	p[1] = pco_scale(extreme, 16*16*48);

	printf("pvals (%.3g %.3g)\n", p[0], p[1]);

	tm = time(0);
	printf("\nprocessed %.2g bytes in %.3g seconds. %s\n",
		n*0.5, (double)(tm-tstart), ctime(&tm));

	if (!final) printf("progress ");
	printf("one sided P value (very small numbers are bad)\n");
	printf("%c = %.3g\n", "pP"[final], pcombo(p, 2));
	if (!final) printf("------\n\n");
}


static void
adjust(int pos)
{
	int i, j, k;

	for (i=0; i<16; i++)
	{
		j=last[i]-pos;
		if (j< -9000000) j= -9000000;
		last[i]=j;
	}

	for (i=0; i<16; i++) for (j=0; j<16; j++) for (k=1; k<=49; k++)
	{
		count[i][j][k]+=c32[i][j][k];
		c32[i][j][k]=0;
	}
}

static void
readstuff(int64_t n, int progress)
{
	static int64_t progsize[]=
	{
		500000,
		700000,
		1000000,
		1500000,
		2000000,
		3000000,
		0
	};
	unsigned char buf[4096];
	int64_t posll=0, nextp=progsize[0];
	int pos=0, j, progi=0;

	n <<= 1;

	while (n<0 || posll<n)
	{
		j=fread(buf, 1, 4096, stdin);
		if (j<=0) break;
		j+=j;
		if (n>0 && j>n-posll) j=n-posll;
		dobuf(buf, pos, j>>1);
		posll+=j; pos+=j;
		if (pos>=4000000) {adjust(pos); pos=0;}
		if (progress && posll>nextp*2)
		{
			if (pos) {adjust(pos); pos=0;}
			doan(posll, 0);
			progsize[progi++] *= 10;
			nextp = progsize[progi];
			if (!nextp) {progi = 0; nextp = progsize[0];}
		}
	}

	adjust(pos);

	if (n>0 && posll<n)
	{
		fprintf(stderr,
			"Warning, expected %.0f 4bit samples, saw only %.0f\n",
			(double)n, (double)posll);
		seterr(1);
	}

	if (posll<100000)
	{
		fprintf(stderr, "Warning, < 100000 samples, don't trust.\n");
		seterr(1);
	}

	doan(posll, 1);
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

	init();

	readstuff(n, progress);

	return errorlevel;
}
