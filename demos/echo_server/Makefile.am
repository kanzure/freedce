## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

IDL_FLAGS=-I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include $(DCETHREADINCLUDES)

install-strip:	install

bin_PROGRAMS=echo_server echo_client

echo_server_SOURCES=misc.c echo_server.c
echo_client_SOURCES=misc.c echo_client.c

noinst_HEADERS=misc.h echo.idl echo.acf

CLEANFILES=echo.h echo_sstub.o echo_cstub.o

libs=$(top_builddir)/dcelib/libdcerpc.la \
	$(top_builddir)/uuid/libuuid.la -lstdc++

echo_server_LDADD=echo_sstub.o $(libs)
echo_client_LDADD=echo_cstub.o $(libs)

echo_server.c: echo.h

# XXX ooo, baad.  major hack.  _must_ write a cross-dceidl for mingw32.
echo.h echo_sstub.o echo_cstub.o: echo.idl
	$(IDL)                 -cc_cmd "$(CC) -D_BSD $(IDL_CFLAGS) -c" \
	                -I/opt/dce-win32/include/ \
		$(top_srcdir)/demos/echo_server/echo.idl

