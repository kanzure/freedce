## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk


C_FLAGS=-DNCK -D_POSIX_SOURCE=1 -I$(top_srcdir)/ncklib/include -I$(top_srcdir)/ncklib/include/$(target_os)
CFLAGS+=$(C_FLAGS)
CXXFLAGS+=$(C_FLAGS)

IDLFLAGS=-keep c_source -v -no_mepv -cepv -client none -server none -I. -I$(IDL_INCLUDE_DIR)/.. $(DCETHREADINCLUDES)

bin_PROGRAMS=rpcd

rpcd_SOURCES=dsm.c dsm_unix.c rpcd.c rpcddb.c rpcdep.c \
 rpcdepdb.c rpcdsliv.c rpcdutil.c oxids.cxx

noinst_HEADERS=dsm_p.h rpcddb.h rpcdepdb.h rpcdepdbp.h rpcdp.h rpcdutil.h dsm.idl 

rpcd_LDADD=ep_sstub.o objex_sstub.o ../dcelib/libdcerpc.la ../uuid/libuuid.la
rpcd_LDFLAGS=-lstdc++

BUILT_SOURCES=dsm.h

CLEAN_FILES=dsm.h ep_sstub.o objex_sstub.o

objex_sstub.o: $(IDL_INCLUDE_DIR)/objex.idl
	$(IDL) -keep object -no_cpp -v -no_mepv -cepv -client none \
		-I$(IDL_INCLUDE_DIR)/../ \
		-cc_cmd "gcc -D_BSD $(IDL_CFLAGS) -c" \
		$(IDL_INCLUDE_DIR)/objex.idl
	@rm -f objex.h

ep_sstub.o: $(IDL_INCLUDE_DIR)/ep.idl
	$(IDL) -keep object -no_cpp -v -no_mepv -cepv -client none \
		-I$(IDL_INCLUDE_DIR)/../ \
		-cc_cmd "gcc -D_BSD $(IDL_CFLAGS) -c" \
		$(IDL_INCLUDE_DIR)/ep.idl
	@rm -f ep.h

dsm.h: dsm.idl

