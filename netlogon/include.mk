-include $(top_builddir)/config.mk

CFLAGS= -g -Wall -W -pipe -I$(top_srcdir)/include $(DCEINCLUDES)

SUFFIXES=.idl

# XXX add path
IDL=/opt/dce/bin/dceidl
IDL_INCLUDE_DIR=$(top_srcdir)/include

LTCOMPILE=$(LIBTOOL) --mode=compile $(CC) $(DEFS) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
LINK=$(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

IDLFLAGS=$(IDL_CFLAGS) -I$(IDL_INCLUDE_DIR) -cc_cmd "$(LTCOMPILE) -c -x c"

MODULELDFLAGS=-module -avoid-version -export-dynamic

