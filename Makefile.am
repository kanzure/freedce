## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

# the "include" dir is listed twice, so that on the second pass
# it can use the idl compiler to generate headers for use when building the
# rest of the rpc code.

SUBDIRS=include idl include idllib uuid ncklib dcelib rpcd demos .

EXTRA_DIST=include.mk perf/*


