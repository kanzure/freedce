## Process this file with automake to produce Makefile.in
include $(top_srcdir)/include.mk
include $(top_srcdir)/ncklib/ncklib.mk

if AUTH_DUMMY
auth_none=libauth_none.la
libauth_none_la_SOURCES=noauth.c @AUTH_DUMMY_EXTRAS@
endif

if AUTH_NTLMSSP
auth_ntlmssp=libauth_ntlmssp.la

libauth_ntlmssp_la_SOURCES=ntlmsspauth.c ntlmsspauthcn.c ntlmssp_api.c \
			parse_net.c parse_prs.c \
			parse_ntlmssp.c parse_misc.c \
			util.c sma.c pwd_cache.c \
			crc32.c md5.c md4.c hmacmd5.c smbdes.c smbencrypt.c
libauth_ntlmssp_la_LIBADD=/usr/local/samba/lib/libwinbindntauth.la
endif

if AUTH_GSS
auth_gss=libauth_gss.la
libauth_gss_la_SOURCES=gssauth.c gssauthcn.c util.c crc32.c
endif

lib_LTLIBRARIES=$(auth_none) $(auth_ntlmssp) $(auth_gss)

