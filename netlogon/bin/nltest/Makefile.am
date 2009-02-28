## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

IDLSTUBS=netlogon_cstub.o
IDLFLAGS+=-server none $(DCEINCLUDES)

libexec_PROGRAMS=nltest
nltest_SOURCES=nltest.c
noinst_HEADERS=
nltest_LDADD=$(IDLSTUBS) $(top_srcdir)/lib/netlogon/libnetlogon.la -llsarpc -ldcerpc 
nltest_LDFLAGS=

CLEANFILES=$(IDLSTUBS) netlogon.h

nltest.c: netlogon.h

netlogon.h netlogon_cstub.o: $(top_srcdir)/include/netlogon.idl
	$(IDL) $(IDLFLAGS) $(top_srcdir)/include/netlogon.idl

