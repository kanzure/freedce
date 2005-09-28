## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

CFLAGS+=-I$(top_srcdir)/ncklib/include
IDL_FLAGS=-I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include $(DCETHREADINCLUDES)

install-strip:	install

bin_PROGRAMS=echo_server echo_client

echo_server_SOURCES=misc.c echo_server.c
echo_client_SOURCES=misc.c echo_client.c

noinst_HEADERS=misc.h echo.idl echo.acf

CLEANFILES=echo.h echo_sstub.o echo_cstub.o

libs=$(top_builddir)/dcelib/libdcerpc.la \
	$(top_builddir)/uuid/libuuid.la -lstdc++

# ugh.
if TARGET_OS_WIN32
libs+=$(top_builddir)/ncklib/naf/libnaf_ip.la \
	$(top_builddir)/ncklib/transport/dg/libprot_ncadg.la  \
	$(top_builddir)/ncklib/transport/cn/libprot_ncacn.la  \
	$(top_builddir)/ncklib/com/libcom.la -lrpcrt4
endif


echo_server_LDADD=echo_sstub.o $(libs)
echo_client_LDADD=echo_cstub.o $(libs)

echo_server.c: echo.h

if TARGET_OS_WIN32
IDL_INCS= -I/opt/dce-win32/include/
endif

# XXX ooo, baad.  major hack.  _must_ write a cross-dceidl for mingw32.
echo.h echo_sstub.o echo_cstub.o: echo.idl
	$(IDL)                 -cc_cmd "$(CC) -D_BSD $(IDL_FLAGS) $(IDL_CFLAGS) -c" \
		$(IDL_INCS) \
		$(top_srcdir)/demos/echo_server/echo.idl

