## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

noinst_LTLIBRARIES=libcom.la

libcom_la_SOURCES=comauth.c combind.c comcall.c comcthd.c comep.c comif.c \
 cominit.c cominit_ux.c comnaf.c comnet.c comnlsn.c comobj.c comp.c \
 comtwr.c comtwrflr.c comtwrref.c comutil.c

libcom_la_LIBADD=ep_cstub.lo

ep_cstub.lo:	$(IDL_INCLUDE_DIR)/ep.idl
	$(IDL) $(NCK_IDLFLAGS) -cepv -server none -cc_cmd '$(LIBTOOL) --mode=compile $(CC) -c ' $<
	@rm -f ep.h

