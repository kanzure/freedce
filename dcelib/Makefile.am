## Process this file with automake to produce Makefile.in

include $(top_srcdir)/include.mk
CFLAGS+=-D_POSIX_C_SOURCE=1 

# Build the shared library
lib_LTLIBRARIES = libdcerpc.la
libdcerpc_la_LDFLAGS = -version-info 0:1
libdcerpc_la_SOURCES=dummyfuncs.c

libdcerpc_la_LIBADD=../ncklib/libnck.la ../idllib/libidl.la


