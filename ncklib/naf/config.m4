AC_OUTPUT(ncklib/naf/Makefile)

RPC_ARG_DEFINE(afip, NAF_IP, yes,IP network address family support)
AM_CONDITIONAL(NAF_IP, test x${rpc_arg_afip} = xyes)
AM_CONDITIONAL(NAF_IP_STATIC, test x${rpc_arg_afip} = xstatic)
RPC_ARG_DEFINE(ifloopback, USE_LOOPBACK, yes,Include loopback interface support)
