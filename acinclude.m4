dnl $Revision: 1.1 $
AC_DEFUN(RPC_ARG_DEFINE,
[
AC_ARG_ENABLE($1,
dnl $1=option name
dnl $2=symbol name
dnl $3=if yes, then enable by default
dnl $4=help string
[  --enable-$1		$4 (default=$3)],
[
 case "${enableval}" in
	yes)
		AC_DEFINE($2, 1, [$4]);
		rpc_arg_$1=yes
		;;
	no)
		;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-$1);;
	esac
],
test x$3 = xyes && { rpc_arg_$1=yes;
AC_DEFINE($2, 1, [$4]) }
)
])

dnl Find out where the dcethreads library has been installed
AC_DEFUN(RPC_CHECK_LIBDIR,
dnl RPC_CHECK_LIBDIR(func, library, dirs,action-present,action-notpresent)
[AC_PREREQ([2.13])
AC_CACHE_CHECK([for -l$2 in one of $3], [rpc_libdir_$2],
[rpc_func_save_LIBS="$LIBS"
rpc_libdir_$2="no"
AC_TRY_LINK_FUNC([$1], [rpc_libdir_$2="none required"])
if test "$rpc_libdir_$2" = "no"; then
	LIBS="-l$2 $rpc_func_save_LIBS"
	AC_TRY_LINK_FUNC([$1], [rpc_libdir_$2="none required"])
	test "$rpc_libdir_$2" = "no" && for i in $3; do
		LIBS="-L$i -l$2 $rpc_func_save_LIBS"
		AC_TRY_LINK_FUNC([$1],
		[rpc_libdir_$2="-L$i"
		break])
	done
fi
LIBS="$rpc_func_save_LIBS"])
if test "$rpc_libdir_$2" != "no"; then
	LIBS="-l$2 $LIBS"
	test "$rpc_libdir_$2" = "none required" || {
		LDFLAGS="$rpc_libdir_$2 $LDFLAGS"
		AC_MSG_RESULT([found in $rpc_libdir_$2])
	}
	$4
else	:
	$5
fi])

	

