The programs in this directory relate to uniform random bitstreams.

Each program corresponds to a single .c file. The blat* programs
need to be linked with the gjrand library. Others should need only the
standard C library and math library.
compile is a shell script that compiled it all on my machine.

Some need the erfc() function. Some C compilers supply erfc() most
likely in the math library, or maybe a SYSV or BSD compatibility library.
If you don't have it, a poor quality version is supplied in
../../testcommon/erfc.h .

See also file TESTS.txt for a discussion of some of the tests.
Also comments inside the source files.

blatbytes
=========
writes a stream of pseudo-random bits to standard output. It uses
gjrand_randbytes() to generate the bits. It takes up to two optional arguments
size
optional seed.

size should be a number. If positive, blatbytes writes
that many bytes to standard output. If negative, blatbytes attempts to
write forever. This could be useful in a pipeline to one of the
test programs below.

seed, if present should be a decimal integer which will be used
as the seed to the random generator. Otherwise it will be seeded
using somewhat unpredictable operating system data.

blat64 blatil blatseed blat32v blat64v blatld
====== ====== ======== ======= ======= ======
More of same. See source for internals. blat32v and blat64v take an extra
optional blocksize parameter which is first.


The remaining programs are all for testing uniform bitstreams, such as those
produced by blatbytes.

Normally you would run them under the control of the mcp in the directory
above, but you can also run them seperately. Optional arguments are:
A numeric argument is the number of bytes to test. If absent, use all
data until end of file.
--progress means to write progress reports from time to time.

==============================================

One-sided P-values.
===================

The one-sided P-value is supposed to be a "single figure of merit" that
summarises a test result for people in a hurry. It doesn't usually encapsulate
all the information found by a particular test. Instead it usually focuses on
the most common or most serious failure modes. Some tests produce several
other results before the P-value, and these are worth reading and
understanding if you really want to know what's going on. In many cases the
actual tests are two-sided. But the P-value reported will always be one-sided.
Some are not very accurate, but should be close enough to support the rough
guide to interpretation below.

Interpretation of one-sided P-values.

P > 0.1 : This P-value is completely ok. Don't try to compare
	P-values in this range. "0.9 is higher than 0.4 so 0.9 must
	be better." Wrong. "0.5 is better than 0.999 because 0.5 is
	right in the middle of the range and therefore more random."
	Wrong. Each one-sided P-value test was designed to return an
	almost arbitrary value P > 0.1 if no bad behaviour was detected.
	If P > 0.1, you know no bad behaviour was detected, and you shouldn't
	try to interpret the result as anything more than that.
	Of course, a good P value like this doesn't prove the generator is
	good. It is always possible trouble will be spotted with a
	different test, or more data.

P <= 0.1 : There might be some cause for concern. In this range,
	the lower P is, the worse it is.

0.001 < P <= 0.1 : Not really proof that anything is wrong, but reason for
	vague suspicion. Further testing with a different random seed
	and perhaps larger data size is recommended. If a generator ALWAYS
	produces P values in this range or lower for a particular test,
	after many runs with different seeds, then it's probably broken.

1.0e-6 < P <= 0.001 : Like above, but moreso. Here you should start to worry.

1.0e-12 < P <= 1.0e-6 : There is a very remote possiblility that the
	generator is ok, but you should be seriously worried. After you
	see a P value this low, you can only rehabilitate the generator
	if you run the test that failed for many different seeds and
	it repeatedly produces good P-values with no more seriously bad ones.

P <= 1.0e-12 : You are most unlikely to see a good generator do this on a
	good test even once in your lifetime. You would be safe to reject any
	generator that does this on a properly calibrated test.
