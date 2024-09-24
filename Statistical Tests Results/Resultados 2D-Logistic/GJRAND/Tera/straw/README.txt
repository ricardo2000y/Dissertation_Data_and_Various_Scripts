This directory contains a number of pseudo-random number generators that
can be used in the same way as testunif/blatrand . However, most of the
ones in this directory have something seriously wrong with them.

I wrote these because it is boring (not to mention also poor testing
practice) to run the testunif tests over and over on really good
generators and have them always say the numbers are ok.

A typical test run is something like

cd ..
./straw/lcon64 | ./mcp --small

In comments in the source-code there may be notes on which tests the
program fails. Beware that these comments may be incomplete or out of
date. If you are brave or foolish enough to want to use these programs for
anything, first do your own testing and try to understand just how bad
they are.

"they not so much fly as plummet."
