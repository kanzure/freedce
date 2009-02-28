## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
ntsecincludedir=$(includedir)/nt

#IDLFLAGS=-keep all -cc_cmd "gcc -g -g -c" -I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include $(DCETHREADINCLUDES)
IDL_FLAGS=-client none -server none -cc_cmd "/bin/sh ../../libtool --mode=compile gcc -pipe -g -g -c" -I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include $(DCETHREADINCLUDES)


install-strip:	install

NTSECHEADERS=ntsec.h
ntsecinclude_HEADERS=$(NTSECHEADERS)
ntsecincludeuninstallfiles=$(NTSECHEADERS)

## override the normal automake rule so it doesn't try and copy
## linux-gnu/header.h into /opt/ntsec/include/ntsec/linux-gnu
install-ntsecincludeHEADERS: $(ntsecinclude_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(ntsecincludedir)
	@list='$(ntsecinclude_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(ntsecincludedir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(ntsecincludedir); \
	done

uninstall-ntsecincludeHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(ntsecincludeuninstallfiles)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(ntsecincludedir)/$$p; \
	done

dist-hook:
	@echo ============ dist hook =============
	@-cp -pr $(srcdir)/* $(distdir) 2>/dev/null


CLEANFILES=$(IDLHEADERS)

# remove the symlinks we put in during configure
distclean: distclean-am
	-rm -f $(osdepheaders) $(cpudepheaders)

all-local: $(IDLHEADERS)



noinst_HEADERS=ntsec.idl

CLEANFILES+=ntsec.h 

ntsec.h: ntsec.idl
	$(IDL) $(IDL_FLAGS) $(top_srcdir)/demos/ntsec/ntsec.idl


