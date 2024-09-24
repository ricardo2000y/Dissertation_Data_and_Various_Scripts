/* Test software for gjrand random numbers version 4.2.2.0 or later. */
/* Copyright (C) 2004-2017 G. Jones. */
/* Licensed under the GNU General Public License version 2 or 3. */

/* Parallel Master Control Program. */

/* It runs a lot of the other test programs from this directory. */
/* This one runs somewhat in parallel. If you have multiple CPUs it may */
/* well be faster than mcp. Even on a single CPU box, it will be faster */
/* if you are piping input from a slow generator since all tests use */
/* the same data which is generated just once. */
/* Results should be fairly close to those from mcp on the same data */
/* but not identical, because the different tests here all start at */
/* the start of the data stream (they might not in mcp). */

/* You probably need to be in the testunif directory (not a subdirectory */
/* of it) when you run. */

/* This is a sprintf format string that will build the actual command */
/* lines to be run for each program. */
/* %.50s will be taken from progs[i].name */
/* %.0f will be a numeric argument that the programs expect. */
/* %.350s will be a temporary filename assigned by pmcp. */
/* A different version of comfmt will likely be needed if your system */
/* doesn't have a POSIX compatible sh program. (I'm thinking MS-Windows?) */

static const char * const comfmt="bin/%.50s %.0f >> %.350s 2>&1";

static const char * const mcp_version="PMCP version 13";

struct prog {const char *name; int divisor;};

static const struct prog progs[]=
{
	{"binr -c", 280},
	{"rda", 1},
	{"z9", 1},
	{"diff12", 7},
	{"lownda", 2},
	{"nda", 28},
	{"v256", 1},
	{"mod3", 1},
	{"selfcor", 1},
	{"rda16", 12},
	{"z9 -t", 1},
	{"sh5da", 1},
	{"slicerda", 3},
	{0, 0}
};

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../../src/my_int_types.h"
#include "../../testcommon/pcombo.h"

static char lockname[400]="preport.lck";
static char reportname[400]="preport.txt";

static void
cleanup(int j)
{
	remove(lockname);
	exit(j);
}

#define K ((int64_t)1024)
#define M (K*K)
#define G (K*M)

#define TINY (10*M)
#define SMALL (100*M)
#define STANDARD (G)
#define BIG (10*G)
#define REALLYHUGE (100*G)
#define TERA (K*G)
#define TENTERA (10*TERA)

#define MBUFSIZ (1<<13)

static const char *dir=".";

static char *
tmpfname(int i)
{
	static char buf[350];
	sprintf(buf, "%.299s/tmp_rpt_%02d", dir, i);
	return buf;
}

static void
validchars(const char *dir)
{
	int j, k;

	j = 0;
	if ((k= *dir) == '-') goto bad;

	for (j=0; (k=dir[j])!=0; j++)
	{
		if (j>200) goto bad;
		if (k>='a' && k<='z') continue;
		if (k>='A' && k<='Z') continue;
		if (k>='0' && k<='9') continue;
		if (k=='.') continue;
		if (k=='/') continue;
		if (k=='_') continue;
		if (k=='-') continue;
		if (k==':') continue;
		goto bad;
	}

	return;

	bad:
	fprintf(stderr, "dir length >= %d , character = HEX[%x]\n"
"We allow only the characters A-Z a-z 0-9 . : - _ in directory names\n"
"and set a maximum length of 200 characters.\n", j, k);
	exit(1);
}

#if 0
/* Change 0 to a 1 if your system doesn't provide open(). */
/* This version only uses standard C library files. */
/* It has a race condition, but should work ok unless */
/* someone is out to get you. */
/* It only implements the special case of opening a lockfile */
/* and actually ignores the flags. */

static
int open(const char *name, int fl, ...)
{
	FILE *f=fopen(name, "r");
	if (f) {fclose(f); return -1;}
	f = fopen(name, "w");
	if (!f) return -1;
	return 9;
}
#endif

