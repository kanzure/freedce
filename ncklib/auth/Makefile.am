## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

if AUTH_NTLM
auth_ntlm=libauth_ntlm.la
libauth_ntlm_la_SOURCES=ntlmauth.c
endif

if AUTH_DUMMY
auth_none=libauth_none.la

libauth_none_la_SOURCES=noauth.c @AUTH_DUMMY_EXTRAS@
endif

lib_LTLIBRARIES=$(auth_none) $(auth_ntlm) 
