AC_MSG_CHECKING(UUID library and tool requirements)
AC_MSG_RESULT()

AC_CHECK_FUNCS(gettimeofday socket)
AC_OUTPUT(uuid/Makefile)

