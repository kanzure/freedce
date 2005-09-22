## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

## Transports are included depending on configure switches

SUBDIRS=cn dg

noinst_LTLIBRARIES=libtrans.la

if TARGET_OS_WIN32
libtrans_la_SOURCES=comsoc_bsd.c comsoc_win32.c comsoc_win32_sendrcv.c
else
libtrans_la_SOURCES=comsoc_bsd.c
endif

