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
**      comsoc.c
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  Veneer over the BSD socket abstraction not provided by the old sock_
**  or new rpc_{tower,addr}_ components.
**
**
*/

#include <commonp.h>
#include <com.h>
#include <comprot.h>
#include <comnaf.h>
#include <comp.h>
#include <fcntl.h>
/*#include <dce/cma_ux_wrappers.h>*/

/* ======================================================================== */

/*
 * What we think a socket's buffering is in case rpc__socket_set_bufs()
 * fails miserably.  The #ifndef is here so that these values can be
 * overridden in a per-system file.
 */

#ifndef RPC_C_SOCKET_GUESSED_RCVBUF
#  define RPC_C_SOCKET_GUESSED_RCVBUF    (4 * 1024)
#endif

#ifndef RPC_C_SOCKET_GUESSED_SNDBUF
#  define RPC_C_SOCKET_GUESSED_SNDBUF    (4 * 1024)
#endif

/*
 * Maximum send and receive buffer sizes.  The #ifndef is here so that
 * these values can be overridden in a per-system file.
 */

#ifndef RPC_C_SOCKET_MAX_RCVBUF     
#  define RPC_C_SOCKET_MAX_RCVBUF (32 * 1024)
#endif

#ifndef RPC_C_SOCKET_MAX_SNDBUF     
#  define RPC_C_SOCKET_MAX_SNDBUF (32 * 1024)
#endif

/* ======================================================================== */

/*
 * R P C _ _ S O C K E T _ O P E N
 *
 * Create a new socket for the specified Protocol Sequence.
 * The new socket has blocking IO semantics.
 *
 * (see BSD UNIX socket(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_open
#ifdef _DCE_PROTO_
(
    rpc_protseq_id_t    pseq_id,
    rpc_socket_t        *sock
)
#else
(pseq_id, sock)
rpc_protseq_id_t    pseq_id;
rpc_socket_t        *sock;
#endif
{
    RPC_LOG_SOCKET_OPEN_NTR;

    *sock = socket(
        (int) RPC_PROTSEQ_INQ_NAF_ID(pseq_id),
        (int) RPC_PROTSEQ_INQ_NET_IF_ID(pseq_id),
        (int) RPC_PROTSEQ_INQ_NET_PROT_ID(pseq_id));

    RPC_LOG_SOCKET_OPEN_XIT;
    return ((*sock == -1) ? errno : RPC_C_SOCKET_OK);
}

/*
 * R P C _ _ S O C K E T _ O P E N _ B A S I C
 *
 * A special version of socket_open that is used *only* by 
 * the low level initialization code when it is trying to 
 * determine what network services are supported by the host OS.
 */

PRIVATE rpc_socket_error_t rpc__socket_open_basic
#ifdef _DCE_PROTO_
(
    rpc_naf_id_t        naf,
    rpc_network_if_id_t net_if,
    rpc_network_protocol_id_t net_prot,
    rpc_socket_t        *sock
)
#else
(naf, net_if, net_prot, sock)
rpc_naf_id_t        naf;
rpc_network_if_id_t net_if;
rpc_network_protocol_id_t net_prot;
rpc_socket_t        *sock;
#endif
{
    *sock = socket((int) naf, (int) net_if, (int) net_prot);

    return ((*sock == -1) ? errno : RPC_C_SOCKET_OK);
}

