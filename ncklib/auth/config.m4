AC_OUTPUT(ncklib/auth/Makefile)

RPC_ARG_DEFINE(ntlmauth, AUTH_NTLM, yes,Include NTMLSSP auth)
AM_CONDITIONAL(AUTH_NTLM, test x${rpc_arg_ntlmauth} = xyes)


RPC_ARG_DEFINE(dummyauth, AUTH_DUMMY, no,Include the DCE dummy auth service)
AM_CONDITIONAL(AUTH_DUMMY, test x${rpc_arg_dummyauth} = xyes)

if test "x${rpc_arg_dummyauth}" = "xyes" ; then

	AUTH_DUMMY_EXTRAS=""
	test "x${rpc_arg_ncacn}" = "xyes" && AUTH_DUMMY_EXTRAS="$AUTH_DUMMY_EXTRAS noauthcn.c"
	test "x${rpc_arg_ncadg}" = "xyes" && AUTH_DUMMY_EXTRAS="$AUTH_DUMMY_EXTRAS noauthdg.c"
	AC_SUBST(AUTH_DUMMY_EXTRAS)

fi

