INCLUDES+=-I$(top_srcdir)/ncklib/include -I$(top_srcdir)/ncklib/include/$(target_os)
AM_CFLAGS+=-DNCK -D_POSIX_C_SOURCE=1
