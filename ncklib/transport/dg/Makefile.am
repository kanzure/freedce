## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

idlobjs=conv_cstub.lo convc_cstub.lo convc_sstub.lo

CFLAGS+=-I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include 

noinst_LTLIBRARIES=libtransdgram.la

if PROT_NCADG
libtransdgram_la_SOURCES= conv.c dg.c dgcall.c dgccall.c \
 dgccallt.c dgcct.c dgclive.c dgclsn.c dgexec.c dgfwd.c \
 dgglob.c dghnd.c dginit.c dglossy.c dglsn.c dgpkt.c dgrq.c \
 dgscall.c dgsct.c dgslive.c dgslsn.c dgsoc.c dgutl.c dgxq.c

libtransdgram_la_LIBADD=$(idlobjs)

conv_cstub.lo:	$(IDL_INCLUDE_DIR)/conv.idl
	$(IDL) $(NCK_IDLFLAGS) -cepv -server none -cc_cmd '$(LIBTOOL) --mode=compile $(CC) -c ' $<

convc_cstub.lo convc_sstub.lo convc_cstub.c convc_sstub.c: $(IDL_INCLUDE_DIR)/convc.idl
	$(IDL) $(NCK_IDLFLAGS) -cepv -cc_cmd '$(LIBTOOL) --mode=compile $(CC) -c ' $<

CLEANFILES=conv.h convc.h
else
libtransdgram_la_SOURCES=dummy.c
endif
