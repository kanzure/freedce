## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

IDL_FLAGS= -I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include $(DCETHREADINCLUDES)

install-strip:	install

bin_PROGRAMS=samr_server samr_client

samr_server_SOURCES=misc.c samr_server.c
samr_client_SOURCES=misc.c samr_client.c

noinst_HEADERS=misc.h samr.idl samr.acf

CLEANFILES=samr.h samr_sstub.o samr_cstub.o

libs=-L/opt/dce/lib -ldcerpc -luuid -lstdc++

samr_server_LDADD=samr_sstub.o $(libs)
samr_client_LDADD=samr_cstub.o $(libs)

samr_server.c: samr.h
samr.h samr_sstub.o samr_cstub.o: samr.idl
	$(IDL) $(IDL_FLAGS) $(top_srcdir)/demos/samr/samr.idl

