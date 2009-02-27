## Process this file with automake to produce Makefile.in
# Build the lib before the tests
SUBDIRS = include src tests

EXTRA_DIST = acconfig.h

## 06.03.2003 schuetzk:   added tgz and rpm build rules
REL_VERSION=$(shell cat $(top_srcdir)/RELEASE)
RPMBUILD=$(shell which 2>/dev/null rpmbuild||which 2>/dev/null rpm)
##RPMOPTIONS= --sign

.filelist:
	- rm .filelist
	find . -type f | grep -v "$(top_srcdir)/dist"	     | \
			 grep -v "CVS"  		     | \
			 grep -v "$(top_srcdir)/autom"       | \
			 sed "s/^/dcethreads-$(REL_VERSION)\//" > .filelist
	find . -type l | sed "s/^/dcethreads-$(REL_VERSION)\//" >> .filelist
	echo dcethreads-$(REL_VERSION)/.filelist >> .filelist
	
tgz: .filelist
	rm -f dcethreads-$(REL_VERSION)
	$(LN_S) . dcethreads-$(REL_VERSION)
	tar --owner=0 --group=0 -chzf dcethreads-$(REL_VERSION).tar.gz -T .filelist
	rm dcethreads-$(REL_VERSION)
	@echo "You now have dcethreads-$(REL_VERSION).tar.gz"

WORKDIR=$(shell pwd)

rpm: tgz
	mkdir -p $(WORKDIR)/dist/BUILD \
		 $(WORKDIR)/dist/RPMS \
		 $(WORKDIR)/dist/SOURCES \
		 $(WORKDIR)/dist/SPECS  \
		 $(WORKDIR)/dist/SRPMS \
		 $(WORKDIR)/dist/install
	[ -h $(WORKDIR)/dist/SOURCES/dcethreads-$(REL_VERSION).tar.gz ] || \
	 $(LN_S) ../../dcethreads-$(REL_VERSION).tar.gz \
	       dist/SOURCES/dcethreads-$(REL_VERSION).tar.gz
	if test $(WORKDIR)/dcethreads.spec.in -nt $(WORKDIR)/dist/SPECS/dcethreads.spec ; then \
	  sed -e "s/^\(Version:\).*/\1 $(REL_VERSION)/;" \
	      -e "s/^\(Packager:\).*/\1 $(USER)@$(HOSTNAME)/;" < dcethreads.spec.in \
	  > dist/SPECS/dcethreads.spec ; \
        fi
	$(RPMBUILD) -ba ${RPMOPTIONS}\
	   --define "_topdir $(WORKDIR)/dist" \
	   --define "buildroot $(WORKDIR)/dist/install" \
	   $(WORKDIR)/dist/SPECS/dcethreads.spec


.PHONY: rpm tgz


##
