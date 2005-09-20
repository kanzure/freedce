## Process this file with automake to produce Makefile.in
# Makefile for the UUID library and tools

include $(top_srcdir)/include.mk

# any libraries specified will be listed as dependencies; avoid this
# as is means that a loop is present when we build libdcerpc
LIBS += -lkernel32 -lrpcrt4

# Build the UUID shared library
lib_LTLIBRARIES = libuuid.la
if TARGET_OS_WIN32
libuuid_la_SOURCES = uuid.c uuid-cpp.cpp uuidsys.c get_ether_addr.c win32timeofday.c win32uuid.c
else
libuuid_la_SOURCES = uuid.c uuid-cpp.cpp uuidsys.c get_ether_addr.c 
endif
libuuid_la_LDFLAGS=-lstdc++

noinst_HEADERS= uuid_i.h uuid.hpp

# Build the UUID generation tool
bin_PROGRAMS = uuid
uuid_SOURCES = uuidtool.cpp
uuid_LDADD = libuuid.la


