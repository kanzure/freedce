## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

if AUTH_DUMMY
lib_LTLIBRARIES=libauth_none.la

libauth_none_la_SOURCES=noauth.c @AUTH_DUMMY_EXTRAS@
endif
