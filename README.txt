test-clone :: test cases for clone pruning

This file will build on Linux systems with
GCC 14 or higher and x86_64 or aarch64 
architectures.

To build the test cases, run 'make'

The output will be:

  test-clone-ARCH-prune

    This file has two implementations
    of the scale_samples function, which
    are effectively identical.

  test-clone-ARCH-noprune

    This binary has two implementations
    of the scale_sample function, which
    are significantly different. 

In both cases, ARCH will be replaced with
the string 'aarch64' or 'x86' according to
the system on which the build was performed.

To enable GCC diagnostic dumps, define the 
DUMP_ALL variable to have a non-null value.
See the comments in the Makefile.

-Chris Tyler 2024-11-20 - Version 0.1

