/*
 * 
 * (c) Copyright 1991 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1991 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1991 DIGITAL EQUIPMENT CORPORATION
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *                 permission to use, copy, modify, and distribute this
 * file for any purpose is hereby granted without fee, provided that
 * the above copyright notices and this notice appears in all source
 * code copies, and that none of the names of Open Software
 * Foundation, Inc., Hewlett-Packard Company, or Digital Equipment
 * Corporation be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Neither Open Software Foundation, Inc., Hewlett-
 * Packard Company, nor Digital Equipment Corporation makes any
 * representations about the suitability of this software for any
 * purpose.
 * 
 */
/*
 */

#ifndef _COMSOC_SYS_H 
#define _COMSOC_SYS_H

#if 0
#include <sys/uio.h>
#include <sys/socket.h>
#endif

struct sockaddr;
struct fd_set;
struct timeval;

extern int win32_recvfrom(int,char*,int,int,struct sockaddr*,int*);
extern int win32_sendto(int,const char*,int,int,const struct sockaddr*,int);
extern int win32_socket_err(void);
extern int win32_fd_isset(int fd, struct fd_set *set);
extern int win32_select (int, struct fd_set *, struct fd_set *, struct fd_set *,
		struct timeval *);

extern int win32_socket(int,int,int);
extern int win32_close(int);
int  win32_accept(int,struct sockaddr*,int*);
int  win32_bind(int,const struct sockaddr*,int);
int  win32_connect(int,const struct sockaddr*,int);
int  win32_ioctlsocket(int,long, unsigned long *);
int  win32_getpeername(int,struct sockaddr*,int*);
int  win32_getsockname(int,struct sockaddr*,int*);
int  win32_getsockopt(int,int,int,void*,int*);
unsigned long  win32_inet_addr(const char*);
/*char * win32_inet_ntoa(struct in_addr);*/
int  win32_listen(int,int);
int  win32_recv(int,char*,int,int);
int  win32_send(int,const char*,int,int);
int  win32_setsockopt(int,int,int,const void*,int);
int  win32_shutdown(int,int);
int  win32_socksys_init(void);
short win32_htons(short);
short win32_ntohs(short);
int win32_get_ifaces_hnd(void**hnd);
void win32_get_iface(void* hnd, int idx,
		unsigned long *dwidx,
		unsigned long *addr,
		unsigned long *mask,
		unsigned long *bcast);
void win32_free_ifaces_hnd(void*);

#define socket_error win32_socket_err()

#include <comsoc_win32.h>



#endif /* _COMSOC_SYS_H */
