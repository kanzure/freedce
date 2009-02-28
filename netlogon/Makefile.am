## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk

SUBDIRS=include lib libexec 

EXTRA_DIST=include.mk buildconf

dist-hook:
	-chmod -R a+r $(distdir)
	$(TAR) choIf $(distdir).tar.bz2 $(distdir)

