## Process this file with automake to produce Makefile.in

INCLUDES=-I$(top_srcdir)/include

EXTRA_PROGRAMS = test1 test2 test3 test3var test4 test5 
TESTS=test1 test2 test3 test3var test4 invert-test5 
EXTRA_DIST=inverttest

DEFS = -DCOMPILING_DCETHREADS -I. -I$(srcdir)

CLEANFILES=$(EXTRA_PROGRAMS) $(TESTS)

THREADLIB=$(top_builddir)/src/libdcethreads.la -ldl

# Create a symlink to our generic result inverting script.
# If it is possible to adjust the command line that "make check" runs, then a
# better way to do this is to just use the shell to invert the result.
invert-test5: test5
	-ln -sf $(srcdir)/inverttest $@

test1_SOURCES=test1.c
test1_LDADD = $(THREADLIB)

test2_SOURCES=test2.c
test2_LDADD = $(THREADLIB)

test3_SOURCES=test3.c
test3_LDADD = $(THREADLIB)

test3var_SOURCES=test3var.c
test3var_LDADD = $(THREADLIB)


test4_SOURCES=test4.c
test4_LDADD = $(THREADLIB)

test5_SOURCES=test5.c
test5_LDADD = $(THREADLIB)

#test_atfork1_SOURCES=test_atfork1.c
#test_atfork1_LDADD = $(THREADLIB)
