## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

## included depending on configure switches

noinst_LTLIBRARIES=libauth.la

if AUTH_DUMMY
libauth_la_SOURCES=noauth.c
 if PROT_NCACN
libauth_la_SOURCES+= noauthcn.c
 endif
 if PROT_NCADG
libauth_la_SOURCES+= noauthdg.c
 endif
endif

