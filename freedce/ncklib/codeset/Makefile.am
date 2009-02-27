## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

if BUILD_CODESET
noinst_LTLIBRARIES=libcodeset.la

libcodeset_la_SOURCES=cs_s_eval.c cs_s_reg.c cs_s_stub.c

libcodeset_la_LIBADD=codesets_cstub.lo

codesets_cstub.lo: $(IDL_INCLUDE_DIR)/codesets.idl
	$(IDL) $(NCK_IDLFLAGS) -server none -cc_cmd '$(LIBTOOL) --mode=compile $(CC) -c ' $<
	@rm -f codesets.h
else
EXTRA_FILES=$(codeset_files)
endif
