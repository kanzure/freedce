## Process this file with automake to produce Makefile.in
# Build this dir before the tests
SUBDIRS=. tests dce

INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/dce

CFLAGS= -g -Wall -W -O -pipe -Werror -D__BUILD_DCE_PTHREADS_LIBRARY -D__BUILD_DCE_PTHREADS_LIBRARY_STDALONE -DYIELD_AFTER_PTHREAD_CREATE

EXTRA_DIST=Versions

#  Build the exception/thread library
lib_LTLIBRARIES = libdcethreads.la
libdcethreads_la_SOURCES= exc_handling.c pthd4_libc_wrapers.c \
 pthread_dce.c pthread_dceexc.c

libdcethreads_la_LDFLAGS=-version-info 1:1 -Wl,--version-script,./Versions

