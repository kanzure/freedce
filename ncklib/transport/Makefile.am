## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

## Transports are included depending on configure switches

SUBDIRS=cn dg

noinst_LTLIBRARIES=libtrans.la

libtrans_la_SOURCES=comsoc_bsd.c

libtrans_la_LIBADD=cn/libtranscon.la dg/libtransdgram.la

