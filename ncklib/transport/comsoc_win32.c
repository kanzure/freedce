#ifdef HAVE_OS_WIN32
#include <winsock2.h>
#include <iphlpapi.h>

int win32_socket_close(int fd)
{
	return closesocket(fd);
}

BOOL win32_socksys_init(void)
{
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(1, 1);
        return WSAStartup(wVersionRequested, &wsaData);
}

void win32_socksys_close(void)
{
        WSACleanup();
}

int win32_recvfrom(int sock,char*data,int len,int flags,struct sockaddr* sa,
		int*sa_len)
{
	return recvfrom((SOCKET)sock, data, len, flags, sa, sa_len);
}

int win32_sendto(int sock,const char* data,int len,int flags,
		const struct sockaddr* sa,int sa_len)
{
	return sendto((SOCKET)sock,data,len,flags, sa,sa_len);
}

int win32_socket_err(void)
{
	return WSAGetLastError();
}

int win32_fd_isset(int fd, fd_set *set)
{
	return __WSAFDIsSet(fd, set);
}

int win32_select (int __nfds, fd_set *__restrict __readfds,
	fd_set *__restrict __writefds,
	fd_set *__restrict __exceptfds,
	struct timeval *__restrict __timeout)
{
	return select (__nfds, __readfds, __writefds, __exceptfds, __timeout);
}

int win32_socket(int domain,int type,int protocol)
{
	return (SOCKET) socket(domain,type,protocol);
}

int  win32_accept(int sock,struct sockaddr* sa,int* len)
{
	return accept((SOCKET)sock,sa, len);
}

int  win32_bind(int sock,struct sockaddr* sa,int len)
{
	return bind((SOCKET)sock,sa, len);
}
int  win32_close(int sock)
{
	return closesocket((SOCKET)sock);
}

int  win32_connect(int sock,struct sockaddr* sa,int len)
{
	return connect((SOCKET)sock,sa, len);
}

int  win32_ioctlsocket(SOCKET sock,long ioc,unsigned long *ret)
{
	return ioctlsocket((SOCKET)sock,ioc,ret);
}

int  win32_getpeername(int sock,struct sockaddr* sa,int* len)
{
	return getpeername((SOCKET)sock,sa, len);
}

int  win32_getsockname(int sock,struct sockaddr* sa,int* len)
{
	return getsockname((SOCKET)sock,sa, len);
}

int  win32_getsockopt(SOCKET sock,int p1,int p2,void* p3,int* p4)
{
	return getsockopt((SOCKET)sock,p1,p2,(char*)p3,p4);
}

unsigned long  win32_inet_addr(const char* a)
{
	return inet_addr(a);
}

char * win32_inet_ntoa(struct in_addr a)
{
	return inet_ntoa(a);
}
int  win32_listen(SOCKET sock,int p1)
{
	return listen((SOCKET)sock,p1);
}

int  win32_recv(SOCKET sock,char* p1,int p2,int p3)
{
	return recv((SOCKET)sock,p1,p2,p3);
}
int  win32_send(SOCKET sock,const char* p1,int p2,int p3)
{
	return send((SOCKET)sock,p1,p2,p3);
}
int  win32_setsockopt(SOCKET sock,int p1,int p2,const void* p3,int p4)
{
	return setsockopt((SOCKET)sock,p1,p2,(const char*)p3,p4);
}
int  win32_shutdown(SOCKET sock,int opt)
{
	return shutdown((SOCKET)sock,opt);
}

short win32_ntohs(short a)
{
	return ntohs(a);
}

short win32_htons(short a)
{
	return htons(a);
}

int win32_get_ifaces_hnd(void**hnd)
{
        PMIB_IPADDRTABLE pIPAddrTable;
        DWORD dwSize = 0;

        pIPAddrTable = (MIB_IPADDRTABLE*) HeapAlloc( GetProcessHeap(), 0,
			sizeof( MIB_IPADDRTABLE));

        // Make an initial call to GetIpAddrTable to get the
        // necessary size into the dwSize variable
        if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
			ERROR_INSUFFICIENT_BUFFER)
	{
		HeapFree( GetProcessHeap(), 0, pIPAddrTable );
		if (dwSize == 0)
			return 0;
		pIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
	}

	if (pIPAddrTable == NULL)
		return 0;

	if (GetIpAddrTable( pIPAddrTable, &dwSize, 0) != NO_ERROR)
	{
		HeapFree( GetProcessHeap(), 0, pIPAddrTable );
		return 0;
	}

	(*hnd) = (void*)pIPAddrTable;
	return pIPAddrTable->dwNumEntries;
}

void win32_get_iface(void* hnd, int idx, 
		unsigned long *dwidx,
		unsigned long *addr,
		unsigned long *mask, unsigned long *bcast)
{
        PMIB_IPADDRTABLE pIPAddrTable = (PMIB_IPADDRTABLE)hnd;
	if (dwidx != NULL)
		(*dwidx)  = pIPAddrTable->table[idx].dwIndex;
	if (addr != NULL)
		(*addr)  = pIPAddrTable->table[idx].dwAddr;
	if (mask != NULL)
		(*mask)  = pIPAddrTable->table[idx].dwMask;
	if (bcast != NULL)
		(*bcast) = pIPAddrTable->table[idx].dwBCastAddr;
}
void win32_free_ifaces_hnd(void* hnd)
{
	if (hnd)
		GlobalFree( hnd );
}

#endif
