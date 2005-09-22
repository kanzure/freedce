/*
 * 
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
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

/*
**
**  NAME:
**
**      comsoc_win32_sendrcv.c
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  The platform-specific portion of the internal network "socket" object
**  interface.  See abstract in "comsoc.h" for details.
**
**  For BSD 4.3 & 4.4 systems.  
**
*/

#include <commonp.h>
#include <com.h>
#include <dce/dce_win32mem.h>

#define HACK_DEBUG

#ifdef HACK_DEBUG
extern void print_data(char *data, int len);
#endif

/*
 * Macros for performance critical operations.
 */
void RPC_SOCKET_SENDMSG(
	rpc_socket_t sock,
	rpc_socket_iovec_p_t iovp,
	int iovlen,
	rpc_addr_p_t addrp,
	volatile int *ccp,
  	volatile rpc_socket_error_t *serrp
		)
{
	struct sockaddr *ad = NULL;
	size_t ad_len = 0;
	char *data;
	char *data_ptr;
	size_t data_len = 0;
	int i;

	for (i = 0; i < iovlen; i++)
	{
#ifdef HACK_DEBUG
		printf("%s: iov %d: %d\n", __FUNCTION__, i, (int)iovp[i].iov_len);
#endif
		data_len += iovp[i].iov_len;
	}

#ifdef HACK_DEBUG
	printf("%s: data len to be sent: %d\n", __FUNCTION__, data_len);
#endif

	data = sys_malloc(data_len+100);

	/* flatten the data before sending as a single block */
	data_ptr = data;
	for (i = 0; i < iovlen; i++)
	{
		memcpy(data_ptr, iovp[i].iov_base, iovp[i].iov_len);
		data_ptr += iovp[i].iov_len;
	}

#ifdef HACK_DEBUG
	printf("%s:\n", __FUNCTION__);
	/*print_data(data, data_len);*/
#endif

sendmsg_again:
	RPC_LOG_SOCKET_SENDMSG_NTR;
	if ((addrp) != NULL)
	{
		RPC_SOCKET_FIX_ADDRLEN(addrp);
		ad = ((struct sockaddr *) &(addrp)->sa);
		ad_len = (addrp)->len;
	}
	*(ccp) = win32_sendto ((int) sock, data, data_len, 0, ad, ad_len);
	*(serrp) = (*(ccp) == -1) ? win32_socket_err() : RPC_C_SOCKET_OK;
	RPC_LOG_SOCKET_SENDMSG_XIT;
	if (*(serrp) == RPC_C_SOCKET_EINTR)
	{
		goto sendmsg_again;
	}
	sys_free(data);
}

void RPC_SOCKET_RECVFROM
(
    rpc_socket_t        sock,
    byte_p_t            buf,        /* buf for rcvd data */
    int                 buflen,        /* len of above buf */
    rpc_addr_p_t        from,       /* addr of sender */
    volatile int                 *ccp,        /* returned number of bytes actually rcvd */
	 volatile rpc_socket_error_t *serrp
)
{
	int len;
recvfrom_again:
	if ((from) != NULL) RPC_SOCKET_FIX_ADDRLEN(from);
	RPC_LOG_SOCKET_RECVFROM_NTR;
	len = (from)->len;
	*(ccp) = win32_recvfrom ((int) sock, buf, buflen, 0,
			(struct sockaddr *) &((from)->sa), &len);
	*(serrp) = (*(ccp) == -1) ? win32_socket_err() : RPC_C_SOCKET_OK;
	RPC_LOG_SOCKET_RECVFROM_XIT;
	if ((from) != NULL) RPC_SOCKET_FIX_ADDRLEN(from);
	if (*(serrp) == RPC_C_SOCKET_EINTR)
	{
		goto recvfrom_again;
	}

#ifdef HACK_DEBUG
	printf("%s: %d\n", __FUNCTION__, *serrp);
	if ((*ccp) > buflen || (*ccp) < 0)
		printf("%s: weird return value %d\n", __FUNCTION__, *ccp);
	/*
	else if ((*serrp) == RPC_C_SOCKET_OK)
		print_data(buf, *ccp);
		*/
#endif
}

/* damn windows doesn't have recvmsg.
 * receive the message, then unpack the data into the iovec.
 * i don't care how much data was _actually_ received.
 * am happy to copy garbage.  just want something working.
 * nice is for wimps.
 */
void RPC_SOCKET_RECVMSG
(
    rpc_socket_t        sock,
    rpc_socket_iovec_p_t iovp,       /* array of bufs for rcvd data */
    int                 iovlen,    /* number of bufs */
    rpc_addr_p_t        addrp,       /* addr of sender */
    volatile int                 *ccp,        /* returned number of bytes actually rcvd */
	 volatile rpc_socket_error_t *serrp
)
{
	size_t data_len = 0;
	char *data;
	char *data_ptr;
	int i;

	for (i = 0; i < iovlen; i++)
	{
#ifdef HACK_DEBUG
		printf("%s: iov %d: %d\n", __FUNCTION__, i, (int)iovp[i].iov_len);
#endif
		data_len += iovp[i].iov_len;
	}

#ifdef HACK_DEBUG
	printf("%s: data len expected %d\n", __FUNCTION__, data_len);
#endif
	data = sys_malloc(data_len+100);
	RPC_SOCKET_RECVFROM(sock, data, data_len, addrp, ccp, serrp);

#ifdef HACK_DEBUG
	printf("%s: data received: %d\n", __FUNCTION__, *ccp);
#endif
	if ((*(serrp)) != RPC_C_SOCKET_OK)
	{
		sys_free(data);
		return;
	}

	data_ptr = data;
	for (i = 0; i < iovlen; i++)
	{
		memcpy(iovp[i].iov_base, data_ptr, iovp[i].iov_len);
		data_ptr += iovp[i].iov_len;
	}

	sys_free(data);
}

