## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
lsaincludedir=$(includedir)/nt

#IDLFLAGS=-keep all -cc_cmd "gcc -g -g -c" -I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include $(DCETHREADINCLUDES)
IDL_FLAGS=-cc_cmd "/bin/sh ../../libtool --mode=compile gcc -pipe -g -g -c" -I$(IDL_INCLUDE_DIR)/.. -I$(top_builddir)/include/dce -I$(top_builddir)/include $(DCETHREADINCLUDES)


install-strip:	install

#bin_PROGRAMS=lsa_client lsa_server 
#bin_PROGRAMS=lsa_client

LSAHEADERS=lsarpc.h
lsainclude_HEADERS=$(LSAHEADERS)
lsaincludeuninstallfiles=$(LSAHEADERS)

#lsa_server_SOURCES=lsa_server.c credentials.c smbhash.c
liblsarpc_la_SOURCES=lsarpc_lib.c 
lib_LTLIBRARIES=liblsarpc.la

## override the normal automake rule so it doesn't try and copy
## linux-gnu/header.h into /opt/lsarpc/include/lsarpc/linux-gnu
install-lsaincludeHEADERS: $(lsainclude_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(lsaincludedir)
	@list='$(lsainclude_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(lsaincludedir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(lsaincludedir); \
	done

uninstall-lsaincludeHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(lsaincludeuninstallfiles)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(lsaincludedir)/$$p; \
	done

dist-hook:
	@echo ============ dist hook =============
	@-cp -pr $(srcdir)/* $(distdir) 2>/dev/null



# remove the symlinks we put in during configure
distclean: distclean-am
	-rm -f $(osdepheaders) $(cpudepheaders)

all-local: $(IDLHEADERS)



noinst_HEADERS=lsarpc.idl lsarpc.acf

CLEANFILES=lsa.h lsa_sstub.o lsa_cstub.o lsa_sstub.c lsa_cstub.c

libs=-L/opt/dce/lib -ldcerpc -luuid -lstdc++

#lsa_server_LDADD=lsa_sstub.o $(libs)
#lsa_client_LDADD=lsa_cstub.o $(libs)
liblsarpc_la_LIBADD=lsarpc_cstub.lo

#lsa_server.c: lsarpc.h
#lsarpc.h lsa_sstub.o lsa_cstub.o: lsarpc.idl

lsarpc.h lsarpc_cstub.c lsarpc_cstub.lo: lsarpc.idl lsarpc.acf
	$(IDL) $(IDL_FLAGS) $(top_srcdir)/demos/lsa/lsarpc.idl


