## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

SUBDIRS=com rpc transport auth naf codeset

EXTRA_DIST=ncklib.mk

# ensure all the files in the include dir get included in a dist.
# we could use `EXTRA_DIST=include' but automake 1.4 has a bug in the rule it
# generates for handling directories...
dist-hook:
	cp -pr include $(distdir)

idlobjs=mgmt_cstub.lo mgmt_sstub.lo

noinst_LTLIBRARIES=libnck.la

libnck_la_SOURCES=comfork.c ndrglob.c mgmt.c

libnck_la_LIBADD=com/libcom.la rpc/librpc.la transport/libtrans.la $(idlobjs)

mgmt_cstub.lo mgmt_sstub.lo: $(IDL_INCLUDE_DIR)/mgmt.idl
	$(IDL) $(NCK_IDLFLAGS) -cepv $<
	@rm -f mgmt.h


#libnck.la:	mgmt_cstub.lo mgmt_sstub.lo
