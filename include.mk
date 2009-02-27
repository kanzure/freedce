include $(top_builddir)/config.mk

dce_includes=-I$(top_srcdir)/include $(DCETHREADINCLUDES)

# DRAT.  DO_NOT_CLOBBER (gcc being too smart) no longer works
#CFLAGS= -g -Wall -W -O -pipe -Werror
AM_CFLAGS= -g -g -Wall -W -pipe

INCLUDES=$(dce_includes)

SUFFIXES=.idl

if TARGET_OS_WIN32
# assume cross-compiling for now: hardcode the damn path.  oh well.
IDL=/opt/dce/bin/dceidl
else
IDL=$(top_builddir)/idl/dceidl$(WIN32_PROG_PREFIX)
endif

IDL_INCLUDE_DIR=$(top_srcdir)/include/dce

IDLFLAGS=$(IDL_CFLAGS) -cepv -client none -server none -I$(IDL_INCLUDE_DIR)/..
NCK_IDLFLAGS=-keep object -no_cpp -v -no_mepv -I$(IDL_INCLUDE_DIR)/.. -I$(top_srcdir)/include $(DCETHREADINCLUDES) $(TARGET_OS) -cc_cmd '$(LIBTOOL) --mode=compile $(IDL_CC) -c $(IDL_CFLAGS) '

%.h: %.idl
	$(IDL) $(IDLFLAGS) -no_mepv $<

# Create default message strings from a msg file
#%_defmsg.h:	%.msg
#	echo $(RM) $(RMFLAGS) $@
#	echo $(SED) -e '/^\$$/d;/^$$/d;s/^[^ ]* /"/;s/$$/",/;' $< > $@

#%.cat:	%.msg
#	$(RM) $(RMFLAGS) $@
#	$(GENCAT) $@ $<
