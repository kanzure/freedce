## Process this file with automake to produce Makefile.in
# Build this dir before the tests
SUBDIRS=. tests dce

INCLUDES=-I$(top_srcdir)# -I$(top_srcdir)/dce

CFLAGS= -Wall -W -O2 -pipe -Werror# -DCOMPILING_DCETHREADS
DEFS = -DCOMPILING_DCETHREADS -I. -I$(srcdir)

EXTRA_DIST=Versions pthread_dce_atfork.h acconfig.h

#  Build the exception/thread library
lib_LTLIBRARIES = libdcethreads.la
libdcethreads_la_SOURCES= exc_handling.c pthd4_libc_wrapers.c \
 pthread_dce_atfork.c pthread_dce.c pthread_dceexc.c

libdcethreads_la_LDFLAGS=-version-info 2:1:0 -Wl,--version-script,./Versions

