## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

## included depending on configure switches

if NAF_IP
lib_LTLIBRARIES=libnaf_ip.la
libnaf_ip_la_SOURCES=ipnaf.c ipnaf_linux.c twr_ip.c

#noinst_LTLIBRARIES=libnaf.la
#libnaf_la_SOURCES=$(common_files) $(osdep_files)
endif
