AC_OUTPUT(ncklib/auth/Makefile)

RPC_ARG_DEFINE(dummyauth, AUTH_DUMMY, no,Include the DCE dummy auth service)
AM_CONDITIONAL(AUTH_DUMMY, test x${rpc_arg_dummyauth} = xyes)

