## Process this file with automake to produce Makefile.in
# Makefile for the DCOM library

include $(top_srcdir)/include.mk

# XXX -ldecrpc is not found!!!!!!!
libdcom_la_LDFLAGS = -version-info 0:1 -ldcerpc

if ENABLE_DCOM
# Build the shared library
lib_LTLIBRARIES = libdcom.la
libdcom_la_SOURCES=init.c
endif
