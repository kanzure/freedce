## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

IDLFLAGS+=-client none -server none $(DCEINCLUDES)

netlogon_idl=netlogon.idl

IDLHEADERS=$(netlogon_idl:.idl=.h)
CLEANFILES=$(IDLHEADERS)

include_HEADERS=$(IDLHEADERS) $(netlogon_idl) 

stamp-h:
	touch stamp-h

BUILT_SOURCES=$(IDLHEADERS)

%.h: %.idl
	$(IDL) $(IDLFLAGS) -no_mepv $<