int
main(int argc, char **argv)
{
#define PVALMAX 29
	FILE * pipes[PVALMAX];
	double pval[PVALMAX], pc, badpv=2.0;
	int j, badness[22], worse;
	char combuf[500], txt[200];
	int64_t size=STANDARD;
	int argi=1, i;
	const char *n, *sizearg="--standard", *badtest="";
	FILE *fp, *tfp;

	setvbuf(stdin, 0, _IONBF, 0);
	setvbuf(stdout, 0, _IONBF, 0);
	setvbuf(stderr, 0, _IONBF, 0);

#if SIGPIPE
	signal(SIGPIPE, SIG_IGN);
#endif

	while (argi<argc)
	{
		if (strcmp(argv[argi], "--tiny")==0) size=TINY;
		else if (strcmp(argv[argi], "--small")==0) size=SMALL;
		else if (strcmp(argv[argi], "--standard")==0) size=STANDARD;
		else if (strcmp(argv[argi], "--big")==0) size=BIG;
		else if (strcmp(argv[argi], "--huge")==0) size=REALLYHUGE;
		else if (strcmp(argv[argi], "--tera")==0) size=TERA;
		else if (strcmp(argv[argi], "--ten-tera")==0) size=TENTERA;
		else if ((i=argv[argi][0]), (i>='1' && i<='9'))
		{
			size = 0;
			for (j=0; i=argv[argi][j], i>='0' && i<='9'; j++)
			{
				size *= 10;
				i -= '0';
				size += i;
			}
			if (i>='a' && i<='z') i += 'A'-'a';
			if (i=='K') size *= K;
			else if (i=='M') size *= M;
			else if (i=='G') size *= G;
		}
		else
		{
			if (strcmp(argv[argi], "--no-rewind")==0) {/* ignore */}
			else if (strcmp(argv[argi], "-d")==0)
			{
				argi++;
				if (argi>=argc)
				{
fputs("-d must have a directory name for next argument.\n", stderr);
					return 1;
				}
				dir = argv[argi];
				validchars(dir);
				sprintf(lockname, "%.299s/preport.lck", dir);
				sprintf(reportname, "%.299s/preport.txt", dir);
			}
			else
			{
				fputs(
"usage: pmcp [ args ]\n"
"    --tiny          (10 MB)\n"
"    --small         (100 MB)\n"
"    --standard      (1 GB) (default)\n"
"    --big           (10 GB)\n"
"    --huge          (100 GB)\n"
"    --tera          (1 TB)\n"
"    --ten-tera      (10 TB)\n"
"    number          set the size in bytes (optionally add k, M or G)\n"
"    -d directory    write report files into this directory.\n"
"Data to be tested for randomness is read from standard input. eg:\n"
"./pmcp --small < somefile.dat\n"
"./gen/blat64v | ./pmcp --huge\n",
					stderr);

				exit(1);
			}
			goto notsize;
		}

		sizearg = argv[argi];

		notsize:
		argi++;
	}

	if (open(lockname, O_WRONLY|O_CREAT|O_EXCL, S_IWUSR|S_IRUSR)<0)
	{
		perror(0);
		fprintf(stderr, "Lockfile %s already exists, or can't be\n"
"created. Possibly you don't have access to the directory, or perhaps pmcp is\n"
"already running. You don't want two pmcp's writing to the same report file\n"
"at once. If the lockfile exists and you are sure pmcp is not running in that\n"
"directory, you could remove the lockfile and then try again.\n",
			lockname);
		exit(1);
	}

	{
		static const int sigs[]=
		{
#if SIGHUP
				SIGHUP,
#endif
#if SIGINT
				SIGINT,
#endif
#if SIGQUIT
				SIGQUIT,
#endif
#if SIGTERM
				SIGTERM,
#endif
				0
		};
		for (j=0; sigs[j]; j++) signal(sigs[j], cleanup);
	}


	/* Start the test programs on pipes. */
	for (i=0; (n=progs[i].name)!=0; i++)
	{
		int64_t s=size/progs[i].divisor;
		FILE *f;

		if (i>=PVALMAX)
		{
			fputs("too many tests, pval[] too small\n", stderr);
			return 1;
		}
		sprintf(combuf, comfmt, n, (double)s, tmpfname(i));
		f=popen(combuf, "w");
		if (f==0)
		{
fprintf(stderr, "pmcp: can't start [%s], crashing.\n", combuf);
			cleanup(1);
		}
		setvbuf(f, 0, _IONBF, 0);
		pipes[i]=f;
	}

	/* Copy our input to all test programs still working. */
	/* Continue until no test programs still working or end of input. */
	while (1)
	{
		char buf[MBUFSIZ];
		int alldone;
		int s=fread(buf, 1, MBUFSIZ, stdin);

		if (s<=0)
		{
			fprintf(stderr,
"warning: End of standard input. Some test programs may hang if short.\n");
			break;
		}

		alldone=1;

		for (i=0; progs[i].name!=0; i++) if (pipes[i])
		{
			int r=fwrite(buf, 1, s, pipes[i]);
			if (r==s) alldone=0;
			else
			{
fprintf(stderr, "short on %d %s\n", i, progs[i].name);
				pclose(pipes[i]);
				pipes[i] = 0;
			}
		}

		if (alldone) break;
	}

	/* Now collect the individual reports from the temporary files, */
	/* print them to preport.txt and stdout, collecting P-values */
	/* as we go. */

	fp = fopen(reportname, "a");
	if (fp==0) {fprintf(stderr, "can't open %s\n", reportname); cleanup(1);}

	fprintf(fp, "\n***** %s %s *****\n\n", mcp_version, sizearg);
	printf("\n***** %s %s *****\n\n", mcp_version, sizearg);
	printf("see %s for details\n", reportname);

	for (i=0; (n=progs[i].name)!=0; i++)
	{
		int64_t s=size/progs[i].divisor;
		int pdone=0;

		fprintf(fp, "\n\n============\n%s 1/%d (%.0f bytes)\n=======\n",
				n, progs[i].divisor, (double)s);

		tfp=fopen(tmpfname(i), "r");
		if (tfp==0)
		{
			fseek(fp, 0L, SEEK_END);
			fprintf(fp, "trouble [%s]\n", progs[i].name);
			fprintf(stderr, "trouble [%s]\n", progs[i].name);
			fprintf(fp, "completed %d ok, 1 crashed\n", i);
			fprintf(stderr, "completed %d ok, 1 crashed\n", i);
			return 1;
		}

		while (fgets(txt, 180, tfp))
		{
			fputs(txt, fp);
			if (txt[0]=='P' && txt[1]==' ' && txt[2]=='=')
			{
				pdone = (sscanf(txt+3, "%lg", pval+i)==1);
				if (pval[i]<badpv)
					{badpv = pval[i]; badtest = n;}
			}
		}
		fclose(tfp);

		if (!pdone)
		{
			fseek(fp, 0L, SEEK_END);
			fprintf(fp, "Missing P value [%s] [%s]\n",
				progs[i].name, tmpfname(i));
			fprintf(stderr, "Missing P value [%s] [%s]\n",
				progs[i].name, tmpfname(i));
			fprintf(fp, "completed %d ok, 1 crashed\n", i);
			fprintf(stderr, "completed %d ok, 1 crashed\n", i);
			cleanup(1);
		}

		/* if not debugging? */
		remove(tmpfname(i));
	}

	/* print summaries */
	fseek(fp, 0L, SEEK_END);
	fprintf(fp, "\n\n====================\ncompleted %d tests\n", i);
	printf("\n\n====================\ncompleted %d tests\n", i);
	for (j=0; j<22; j++) badness[j]=0;
	for (j=0; j<i; j++)
	{
		double d=pval[j], b=0.1;
		int k=0;

		while (d<b && k<20) {b *= 0.1; k++;}
		if (d==0) k = 21;
		badness[k]++;
	}

	fprintf(fp, "%d out of %d tests ok.\n", badness[0], i);
	printf("%d out of %d tests ok.\n", badness[0], i);
	worse = i-badness[0];

	for (j=1; j<22; j++) if (badness[j])
	{
		static const int badmax[7][2]=
		{
				{99, 99},
				{4, 10},
				{1, 3},
				{0, 1},
				{0, 1},
				{0, 1},
				{0, 1}
		};
		const char *msg;

		if (j>6 || worse>badmax[j][1]) msg="(SERIOUSLY BAD)";
		else if (worse>badmax[j][0]) msg="(slightly bad)";
		else msg="(probably ok)";

		fprintf(fp, "%d grade %d failures %s.\n", badness[j], j, msg);
		printf("%d grade %d failures %s.\n", badness[j], j, msg);

		worse-=badness[j];
	}

if (worse!=0) fprintf(stderr, "worse=%d huh?\n", worse);

	pc = pcombo(pval, i);

	n = "ok";
	if (pc<0.01)
	{
		fprintf(fp, "Worst test result [%s].\n", badtest);
		printf("Worst test result [%s].\n", badtest);

		n = "EXTREMELY Worrying and very unusual";
		if (pc>1e-10) n += 10;
		if (pc>1e-8) n += 13;
		if (pc>1e-6) n += 5;
	}

	fprintf(fp,
"\n\nOverall summary one sided P-value (smaller numbers bad)\n"
"P = %.3g : %s\n", pc, n);
	printf(
"\n\nOverall summary one sided P-value (smaller numbers bad)\n"
"P = %.3g : %s\n", pc, n);

	cleanup(0);
	return 0;
}
