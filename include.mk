#include $(top_builddir)/config.mk

dce_includes=-I$(top_srcdir)/include $(DCETHREADINCLUDES)

CFLAGS= -g -Wall -W -O -pipe -Werror
INCLUDES+=$(dce_includes)

SUFFIXES=.idl

IDL=$(top_builddir)/idl/idl
IDL_INCLUDE_DIR=$(top_srcdir)/include/dce

IDLFLAGS=-cepv -client none -server none -I$(IDL_INCLUDE_DIR)/..
NCK_IDLFLAGS=-keep object -no_cpp -v -no_mepv -I$(IDL_INCLUDE_DIR)/.. -I$(top_srcdir)/include $(DCETHREADINCLUDES) $(TARGET_OS)

%.h: %.idl
	$(IDL) $(IDLFLAGS) -no_mepv $<

