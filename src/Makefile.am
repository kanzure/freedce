## Process this file with automake to produce Makefile.in

AM_CFLAGS= -Wall -W -Werror -pipe
DEFS = -DCOMPILING_DCETHREADS -D_GNU_SOURCE -D_REENTRANT
INCLUDES=-I../include

#  Build the exception/thread library
lib_LTLIBRARIES = libdcethreads.la
libdcethreads_la_LDFLAGS = -version-info 2:1:0 -Wl,--version-script,./Versions
libdcethreads_la_SOURCES = \
    exc_handling.c \
    pthd4_libc_wrapers.c \
    pthread_dce_atfork.c \
    pthread_dce.c \
    pthread_dceexc.c

noinst_HEADERS = pthread_dce_atfork.h

EXTRA_DIST = Versions
