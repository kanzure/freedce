## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

IDLSTUBS=netlogon_sstub.o
IDLFLAGS+=-client none $(DCEINCLUDES)

libexec_PROGRAMS=netlogond
netlogond_SOURCES=netlogon_server.c
noinst_HEADERS=
netlogond_LDADD=$(IDLSTUBS) $(top_srcdir)/lib/netlogon/libnetlogon.la -ldcerpc
netlogond_LDFLAGS=

CLEANFILES=$(IDLSTUBS)

netlogon_sstub.o: $(top_srcdir)/include/netlogon.idl
	$(IDL) $(IDLFLAGS) $(top_srcdir)/include/netlogon.idl
	@rm -f netlogon.h

