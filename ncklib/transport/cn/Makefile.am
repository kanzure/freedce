## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

# Makefile for connection orientated transports

noinst_LTLIBRARIES=libtranscon.la

common_source=cnassoc.c cnbind.c cncall.c cncasgsm.c \
 cncassm.c cncclsm.c cncthd.c cnfbuf.c cnid.c cninit.c cnmgmt.c \
 cnnet.c cnp.c cnpkt.c cnrcvr.c cnsasgsm.c cnsassm.c cnsclsm.c \
 cnsm.c cnxfer.c

if PROT_NCACN
libtranscon_la_SOURCES=$(common_source)
else
libtranscon_la_SOURCES=dummy.c
endif
