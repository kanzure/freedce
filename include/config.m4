AC_OUTPUT(include/Makefile)
AC_OUTPUT(include/dce/Makefile)

# Create symlinks for os and cpu dependent files

if test "x$target_os" = x; then
	echo "error: target_os not set $target_os"
	exit 1;
fi
AC_MSG_RESULT([Generating os dependent symlinks for $target_os])
if test ! -d include/dce/$target_os; then
	echo "error: operating system $target_os not supported"
	exit 1;
fi
osdepheaders=`cd include/dce/$target_os && echo *.h`
for header in $osdepheaders ; do
	if test "x$target_os/$header" = "xinclude/dce/$header"; then
		echo "error: source and target location for os dependend"
		echo "       include sources are the same"
		exit 1;
	else
		ln -sf $target_os/$header include/dce/$header
	fi
done;
unset osdepheaders

if test "x$target_cpu" = x; then
	echo "error target_cpu not set $target_cpu"
	exit 1;
fi
AC_MSG_RESULT([Generating cpu dependent symlinks for $target_cpu])
if test ! -d include/dce/$target_cpu; then
	echo "error: architecture $target_cpu not supported"
	exit 1;
fi
cpudepheaders=`cd include/dce/$target_cpu && echo *.h`
for header in $cpudepheaders ; do
	if test "x$target_cpu/$header" = "xinclude/dce/$header"; then
		echo "error: source and target location for cpu dependend"
		echo "       include sources are the same"
		exit 1;
	else
		ln -sf $target_cpu/$header include/dce/$header
	fi
done;
unset cpudepheaders

