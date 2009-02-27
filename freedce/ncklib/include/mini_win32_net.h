typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

typedef u_int   SOCKET;


#ifndef FD_SETSIZE
#define FD_SETSIZE	64
#endif

/* shutdown() how types */
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

#ifndef _SYS_TYPES_FD_SET
/* fd_set may be defined by the newlib <sys/types.h>
 * if __USE_W32_SOCKETS not defined.
 */
#ifdef fd_set
#undef fd_set
#endif
typedef struct fd_set {
	u_int   fd_count;
	SOCKET  fd_array[FD_SETSIZE];
} fd_set;
#ifndef FD_CLR
#define FD_CLR(fd,set) do { u_int __i;\
for (__i = 0; __i < ((fd_set *)(set))->fd_count ; __i++) {\
	if (((fd_set *)(set))->fd_array[__i] == (fd)) {\
	while (__i < ((fd_set *)(set))->fd_count-1) {\
		((fd_set*)(set))->fd_array[__i] = ((fd_set*)(set))->fd_array[__i+1];\
		__i++;\
	}\
	((fd_set*)(set))->fd_count--;\
	break;\
	}\
}\
} while (0)
#endif
#ifndef FD_SET
/* this differs from the define in winsock.h and in cygwin sys/types.h */
#define FD_SET(fd, set) do { u_int __i;\
for (__i = 0; __i < ((fd_set *)(set))->fd_count ; __i++) {\
	if (((fd_set *)(set))->fd_array[__i] == (fd)) {\
		break;\
	}\
}\
if (__i == ((fd_set *)(set))->fd_count) {\
	if (((fd_set *)(set))->fd_count < FD_SETSIZE) {\
		((fd_set *)(set))->fd_array[__i] = (fd);\
		((fd_set *)(set))->fd_count++;\
	}\
}\
} while(0)
#endif
#ifndef FD_ZERO
#define FD_ZERO(set) (((fd_set *)(set))->fd_count=0)
#endif
#ifndef FD_ISSET
#define FD_ISSET(fd, set) win32_fd_isset((SOCKET)(fd), (fd_set *)(set))
#endif
#elif !defined (USE_SYS_TYPES_FD_SET)
#warning "fd_set and associated macros have been defined in sys/types.  \
    This may cause runtime problems with W32 sockets"
#endif /* ndef _SYS_TYPES_FD_SET */

/* The fd_set member is required to be an array of longs.  */
typedef long int __fd_mask;

/* Some versions of <linux/posix_types.h> define these macros.  */
#undef  __NFDBITS
#undef  __FDELT
#undef  __FDMASK
/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define __NFDBITS       (8 * sizeof (__fd_mask))
#define __FDELT(d)      ((d) / __NFDBITS)
#define __FDMASK(d)     ((__fd_mask) 1 << ((d) % __NFDBITS))

/* Sometimes the fd_set member is assumed to have this type.  */
typedef __fd_mask fd_mask;

/* Number of bits per word of `fd_set' (some code assumes this is 32).  */
# define NFDBITS                __NFDBITS

struct in_addr {
	union {
	struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
	struct { u_short s_w1,s_w2; } S_un_w;
	u_long S_addr;
	} S_un;
#define s_addr  S_un.S_addr
#define s_host  S_un.S_un_b.s_b2
#define s_net   S_un.S_un_b.s_b1
#define s_imp   S_un.S_un_w.s_w2
#define s_impno S_un.S_un_b.s_b4
#define s_lh    S_un.S_un_b.s_b3
};


struct sockaddr_in {
	short   sin_family;
	u_short sin_port;
	struct  in_addr sin_addr;
	char    sin_zero[8];
};

struct iovec {
	size_t iov_len;
	char *iov_base;
};

struct sockaddr {
	u_short sa_family;
	char    sa_data[14];
};


