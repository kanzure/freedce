## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

## included depending on configure switches

noinst_LTLIBRARIES=libnaf.la

common_files=ipnaf.c twr_ip.c

if NAF_IP
if TARGET_OS_LINUX
osdep_files=ipnaf_linux.c
endif
endif

libnaf_la_SOURCES=$(common_files) $(osdep_files)
