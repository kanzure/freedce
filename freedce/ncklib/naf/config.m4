AC_OUTPUT(ncklib/naf/Makefile)

RPC_ARG_DEFINE(afip, NAF_IP, yes,IP network address family support)
AM_CONDITIONAL(NAF_IP, test x${rpc_arg_afip} = xyes)

RPC_ARG_DEFINE(afuxd, NAF_UXD, yes,Unix Domain Socket network address family support)
AM_CONDITIONAL(NAF_UXD, test x${rpc_arg_afuxd} = xyes)

RPC_ARG_DEFINE(ifloopback, USE_LOOPBACK, yes,Include loopback interface support)
