## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

IDL_FLAGS=-I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include $(DCETHREADINCLUDES)
IDLCC=-no_mepv -cc_cmd "$(CC) -D_BSD $(IDL_FLAGS) $(IDL_CFLAGS) -c" $(IDL_INCS)

libs=$(top_builddir)/dcelib/libdcerpc.la \
	$(top_builddir)/uuid/libuuid.la -lstdc++ 

all: client server

EXTRA_PROGRAMS=client server
TESTS=run_tests.sh
EXTRA_DIST=run_tests.sh

run_tests.sh:	$(EXTRA_PROGRAMS)

client_SOURCES=client.c 
server_SOURCES=server.c perf.c

client_LDADD=perf_cstub.o perfb_cstub.o perfc_sstub.o perfg.o perfg_cstub.o util.o $(libs)
server_LDADD=perf_sstub.o perfb_sstub.o perfc_cstub.o perfg.o perfg_sstub.o perfb.o util.o $(libs)

#BUILT_SOURCES=perf.h perfb.h perfc.h perfg.h
CLEANFILES=perf.h perfb.h perfc.h perfg.h server client \
           perf_sstub.o perfb_sstub.o perfc_sstub.o perfg_sstub.o \
           perf_cstub.o perfb_cstub.o perfc_cstub.o perfg_cstub.o 

server.c: perf.h perfb.h perfc.h perfg.h
client.c: perf.h perfb.h perfc.h perfg.h

perf_cstub.o perf_sstub.o perf.h:	perf.idl
	$(IDL) $(IDLCC) perf.idl

perfb_cstub.o perfb_sstub.o perfb.h: perfb.idl
	$(IDL) $(IDLCC) perfb.idl

perfc_cstub.o perfc_sstub.o perfc.h: perfc.idl
	$(IDL) $(IDLCC) perfc.idl

perfg_cstub.o perfg_sstub.o perfg.h: perfg.idl
	$(IDL) $(IDLCC) perfg.idl

