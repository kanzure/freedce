AC_OUTPUT(include/Makefile)
AC_OUTPUT(include/dce/Makefile)

# Create symlinks for os and cpu dependent files

AC_MSG_RESULT([Generating os dependent symlinks])
osdepheaders=`cd include/dce/$target_os && echo *.h`
for header in $osdepheaders ; do
	ln -sf $target_os/$header include/dce/$header
done;
unset osdepheaders

AC_MSG_RESULT([Generating cpu dependent symlinks])
cpudepheaders=`cd include/dce/$target_cpu && echo *.h`
for header in $cpudepheaders ; do
	ln -sf $target_cpu/$header include/dce/$header
done;
unset cpudepheaders

