The original tar file is `datalab-handout.tar` and the writeup is
`datalab.pdf`. My solution is in `datalab-handout-solution`.

When working on WSL on Windows, I need to remove the `-m32` flag
in the `Makefile` in order to compile. In addition, on WSL, running
`./btest` gives me full mark and running `./dlc bits.c` gives no
warning or violation, but `./driver.pl` gives me zero mark. This might
be because of the removal of the `-m32` flag in `Makefile`.

`Possible solution.txt` contains sites that are helpful when working on the lab.  

### Solution Analysis
