AC_OUTPUT(ncklib/auth/Makefile)

RPC_ARG_DEFINE(ntlmsspauth, AUTH_NTLMSSP, no,Include NTMLSSP auth)
AM_CONDITIONAL(AUTH_NTLMSSP, test x${rpc_arg_ntlmsspauth} = xyes)

#if test "x${rpc_arg_ntlmsspauth}" = "xyes" ; then
#	AUTH_NTLMSSP_EXTRAS=""
#	test "x${rpc_arg_ncacn}" = "xyes" && AUTH_DUMMY_EXTRAS="$AUTH_NTLMSSP_EXTRAS ntlmsspauthcn.c"
#	AC_SUBST(AUTH_NTLMSSP_EXTRAS)
#fi

RPC_ARG_DEFINE(gssauth, AUTH_GSS, no,Include GSS-API authentication)
AM_CONDITIONAL(AUTH_GSS, test x${rpc_arg_gssauth} = xyes)

GSSAPI_DIRS="/usr/heimdal"

AC_ARG_WITH(gssapi-dir, [  --with-gssapi-dir   Specify where you have installed GSSAPI],
[GSSAPI_DIRS="$withval $GSSAPI_DIRS"])

if test "x${rpc_arg_gssauth}" = "xyes" ; then
	RPC_CHECK_LIBDIR(gss_wrap, gssapi, $GSSAPI_DIRS,,,)
#	RPC_CHECK_INCDIR(gssapi.h, gssapi, $GSSAPI_DIRS,,,)
#	GSSAPIINCLUDES="${rpc_incdir_gssapi+-I$rpc_incdir_gssapi}"
	GSSAPIINCLUDES="-I/usr/heimdal/include"
	AC_SUBST(GSSAPIINCLUDES)
fi

#	AUTH_GSS_EXTRAS=""
#	test "x${rpc_arg_ncacn}" = "xyes" && AUTH_DUMMY_EXTRAS="$AUTH_GSS_EXTRAS gssauthcn.c"
#	AC_SUBST(AUTH_GSS_EXTRAS)

RPC_ARG_DEFINE(dummyauth, AUTH_DUMMY, no,Include the DCE dummy auth service)
AM_CONDITIONAL(AUTH_DUMMY, test x${rpc_arg_dummyauth} = xyes)

if test "x${rpc_arg_dummyauth}" = "xyes" ; then

	AUTH_DUMMY_EXTRAS=""
	test "x${rpc_arg_ncacn}" = "xyes" && AUTH_DUMMY_EXTRAS="$AUTH_DUMMY_EXTRAS noauthcn.c"
	test "x${rpc_arg_ncadg}" = "xyes" && AUTH_DUMMY_EXTRAS="$AUTH_DUMMY_EXTRAS noauthdg.c"
	AC_SUBST(AUTH_DUMMY_EXTRAS)
fi