/*
 * R P C _ _ S O C K E T _ C L O S E
 *
 * Close (destroy) a socket.
 *
 * (see BSD UNIX close(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_close
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock
)
#else
(sock)
rpc_socket_t        sock;
#endif
{
    rpc_socket_error_t  serr;

    RPC_LOG_SOCKET_CLOSE_NTR;
    serr = (close(sock) == -1) ? errno : RPC_C_SOCKET_OK;
    RPC_LOG_SOCKET_CLOSE_XIT;
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ B I N D
 *
 * Bind a socket to a specified local address.
 *
 * (see BSD UNIX bind(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_bind
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    rpc_addr_p_t        addr
)
#else
(sock, addr)
rpc_socket_t        sock;
rpc_addr_p_t        addr;
#endif
{
    rpc_socket_error_t  serr = EINVAL;
    unsigned32 status;
    rpc_addr_p_t temp_addr = NULL;
    boolean has_endpoint = false;
    int setsock_val = 1;

    RPC_LOG_SOCKET_BIND_NTR;

    /*
     * Check if the address has a well-known endpoint.
     */
    if (addr->rpc_protseq_id == RPC_C_PROTSEQ_ID_NCACN_IP_TCP)
    {
        unsigned_char_t *endpoint;

        rpc__naf_addr_inq_endpoint (addr, &endpoint, &status);

        if (status == rpc_s_ok && endpoint != NULL)
        {
            if (endpoint[0] != '\0')    /* test for null string */
                has_endpoint = true;

            rpc_string_free (&endpoint, &status);
        }
        status = rpc_s_ok;
    }

    /* 
     * If there is no port restriction in this address family, then do a 
     * simple bind. 
     */
    
    if (! RPC_PROTSEQ_TEST_PORT_RESTRICTION (addr -> rpc_protseq_id))
    {
        serr = 
            (bind(sock, (struct sockaddr *)&addr->sa, addr->len) == -1) ? 
		errno : RPC_C_SOCKET_OK;
#if defined(SOL_SOCKET) && defined(SO_REUSEADDR)
        if (serr == RPC_C_SOCKET_EADDRINUSE && has_endpoint)
        {
            if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                           &setsock_val, sizeof(setsock_val)) != -1)
            {
                serr = (bind(sock, (struct sockaddr *)&addr->sa, addr->len) == -1)
                    ? errno : RPC_C_SOCKET_OK;
            }
        }
#endif
    }                                   /* no port restriction */

    else                          
    {
        /* 
         * Port restriction is in place.  If the address has a well-known 
         * endpoint, then do a simple bind.
         */
        
        if (has_endpoint)
        {
            serr = (bind(sock, (struct sockaddr *)&addr->sa, addr->len) == -1)?
                errno : RPC_C_SOCKET_OK;
#if defined(SOL_SOCKET) && defined(SO_REUSEADDR)
            if (serr == RPC_C_SOCKET_EADDRINUSE)
            {
                if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                               &setsock_val, sizeof(setsock_val)) != -1)
                {
                    serr = (bind(sock, (struct sockaddr *)&addr->sa, addr->len) == -1)
                        ? errno : RPC_C_SOCKET_OK;
                }
            }
