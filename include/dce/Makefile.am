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
osdepsymlinks=$(osdepheaders:%=$(srcdir)/$(target_os))

cpudepheaders=marshall.h ndr_rep.h ndrtypes.h
cpudepsymlinks=$(cpudepheaders:%=$(srcdir)/$(target_cpu))

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

inteldirs=i486 i586 i686 i786

dist-hook:
	@echo ============ dist hook =============
	@-cp -pr $(srcdir)/* $(distdir) 2>/dev/null
	@echo removing symlinks from dist package
	-rm -rf $(inteldirs:%=$(distdir)/%)


CLEANFILES=$(IDLHEADERS) $(osdepheaders) $(cpudepheaders) $(inteldirs)

.PHONY: do-header-symlinks

all-local: do-header-symlinks $(IDLHEADERS)

$(CPUDEPHEADERS) $(cpudepsymlinks) $(osdepsymlinks): do-header-symlinks

do-header-symlinks:
	list='$(osdepheaders)'; for include in $$list; do \
		ln -sf $(srcdir)/$(target_os)/$$include $(srcdir)/$$include; \
		done;
	list='$(cpudepheaders)'; for include in $$list; do \
		ln -sf $(srcdir)/$(target_cpu)/$$include $(srcdir)/$$include; \
		done;
	list='$(inteldirs)'; for item in $$list; do \
		ln -sf $(srcdir)/i386 $$item; \
		done;

