AC_OUTPUT(ncklib/transport/Makefile)
AC_OUTPUT(ncklib/transport/cn/Makefile)
AC_OUTPUT(ncklib/transport/dg/Makefile)

dnl Check which transports to include
RPC_ARG_DEFINE(ncacn, PROT_NCACN, yes,Support connection based transports)
AM_CONDITIONAL(PROT_NCACN, test x${rpc_arg_ncacn} = xyes)

RPC_ARG_DEFINE(ncadg, PROT_NCADG, yes,Support connectionless transports)
AM_CONDITIONAL(PROT_NCADG, test x${rpc_arg_ncadg} = xyes)


