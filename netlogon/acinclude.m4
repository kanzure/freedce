dnl $Revision: 1.1 $

dnl Generate include/config.h.in as we go along
define(rpc_add_to_autoheader, [[
# is the macro already in the file?
if ! egrep -q \^\#undef\ $1 include/netlogonconfig.h.in ; then
cat >> include/netlogonconfig.h.in <<EOD
/* $2 */
#undef $1

EOD
fi
]])

define(rpc_orig_define, defn([AC_DEFINE]))
undefine([AC_DEFINE])
define(AC_DEFINE,
[
rpc_add_to_autoheader($1, $3)
rpc_orig_define($1,ifelse($#, 2, [$2], $#, 3, [$2], 1))
]
)
define(rpc_orig_define_unquoted, defn([AC_DEFINE_UNQUOTED]))
undefine([AC_DEFINE_UNQUOTED])
define(AC_DEFINE_UNQUOTED,
[
rpc_add_to_autoheader($1, $3)
rpc_orig_define_unquoted($1,ifelse($#, 2, [$2], $#, 3, [$2], 1),$3)
]
)




AC_DEFUN(RPC_ARG_DEFINE,
[
rpc_add_to_autoheader($2, $4)
AC_ARG_ENABLE($1,
dnl $1=option name
dnl $2=symbol name
dnl $3=if yes, then enable by default
dnl $4=help string
[  --enable-$1		$4 (default=$3)],
[
 case "${enableval}" in
	yes)
		AC_DEFINE($2, 1, [$4])
		rpc_arg_$1=yes
		;;
	no)
		;;
	*)
		AC_MSG_ERROR(bad value ${enableval} for --enable-$1)
		;;
	esac
],
if test "x$3" = "xyes" ; then
	rpc_arg_$1=yes;
	AC_DEFINE($2, 1, [$4])
fi
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
		LIBS="-L$i/lib -l$2 $rpc_func_save_LIBS"
		AC_TRY_LINK_FUNC([$1],
		[rpc_libdir_$2="$i/lib"
		break])
	done
fi
LIBS="$rpc_func_save_LIBS"])
if test "$rpc_libdir_$2" != "no"; then
	LIBS="-l$2 $LIBS"
	test "$rpc_libdir_$2" = "none required" || {
		LDFLAGS="-L$rpc_libdir_$2 $LDFLAGS"
		AC_MSG_RESULT([found in $rpc_libdir_$2])
	}
	$4
else	:
	$5
fi])

dnl Find out where the dcethreads includes has been installed
AC_DEFUN(RPC_CHECK_INCDIR,
dnl RPC_CHECK_LIBDIR(header, desc, dirs, action-present, action-notpresent)
[AC_PREREQ([2.13])
AC_CACHE_CHECK([for $2 header in one of $3], [rpc_incdir_$2],
[rpc_incdir_$2="no"
AC_CHECK_HEADER($1, [rpc_incdir_$2="none required"])
test "$rpc_incdir_$2" = "no" && for i in $3; do
	AC_CHECK_HEADER($i/include/$1,
		[rpc_incdir_$2="$i/include"
		break])
done])
if test "$rpc_incdir_$2" = "no"; then
	unset rpc_incdir_$2
	$5
else
	test "$rpc_incdir_$2" = "none required" || {
		AC_MSG_RESULT([found in $rpc_incdir_$2])
	}
	test "$rpc_incdir_$2" = "none required" && unset rpc_incdir_$2
	$4
fi])

dnl Find out if the compiler accepts __unused__
AC_DEFUN(RPC_C_UNUSED,
rpc_add_to_autoheader(__unused, Compiler attribute to mark param as unused)
[AC_CACHE_CHECK([for unused], ac_cv_rpc_c_inline,
[ac_cv_rpc_c_inline=no
for ac_kw in __attribute__\(\(unused\)\) __unused__ __unused; do
	AC_TRY_COMPILE(, [} int foo(int arg $ac_kw) {], [ac_cv_rpc_c_inline=$ac_kw; break])
done
])
case "$ac_cv_rpc_c_inline" in
	__unused | yes) ;;
	no) AC_DEFINE(__unused, ) ;;
	*)	 AC_DEFINE_UNQUOTED(__unused, $ac_cv_rpc_c_inline) ;;
esac
])
