## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
#	This Makefile builds the IDL compiler

# configure/automake adds the dcethread library to everything that is built in
# this package.  Most things need it, but the idl compiler doesn't, so we
# remove the dcethread library from LIBS here.  You might not expect
# -ldcethread to stick during linking, but it does because it includes
# implementations of some io functions.
LIBS:=$(subst -ldcethread,,$(LIBS))

CFLAGS+=-DMIA

bin_PROGRAMS = idl

idl_SOURCES = acf_y.y acf_l.l nidl_y.y nidl_l.l astp_com.c astp_cpx.c \
	astp_gbl.c astp_sim.c astp_dmp.c checker.c chkichar.c command.c \
	driver.c errors.c files.c frontend.c getflags.c keywds.c main.c \
	message.c nametbl.c namdump.c propagat.c sysdep.c mtsbacke.c \
	ddbe.c ddspell.c cspeldcl.c cspell.c cstubmts.c hdgen.c clihamts.c \
	mtspipes.c irepgen.c irepscp.c ifspemts.c sstubmts.c comstmts.c \
	user_exc.c icharsup.c

noinst_HEADERS= acf.h ast.h astp.h astp_dmp.h backend.h be_pvt.h bedeck.h \
 checker.h chkichar.h clihamts.h clihandl.h command.h commstat.h cspeldcl.h \
 cspell.h cstubgen.h cstubmts.h ddbe.h default_msg.h driver.h dutils.h \
 errors.h fe_pvt.h files.h flex_bison_support.h frontend.h genpipes.h \
 getflags.h hdgen.h icharsup.h ifspec.h ir_pvt.h irep.h marshall.h \
 message.h mtsbacke.h mtspipes.h nametbl.h namtbpvt.h nidl.h nidlmsg.h \
 propagat.h sstubmts.h sysdep.h user_exc.h


BUILT_SOURCES=acf_y.c acf_l.c nidl_y.c nidl_l.c acf_y.h nidl_y.h

# Override the standard YACC/LEX rules so we can build two scanners into
# the idl compiler.

acf_y.c acf_y.h:	acf_y.y
	$(YLWRAP) "$(YACC)" $(srcdir)/acf_y.y y.tab.c acf_y.c y.tab.h acf_y.h -- $(AM_YFLAGS) $(YFLAGS) -p acf_yy -d

acf_l.c: acf_l.l
	$(YLWRAP) "$(LEX)" $(srcdir)/acf_l.l $(LEX_OUTPUT_ROOT).c acf_l.c -- $(AM_LFLAGS) $(LFLAGS) -Pacf_yy -o$(LEX_OUTPUT_ROOT).c

nidl_y.c nidl_y.h: nidl_y.y
	$(YLWRAP) "$(YACC)" $(srcdir)/nidl_y.y y.tab.c nidl_y.c y.tab.h nidl_y.h -- $(AM_YFLAGS) $(YFLAGS) -p nidl_yy -d

nidl_l.c: nidl_l.l
	$(YLWRAP) "$(LEX)" $(srcdir)/nidl_l.l $(LEX_OUTPUT_ROOT).c nidl_l.c -- $(AM_LFLAGS) $(LFLAGS) -Pnidl_yy -o$(LEX_OUTPUT_ROOT).c

# Ensure that YACC/LEX generated files go away during clean
CLEANFILES=acf_y.o acf_y.c acf_l.o acf_l.c acf_y.h nidl_y.o nidl_y.c nidl_l.o nidl_l.c nidl_y.h

