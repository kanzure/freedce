## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
dceincludedir = $(includedir)/dce

commonidl= codesets.idl conv.idl convc.idl ep.idl \
		iovector.idl lbase.idl mgmt.idl nbase.idl ncastat.idl \
		ndrold.idl rpc.idl rpcbase.idl rpcpvt.idl rpcsts.idl rpctypes.idl \
		twr.idl uuid.idl

commonheaders=codesets_stub.h stubbase.h idl_es.h idlbase.h idlddefs.h \
	rpcexc.h

osdepheaders=dce_utils.h sec_authn.h dce.h dce_error.h

cpudepheaders=marshall.h ndr_rep.h ndrtypes.h

IDLHEADERS=$(commonidl:.idl=.h)
OSDEPHEADERS=$(osdepheaders:%=$(target_os)/%)
CPUDEPHEADERS=$(cpudepheaders:%=$(target_cpu)/%)

dceinclude_HEADERS=$(IDLHEADERS) $(OSDEPHEADERS) $(CPUDEPHEADERS) $(commonheaders) $(commonidl)
dceincludeuninstallfiles=$(commonidl) $(commonheaders) $(osdepheaders) $(cpudepheaders)

## override the normal automake rule so it doesn't try and copy
## linux-gnu/header.h into /opt/dce/include/dce/linux-gnu
install-dceincludeHEADERS: $(dceinclude_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(dceincludedir)
	@list='$(dceinclude_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(dceincludedir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(dceincludedir); \
	done

uninstall-dceincludeHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(dceincludeuninstallfiles)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(dceincludedir)/$$p; \
	done

dist-hook:
	@echo ============ dist hook =============
	@-cp -pr $(srcdir)/* $(distdir) 2>/dev/null


CLEANFILES=$(IDLHEADERS)

# remove the symlinks we put in during configure
distclean: distclean-am
	-rm -f $(osdepheaders) $(cpudepheaders)

all-local: $(IDLHEADERS)



