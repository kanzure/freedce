RPC_ARG_DEFINE(idldumpers, DUMPERS, no,[Enable DUMPERS for debugging dceidl])
AC_OUTPUT(idl/Makefile)
AC_OUTPUT(idl/sysdep.h)

dnl IDL needs to set these flags when it compiles stuff
IDL_CFLAGS="-D_GNU_SOURCE -D_REENTRANT"
AC_SUBST(IDL_CFLAGS)

dnl For DCOM, default to C++ compiler for now
if test "x$ENABLE_DCOM" = "xyes" ; then
	IDL_CC="$CXX"
else
	IDL_CC="$CC"
fi
AC_SUBST(IDL_CC)

dnl If using GCC, we need to add -x c-header to force it to treat idl files
dnl as C headers; otherwise it tries to do something wierd...
if test "x${ac_cv_prog_gcc}" = "xyes" ; then
	IDL_CPP="$CPP -x c-header"
else
	IDL_CPP=$CPP
fi
AC_SUBST(IDL_CPP)

