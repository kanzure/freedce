include $(top_builddir)/config.mk

dce_includes=-I$(top_srcdir)/include $(DCETHREADINCLUDES)

CFLAGS= -g -Wall -W -O -pipe -Werror
INCLUDES=$(dce_includes)

SUFFIXES=.idl

IDL=$(top_builddir)/idl/dceidl
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
