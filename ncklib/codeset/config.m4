AC_OUTPUT(ncklib/codeset/Makefile)

RPC_ARG_DEFINE(codeset, BUILD_CODESET, no,Support for the codeset registry (untested))
AM_CONDITIONAL(BUILD_CODESET, test x${rpc_arg_codeset} = xyes)



