Test suite for SGI STL implementation

Boris P. Fomitchev <fbp@metabyte.com>

Last updated : Nov 14, 1997

----------------------------------------------------------------------------

Preface

One of the problems one is faced when deciding whether using STL or not is
the question of portability and reliability. It's not a rare case when
compiler begins to crash when template constructs get too complex. While
it's not possible to predict such effects on arbitrary code, it is often
useful to test basic STL compatibility of the compiler. That's what this
testsuite is for. It don't use too complex construct with STL items. But it
do instantiate about every item to see if it works properly.

----------------------------------------------------------------------------

Genesis

This testsuite is derived from Cygnus Solutions STL testsuite, which is
based on ObjectSpace STL examples. The changes that have been made mostly
involve restructuring. You can run a single short test for particular STL
construct , or try to compile them all and link to single executable. You
may also test if your compiler can handle extremely long source files by
compiling a single source including all others.

----------------------------------------------------------------------------

Platforms

Makefiles for gcc, SUNPro, Borland, Visual C++ compilers are provided with
the suite. Look for .mak files in the distribution. It should be not
difficult to adjust one of them to your compiler.

----------------------------------------------------------------------------

Trying it out

After unpacking, edit appropriate makefile to fit your compiler and include
directories . After you've done, try "make check". This target is output
(stl_test.out) of single executable containing all the tests. Compare it
with stl_test.exp output (or stl_test.rand.exp, see below). 
There should be no differences. If some test fails
to compile, you may try "make test_name.out" to produce single test
executable and run it.

----------------------------------------------------------------------------

Expected differences

As many tests use pseudo-random generators, you may get differences
in test output. 
Basically, there are 2 random generator scemes used :
via rand() function : expected result in stl_test.rand.exp
via lrand48() function : expected result in stl_test.exp.

System-dependent notices:
 If you are using STLport on OS390 machine, you should compare with stl_test.ibm390.exp.

 Linux libc uses different random generator which doesn't match any of the above. Be prepared.
 
map1_test : some compilers don't zero-initialize data when builtin-type default constructor called, thus you may see some garbage instead of 0 in the output. 
