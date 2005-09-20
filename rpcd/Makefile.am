## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk


C_FLAGS=-DNCK -D_POSIX_SOURCE=1 -I$(top_srcdir)/ncklib/include -I$(top_srcdir)/ncklib/include/$(target_os)
CFLAGS+=$(C_FLAGS)
CXXFLAGS+=$(C_FLAGS)
LIBS+= -lrpcrt4

IDLFLAGS+=-keep c_source -v -no_mepv -cepv -client none -server none -I. -I$(IDL_INCLUDE_DIR)/.. $(DCETHREADINCLUDES)

bin_PROGRAMS=rpcd

rpcd_SOURCES=dsm.c dsm_unix.c rpcd.c rpcddb.c rpcdep.c \
 rpcdepdb.c rpcdsliv.c rpcdutil.c oxids.cxx 

noinst_HEADERS=dsm_p.h rpcddb.h rpcdepdb.h rpcdepdbp.h rpcdp.h rpcdutil.h dsm.idl 

# removing objex due to problems with include/objex.idl - this
# should be a configure option [--enable-dcom]
if TARGET_OS_WIN32
rpcd_LDADD=ep_sstub.o objex_sstub.o ../dcelib/libdcerpc.la ../uuid/libuuid.la ../ncklib/naf/libnaf_ip.la
else
rpcd_LDADD=ep_sstub.o objex_sstub.o ../dcelib/libdcerpc.la ../uuid/libuuid.la
endif
#rpcd_LDADD=ep_sstub.o ../dcelib/libdcerpc.la ../uuid/libuuid.la
rpcd_LDFLAGS=-lstdc++ -lntdll 

BUILT_SOURCES=dsm.h

CLEAN_FILES=dsm.h ep_sstub.o objex_sstub.o
#CLEAN_FILES=dsm.h ep_sstub.o 

# XXX: manually added in /opt/dce-win32/include (uh-oh) to get
# round problem of objex_sstub cross-compile...
objex_sstub.o: $(IDL_INCLUDE_DIR)/objex.idl
	$(IDL) -keep object -no_cpp -v -no_mepv -cepv -client none \
		-I/opt/dce-win32/include/ \
		-I$(IDL_INCLUDE_DIR)/../ \
		-cc_cmd "$(CC) -D_BSD $(IDL_CFLAGS) -c" \
		$(IDL_INCLUDE_DIR)/objex.idl
	@rm -f objex.h

ep_sstub.o: $(IDL_INCLUDE_DIR)/ep.idl
	$(IDL) -keep object -no_cpp -v -no_mepv -cepv -client none \
		-I/opt/dce-win32/include/ \
		-I$(IDL_INCLUDE_DIR)/../ \
		-cc_cmd "$(CC) -D_BSD $(IDL_CFLAGS) -c" \
		$(IDL_INCLUDE_DIR)/ep.idl
	@rm -f ep.h

dsm.h: dsm.idl