#ifndef WSABASEERR
#define WSABASEERR	10000
#define WSAEINTR	(WSABASEERR+4)
#define WSAEBADF	(WSABASEERR+9)
#define WSAEACCES	(WSABASEERR+13)
#define WSAEFAULT	(WSABASEERR+14)
#define WSAEINVAL	(WSABASEERR+22)
#define WSAEMFILE	(WSABASEERR+24)
#define WSAEWOULDBLOCK	(WSABASEERR+35)
#define WSAEINPROGRESS	(WSABASEERR+36) /* deprecated on WinSock2 */
#define WSAEALREADY	(WSABASEERR+37)
#define WSAENOTSOCK	(WSABASEERR+38)
#define WSAEDESTADDRREQ	(WSABASEERR+39)
#define WSAEMSGSIZE	(WSABASEERR+40)
#define WSAEPROTOTYPE	(WSABASEERR+41)
#define WSAENOPROTOOPT	(WSABASEERR+42)
#define WSAEPROTONOSUPPORT	(WSABASEERR+43)
#define WSAESOCKTNOSUPPORT	(WSABASEERR+44)
#define WSAEOPNOTSUPP	(WSABASEERR+45)
#define WSAEPFNOSUPPORT	(WSABASEERR+46)
#define WSAEAFNOSUPPORT	(WSABASEERR+47)
#define WSAEADDRINUSE	(WSABASEERR+48)
#define WSAEADDRNOTAVAIL	(WSABASEERR+49)
#define WSAENETDOWN	(WSABASEERR+50)
#define WSAENETUNREACH	(WSABASEERR+51)
#define WSAENETRESET	(WSABASEERR+52)
#define WSAECONNABORTED	(WSABASEERR+53)
#define WSAECONNRESET	(WSABASEERR+54)
#define WSAENOBUFS	(WSABASEERR+55)
#define WSAEISCONN	(WSABASEERR+56)
#define WSAENOTCONN	(WSABASEERR+57)
#define WSAESHUTDOWN	(WSABASEERR+58)
#define WSAETOOMANYREFS	(WSABASEERR+59)
#define WSAETIMEDOUT	(WSABASEERR+60)
#define WSAECONNREFUSED	(WSABASEERR+61)
#define WSAELOOP	(WSABASEERR+62)
#define WSAENAMETOOLONG	(WSABASEERR+63)
#define WSAEHOSTDOWN	(WSABASEERR+64)
#define WSAEHOSTUNREACH	(WSABASEERR+65)
#define WSAENOTEMPTY	(WSABASEERR+66)
#define WSAEPROCLIM	(WSABASEERR+67)
#define WSAEUSERS	(WSABASEERR+68)
#define WSAEDQUOT	(WSABASEERR+69)
#define WSAESTALE	(WSABASEERR+70)
#define WSAEREMOTE	(WSABASEERR+71)
#define WSAEDISCON	(WSABASEERR+101)
#define WSASYSNOTREADY	(WSABASEERR+91)
#define WSAVERNOTSUPPORTED	(WSABASEERR+92)
#define WSANOTINITIALISED	(WSABASEERR+93)
#define WSAHOST_NOT_FOUND	(WSABASEERR+1001)
#define WSATRY_AGAIN	(WSABASEERR+1002)
#define WSANO_RECOVERY	(WSABASEERR+1003)
#define WSANO_DATA	(WSABASEERR+1004)

/* WinSock2 specific error codes */
#define WSAENOMORE	(WSABASEERR+102)
#define WSAECANCELLED	(WSABASEERR+103)
#define WSAEINVALIDPROCTABLE	(WSABASEERR+104)
#define WSAEINVALIDPROVIDER	(WSABASEERR+105)
#define WSAEPROVIDERFAILEDINIT	(WSABASEERR+106)
#define WSASYSCALLFAILURE	(WSABASEERR+107)
#define WSASERVICE_NOT_FOUND	(WSABASEERR+108)
#define WSATYPE_NOT_FOUND	(WSABASEERR+109)
#define WSA_E_NO_MORE	(WSABASEERR+110)
#define WSA_E_CANCELLED	(WSABASEERR+111)
#define WSAEREFUSED	(WSABASEERR+112)

#endif

#ifdef CAUSES_PROBLEMS
#define SOMAXCONN      0x7fffffff /* (5) in WinSock1.1 */
#else
#define SOMAXCONN      5
#endif

#define SOL_SOCKET      0xffff

#if ! (defined (__INSIDE_CYGWIN__) || defined (__INSIDE_MSYS__))
#define IP_OPTIONS      1
#define SO_DEBUG        1
#define SO_ACCEPTCONN   2
#define SO_REUSEADDR    4
#define SO_KEEPALIVE    8
#define SO_DONTROUTE    16
#define SO_BROADCAST    32
#define SO_USELOOPBACK  64
#define SO_LINGER       128
#define SO_OOBINLINE    256
#define SO_DONTLINGER   (u_int)(~SO_LINGER)
#define SO_SNDBUF       0x1001
#define SO_RCVBUF       0x1002
#define SO_SNDLOWAT     0x1003
#define SO_RCVLOWAT     0x1004
#define SO_SNDTIMEO     0x1005
#define SO_RCVTIMEO     0x1006
#define SO_ERROR        0x1007
#define SO_TYPE 0x1008
#endif /* ! (__INSIDE_CYGWIN__ || __INSIDE_MSYS__) */

#define IOCPARM_MASK    0x7f
#define IOC_VOID        0x20000000
#define IOC_OUT 0x40000000
#define IOC_IN  0x80000000
#define IOC_INOUT       (IOC_IN|IOC_OUT)

#if ! (defined (__INSIDE_CYGWIN__) || defined (__INSIDE_MSYS__))
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))
#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define FIONBIO _IOW('f', 126, u_long)
#endif /* ! (__INSIDE_CYGWIN__ || __INSIDE_MSYS__) */

#define FIONREAD        _IOR('f', 127, u_long)
#define FIOASYNC        _IOW('f', 125, u_long)
#define SIOCSHIWAT      _IOW('s',  0, u_long)
#define SIOCGHIWAT      _IOR('s',  1, u_long)
#define SIOCSLOWAT      _IOW('s',  2, u_long)
#define SIOCGLOWAT      _IOR('s',  3, u_long)
#define SIOCATMARK      _IOR('s',  7, u_long)


#define IPPROTO_TCP     6
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define TCP_NODELAY     0x0001
#define AF_INET 2

