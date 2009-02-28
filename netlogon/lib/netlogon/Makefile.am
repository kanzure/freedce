## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

CFLAGS+=-DIMAGE_DIR=\"$(libdir)\"

noinst_LTLIBRARIES=libnetlogon.la

netlogon_idl=netlogon.idl lsarpc.idl
IDLSTUBS=$(netlogon_idl:.idl=_cstub.lo)

CLEANFILES=$(netlogon_idl:.idl=_cstub.cxx)

libnetlogon_la_SOURCES=credentials.c misc.c smbhash.c
#libnetlogon_la_LDFLAGS=$(IDLSTUBS)
#libnetlogon_la_DEPENDENCIES=$(IDLSTUBS)

netlogon_cstub.lo: $(top_srcdir)/include/netlogon.idl
	$(IDL) $(IDLFLAGS) -server none $<
	@rm -f netlogon.h

lsarpc_cstub.lo: $(top_srcdir)/include/lsarpc.idl
	$(IDL) $(IDLFLAGS) -server none $<
	@rm -f lsarpc.h

