## Process this file with automake to produce Makefile.in
# Makefile for the UUID library and tools

include $(top_srcdir)/include.mk

# Build the UUID shared library
noinst_LTLIBRARIES = libuuid.la
libuuid_la_SOURCES = uuid.c uuid-cpp.cpp uuidsys.c get_ether_addr.c

noinst_HEADERS= uuid_i.h uuid.hpp

# Build the UUID generation tool
bin_PROGRAMS = uuid
uuid_SOURCES = uuidtool.cpp
uuid_LDADD = libuuid.la