#endif
        }                               /* well-known endpoint */
        else
	{

	    unsigned_char_t *endpoint;
	    unsigned char c;

	    rpc__naf_addr_inq_endpoint (addr, &endpoint, &status);

	    c = endpoint[0];               /* grab first char */
	    rpc_string_free (&endpoint, &status);

	    if (c != '\0')       /* test for null string */
	    {
	        serr = (bind(sock, (struct sockaddr *)&addr->sa, addr->len) == -1)?
		    errno : RPC_C_SOCKET_OK;
	    }                               /* well-known endpoint */

	    else
	    {
	        /* 
	         * Port restriction is in place and the address doesn't have a 
	         * well-known endpoint.  Try to bind until we hit a good port, 
	         * or exhaust the retry count.
	         * 
	         * Make a copy of the address to work in; if we hardwire an 
	         * endpoint into our caller's address, later logic could infer 
	         * that it is a well-known endpoint. 
	         */
	    
	        unsigned32 i;
	        boolean found;
	    
	        for (i = 0, found = false; 
		     (i < RPC_PORT_RESTRICTION_INQ_N_TRIES (addr->rpc_protseq_id))
		     && !found;
		     i++)   
	        {
		    unsigned_char_p_t port_name;

		    rpc__naf_addr_overcopy (addr, &temp_addr, &status);

		    if (status != rpc_s_ok)
		    {
		        serr = RPC_C_SOCKET_EIO;
		        break;
		    }

		    rpc__naf_get_next_restricted_port (temp_addr -> rpc_protseq_id,
						   &port_name, &status);

		    if (status != rpc_s_ok)
		    {
		        serr = RPC_C_SOCKET_EIO;
		        break;
		    }
    
		    rpc__naf_addr_set_endpoint (port_name, &temp_addr, &status);

		    if (status != rpc_s_ok)
		    {
		        serr = RPC_C_SOCKET_EIO;
		        rpc_string_free (&port_name, &status);
		        break;
		    }

		    if (bind(sock, (struct sockaddr *)&temp_addr->sa, temp_addr->len) == 0)
		    {
		        found = true;
		        serr = RPC_C_SOCKET_OK;
		    }
		    else
		        serr = RPC_C_SOCKET_EIO;

		    rpc_string_free (&port_name, &status);
	        }                           /* for i */

	        if (!found)
	        {
		    serr = RPC_C_SOCKET_EADDRINUSE;
	        }
	    }                               /* no well-known endpoint */
        }				/* has endpoint */
    }                                   /* port restriction is in place */

    if (temp_addr != NULL)
        rpc__naf_addr_free (&temp_addr, &status);

    RPC_LOG_SOCKET_BIND_XIT;
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ C O N N E C T
 *
 * Connect a socket to a specified peer's address.
 * This is used only by Connection oriented Protocol Services.
 *
 * (see BSD UNIX connect(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_connect
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    rpc_addr_p_t        addr
)
#else
(sock, addr)
rpc_socket_t        sock;
rpc_addr_p_t        addr;
#endif
{
    rpc_socket_error_t  serr;

connect_again:
    RPC_LOG_SOCKET_CONNECT_NTR;
    serr = (connect (
                     (int) sock,
                     (struct sockaddr *) (&addr->sa),
                     (int) (addr->len))
            == -1) ? errno : RPC_C_SOCKET_OK;
    RPC_LOG_SOCKET_CONNECT_XIT;
    if (serr == EINTR)
    {
        goto connect_again;
    }
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ A C C E P T
 *
 * Accept a connection on a socket, creating a new socket for the new
 * connection.  A rpc_addr_t appropriate for the NAF corresponding to
 * this socket must be provided.  addr.len must set to the actual size
 * of addr.sa.  This operation fills in addr.sa and sets addr.len to
 * the new size of the field.  This is used only by Connection oriented
 * Protocol Services.
 * 
 * (see BSD UNIX accept(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_accept
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    rpc_addr_p_t        addr,
    rpc_socket_t        *newsock
)
#else
(sock, addr, newsock)
rpc_socket_t        sock;
rpc_addr_p_t        addr;
rpc_socket_t        *newsock;
#endif
{
    rpc_socket_error_t  serr;

accept_again:
    RPC_LOG_SOCKET_ACCEPT_NTR;
    if (addr == NULL)
    {
        int addrlen;

        addrlen = 0;
        *newsock = accept
            ((int) sock, (struct sockaddr *) NULL, (int *) &addrlen);
    }
    else
    {
        RPC_SOCKET_FIX_ADDRLEN(addr);
        *newsock = accept
            ((int) sock, (struct sockaddr *) (&addr->sa), (int *) (&addr->len));
        RPC_SOCKET_FIX_ADDRLEN(addr);
    }
    serr = (*newsock == -1) ? errno : RPC_C_SOCKET_OK;
    RPC_LOG_SOCKET_ACCEPT_XIT;
    if (serr == EINTR)
    {
        goto accept_again;
    }
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ L I S T E N
 *
 * Listen for a connection on a socket.
 * This is used only by Connection oriented Protocol Services.
 *
 * (see BSD UNIX listen(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_listen
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    int                 backlog
)
#else
(sock, backlog)
rpc_socket_t        sock;
int                 backlog;
#endif
{
    rpc_socket_error_t  serr;
    
    RPC_LOG_SOCKET_LISTEN_NTR;
    serr = (listen(sock, backlog) == -1) ? errno : RPC_C_SOCKET_OK;
    RPC_LOG_SOCKET_LISTEN_XIT;
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ S E N D M S G
 *
 * Send a message over a given socket.  An error code as well as the
 * actual number of bytes sent are returned.
 *
 * (see BSD UNIX sendmsg(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_sendmsg
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    rpc_socket_iovec_p_t iov,       /* array of bufs of data to send */
    int                 iov_len,    /* number of bufs */
    rpc_addr_p_t        addr,       /* addr of receiver */
    int                 *cc        /* returned number of bytes actually sent */
)
#else
(sock, iov, iov_len, addr, cc)
rpc_socket_t        sock;
rpc_socket_iovec_p_t iov;       /* array of bufs of data to send */
int                 iov_len;    /* number of bufs */
rpc_addr_p_t        addr;       /* addr of receiver */
int                 *cc;        /* returned number of bytes actually sent */
#endif
{
    rpc_socket_error_t serr;

    RPC_SOCKET_SENDMSG(sock, iov, iov_len, addr, cc, &serr);
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ R E C V F R O M
 *
 * Recieve the next buffer worth of information from a socket.  A
 * rpc_addr_t appropriate for the NAF corresponding to this socket must
 * be provided.  addr.len must set to the actual size of addr.sa.  This
 * operation fills in addr.sa and sets addr.len to the new size of the
 * field.  An error status as well as the actual number of bytes received
 * are also returned.
 * 
 * (see BSD UNIX recvfrom(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_recvfrom
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    byte_p_t            buf,        /* buf for rcvd data */
    int                 len,        /* len of above buf */
    rpc_addr_p_t        from,       /* addr of sender */
    int                 *cc        /* returned number of bytes actually rcvd */
)
#else
(sock, buf, len, from, cc)
rpc_socket_t        sock;
byte_p_t            buf;        /* buf for rcvd data */
int                 len;        /* len of above buf */
rpc_addr_p_t        from;       /* addr of sender */
int                 *cc;        /* returned number of bytes actually rcvd */
#endif
{
    rpc_socket_error_t serr;

    RPC_SOCKET_RECVFROM (sock, buf, len, from, cc, &serr);
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ R E C V M S G
 *
 * Receive a message over a given socket.  A rpc_addr_t appropriate for
 * the NAF corresponding to this socket must be provided.  addr.len must
 * set to the actual size of addr.sa.  This operation fills in addr.sa
 * and sets addr.len to the new size of the field.  An error code as
 * well as the actual number of bytes received are also returned.
 * 
 * (see BSD UNIX recvmsg(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_recvmsg
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    rpc_socket_iovec_p_t iov,       /* array of bufs for rcvd data */
    int                 iov_len,    /* number of bufs */
    rpc_addr_p_t        addr,       /* addr of sender */
    int                 *cc        /* returned number of bytes actually rcvd */
)
#else
(sock, iov, iov_len, addr, cc)
rpc_socket_t        sock;
rpc_socket_iovec_p_t iov;       /* array of bufs for rcvd data */
int                 iov_len;    /* number of bufs */
rpc_addr_p_t        addr;       /* addr of sender */
int                 *cc;        /* returned number of bytes actually rcvd */
#endif
{
    rpc_socket_error_t serr;

    RPC_SOCKET_RECVMSG(sock, iov, iov_len, addr, cc, &serr);
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ I N Q _ A D D R
 *
 * Return the local address associated with a socket.  A rpc_addr_t
 * appropriate for the NAF corresponding to this socket must be provided.
 * addr.len must set to the actual size of addr.sa.  This operation fills
 * in addr.sa and sets addr.len to the new size of the field.
 *
 * !!! NOTE: You should use rpc__naf_desc_inq_addr() !!!
 *
 * This routine is indended for use only by the internal routine:
 * rpc__naf_desc_inq_addr().  rpc__socket_inq_endpoint() only has the
 * functionality of BSD UNIX getsockname() which doesn't (at least not
 * on all systems) return the local network portion of a socket's address.
 * rpc__naf_desc_inq_addr() returns the complete address for a socket.
 *
 * (see BSD UNIX getsockname(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_inq_endpoint
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    rpc_addr_p_t        addr
)
#else
(sock, addr)
rpc_socket_t        sock;
rpc_addr_p_t        addr;
#endif
{
    rpc_socket_error_t  serr;

    RPC_LOG_SOCKET_INQ_EP_NTR;
    RPC_SOCKET_FIX_ADDRLEN(addr);
    serr = (getsockname(sock, (void*)&addr->sa, (int*)&addr->len) == -1) ? errno : RPC_C_SOCKET_OK;
    RPC_SOCKET_FIX_ADDRLEN(addr);
    RPC_LOG_SOCKET_INQ_EP_XIT;
    return (serr);
}

/*
 * R P C _ _ S O C K E T _ S E T _ B R O A D C A S T
 *
 * Enable broadcasting for the socket (as best it can).
 * Used only by Datagram based Protocol Services.
 */

PRIVATE rpc_socket_error_t rpc__socket_set_broadcast
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock
)
#else
(sock)
rpc_socket_t        sock;
#endif
{
#ifdef SO_BROADCAST
    int setsock_val = 1;
    int i;

    i = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, 
            &setsock_val, sizeof(setsock_val));
    if (i < 0) 
    {
        RPC_DBG_GPRINTF(("(rpc__socket_set_broadcast) error=%d\n", errno));
        return (errno);
    }

    return(RPC_C_SOCKET_OK);
#else
    return(RPC_C_SOCKET_OK);
#endif
}

/*
 * R P C _ _ S O C K E T _ S E T _ B U F S
 *
 * Set the socket's send and receive buffer sizes and return the new
 * values.  Note that the sizes are min'd with
 * "rpc_c_socket_max_{snd,rcv}buf" because systems tend to fail the
 * operation rather than give the max buffering if the max is exceeded.
 *
 * If for some reason your system is screwed up and defines SOL_SOCKET
 * and SO_SNDBUF, but doesn't actually support the SO_SNDBUF and SO_RCVBUF
 * operations AND using them would result in nasty behaviour (i.e. they
 * don't just return some error code), define NO_SO_SNDBUF.
 *
 * If the buffer sizes provided are 0, then we use the operating
 * system default (i.e. we don't set anything at all).
 */

PRIVATE rpc_socket_error_t rpc__socket_set_bufs
    
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    unsigned32          txsize,
    unsigned32          rxsize,
    unsigned32          *ntxsize,
    unsigned32          *nrxsize
)
#else
(sock, txsize, rxsize, ntxsize, nrxsize)
rpc_socket_t        sock;
unsigned32          txsize;
unsigned32          rxsize;
unsigned32          *ntxsize;
unsigned32          *nrxsize;
#endif
{
    unsigned32 sizelen;
    int e;

#if (defined (SOL_SOCKET) && defined(SO_SNDBUF)) && !defined(NO_SO_SNDBUF)

    /*
     * Set the new sizes.
     */

    txsize = MIN(txsize, RPC_C_SOCKET_MAX_SNDBUF);
    if (txsize != 0)
    {
        e = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &txsize, sizeof(txsize));
        if (e == -1)
        {
            RPC_DBG_GPRINTF
(("(rpc__socket_set_bufs) WARNING: set sndbuf (%d) failed - error = %d\n", 
                txsize, errno));
        }
    }

    rxsize = MIN(rxsize, RPC_C_SOCKET_MAX_RCVBUF);
    if (rxsize != 0)
    {
        e = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rxsize, sizeof(rxsize));
        if (e == -1)
        {
            RPC_DBG_GPRINTF
(("(rpc__socket_set_bufs) WARNING: set rcvbuf (%d) failed - error = %d\n", 
                rxsize, errno));
        }
    }

    /*
     * Get the new sizes.  If this fails, just return some guessed sizes.
     */
    *ntxsize = 0;
    sizelen = sizeof *ntxsize;
    e = getsockopt(sock, SOL_SOCKET, SO_SNDBUF, ntxsize, (int *)&sizelen);
    if (e == -1)
    {
        RPC_DBG_GPRINTF
(("(rpc__socket_set_bufs) WARNING: get sndbuf failed - error = %d\n", errno));
        *ntxsize = RPC_C_SOCKET_GUESSED_SNDBUF;
    }

    *nrxsize = 0;
    sizelen = sizeof *nrxsize;
    e = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, nrxsize, (int *)&sizelen);
    if (e == -1)
    {
        RPC_DBG_GPRINTF
(("(rpc__socket_set_bufs) WARNING: get rcvbuf failed - error = %d\n", errno));
        *nrxsize = RPC_C_SOCKET_GUESSED_RCVBUF;
    }

#  ifdef apollo
    /*
     * On Apollo, modifying the socket buffering doesn't actually do
     * anything on IP sockets, but the calls succeed anyway.  We can
     * detect this by the fact that the new buffer length returned is
     * 0. Return what we think the actually length is.
     */
    if (rxsize != 0 && *nrxsize == 0) 
    {
        *nrxsize = (8 * 1024);
    }
    if (txsize != 0 && *ntxsize == 0) 
    {
        *ntxsize = (8 * 1024);
    }
#  endif

#else

    *ntxsize = RPC_C_SOCKET_GUESSED_SNDBUF;
    *nrxsize = RPC_C_SOCKET_GUESSED_RCVBUF;

#endif

    return (RPC_C_SOCKET_OK);
}

/*
 * R P C _ _ S O C K E T _ S E T _ N B I O
 *
 * Set a socket to non-blocking mode.
 * 
 * Return RPC_C_SOCKET_OK on success, otherwise an error value.
 */

PRIVATE rpc_socket_error_t rpc__socket_set_nbio
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock
)
#else
(sock)
rpc_socket_t        sock;
#endif
{
#ifndef vms

    int i;

    i = fcntl(sock, F_SETFL, O_NDELAY);
    if (i == -1)
    {
        RPC_DBG_GPRINTF(("(rpc__socket_set_nbio) error=%d\n", errno));
        return (errno);
    }

    return (RPC_C_SOCKET_OK);

#else

#ifdef DUMMY
/*
 * Note: This call to select non-blocking I/O is not implemented
 * by UCX on VMS. If this routine is really needed to work in the future
 * on VMS this will have to be done via QIO's.
 */
    int flag = true;
    
    ioctl(sock, FIONBIO, &flag);
#endif
    return (RPC_C_SOCKET_OK);

#endif
}

/*
 * R P C _ _ S O C K E T _ S E T _ C L O S E _ O N _ E X E C
 *
 *
 * Set a socket to a mode whereby it is not inherited by a spawned process
 * executing some new image. This is possibly a no-op on some systems.
 *
 * Return RPC_C_SOCKET_OK on success, otherwise an error value.
 */

PRIVATE rpc_socket_error_t rpc__socket_set_close_on_exec
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock
)
#else
(sock)
rpc_socket_t        sock;
#endif
{
#ifndef vms
    int i;

    i = fcntl(sock, F_SETFD, 1);
    if (i == -1)
    {
        RPC_DBG_GPRINTF(("(rpc__socket_set_close_on_exec) error=%d\n", errno));
        return (errno);
    }
#endif

    return (RPC_C_SOCKET_OK);
}

/*
 * R P C _ _ S O C K E T _ G E T P E E R N A M E
 *
 * Get name of connected peer.
 * This is used only by Connection oriented Protocol Services.
 *
 * (see BSD UNIX getpeername(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_getpeername 
#ifdef _DCE_PROTO_
(
    rpc_socket_t sock,
    rpc_addr_p_t addr
)
#else
(sock, addr)
rpc_socket_t sock;
rpc_addr_p_t addr;
#endif
{
    rpc_socket_error_t serr;

    RPC_SOCKET_FIX_ADDRLEN(addr);
    serr = (getpeername(sock, (void*)&addr->sa, (int*)&addr->len) == -1) ? errno : RPC_C_SOCKET_OK;
    RPC_SOCKET_FIX_ADDRLEN(addr);

    return (serr);
}

/*
 * R P C _ _ S O C K E T _ G E T _ I F _ I D
 *
 * Get socket network interface id (socket type).
 *
 * (see BSD UNIX getsockopt(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_get_if_id 
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock,
    rpc_network_if_id_t *network_if_id
)
#else
(sock, network_if_id)
rpc_socket_t        sock;
rpc_network_if_id_t *network_if_id;
#endif
{
    int optlen;
    
    optlen = sizeof(rpc_network_if_id_t);
    
    return (getsockopt (sock,
                        SOL_SOCKET,
                        SO_TYPE,
                        network_if_id,
                        &optlen) == -1  ? errno : RPC_C_SOCKET_OK);
}

/*
 * R P C _ _ S O C K E T _ S E T _ K E E P A L I V E
 *
 * Enable periodic transmissions on a connected socket, when no
 * other data is being exchanged. If the other end does not respond to
 * these messages, the connection is considered broken and the
 * so_error variable is set to ETIMEDOUT.
 * Used only by Connection based Protocol Services.
 *
 * (see BSD UNIX setsockopt(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_set_keepalive
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock
)
#else
(sock)
rpc_socket_t        sock;
#endif
{
#ifdef SO_KEEPALIVE
    int setsock_val = 1;
    int i;

    i = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
            &setsock_val, sizeof(setsock_val));
    if (i < 0) 
    {
        RPC_DBG_GPRINTF(("(rpc__socket_set_keepalive) error=%d\n", errno));
        return (errno);
    }

    return(RPC_C_SOCKET_OK);
#else
    return(RPC_C_SOCKET_OK);
#endif
}


/*
 * R P C _ _ S O C K E T _ N O W R I T E B L O C K _ W A I T
 *
 * Wait until the a write on the socket should succede without
 * blocking.  If tmo is NULL, the wait is unbounded, otherwise
 * tmo specifies the max time to wait. RPC_C_SOCKET_ETIMEDOUT
 * if a timeout occurs.  This operation in not cancellable.
 */

PRIVATE rpc_socket_error_t rpc__socket_nowriteblock_wait
#ifdef _DCE_PROTO_
(
    rpc_socket_t sock,
    struct timeval *tmo
)
#else
(sock, tmo)
rpc_socket_t sock;
struct timeval *tmo;
#endif
{
    fd_set  write_fds;
    int     nfds, num_found;
    int     cs;

    FD_ZERO (&write_fds);
    FD_SET (sock, &write_fds);
    nfds = sock + 1;
                  
	 pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

    num_found = select(nfds, NULL, (void *)&write_fds, NULL, tmo);

    pthread_setcancelstate(cs, NULL);

    if (num_found < 0)
    {
        RPC_DBG_GPRINTF(("(rpc__socket_nowriteblock_wait) error=%d\n", errno));
        return errno;
    }

    if (num_found == 0)
    {
        RPC_DBG_GPRINTF(("(rpc__socket_nowriteblock_wait) timeout\n"));
        return RPC_C_SOCKET_ETIMEDOUT;
    }

    return RPC_C_SOCKET_OK;
}

