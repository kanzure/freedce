## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

if NAF_UXD
naf_uxd=libnaf_uxd.la
libnaf_uxd_la_SOURCES=uxdnaf.c uxdnaf_linux.c twr_uxd.c
endif

if NAF_IP
naf_ip=libnaf_ip.la
libnaf_ip_la_SOURCES=ipnaf.c ipnaf_linux.c twr_ip.c
endif

lib_LTLIBRARIES=$(naf_ip) $(naf_uxd)
