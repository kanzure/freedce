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

#define HACK_DEBUG

#include <commonp.h>
#include <com.h>
#include <comprot.h>
#include <comnaf.h>
#include <comp.h>
#include <fcntl.h>
#include <rpcmem.h>
#include <comsoc.h>
#include <comsoc_win32_np.h>
#include <ncacn_np.h>
/*#include <dce/cma_ux_wrappers.h>*/

#ifdef HAVE_OS_WIN32
#define socket_error win32_socket_err()
#define bind win32_bind
#define accept win32_accept
#define listen win32_listen
#define connect win32_connect
#define setsockopt win32_setsockopt
#define getsockopt win32_getsockopt
#define getsockname win32_getsockname
#define getpeername win32_getpeername
#define select win32_select
#else
#define socket_error errno
#endif


/* ======================================================================== */

/*
 * What we think a socket's buffering is in case rpc__socket_np_set_bufs()
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
 * R P C _ _ S O C K E T _ O P E N _ S R V
 *
 * Create a new socket for the specified Protocol Sequence.
 * The new socket has blocking IO semantics.
 *
 * (see BSD UNIX socket(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_np_open_srv
#ifdef _DCE_PROTO_
(
    rpc_protseq_id_t    pseq_id,
    rpc_addr_p_t        addr,
    rpc_socket_t        *sock
)
#else
(pseq_id, addr, sp)
rpc_protseq_id_t    pseq_id;
rpc_addr_p_t        addr;
rpc_socket_t        *sock;
#endif
{
    struct sockaddr_np * npsa = (struct sockaddr_np *)&addr->sa;
    RPC_LOG_SOCKET_OPEN_NTR;
    if (pseq_id != RPC_C_PROTSEQ_ID_NCACN_NP)
	    return rpc_s_cant_create_sock;
    *sock = rpc__namedpipe_create(npsa->pipe_name);
    RPC_LOG_SOCKET_OPEN_XIT;
    return ((*sock == -1) ? np_socket_error() : RPC_C_SOCKET_OK);
}



/*
 * R P C _ _ S O C K E T _ O P E N _ C L I
 *
 * Create a new socket for the specified Protocol Sequence.
 * The new socket has blocking IO semantics.
 *
 * (see BSD UNIX socket(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_np_open_cli
#ifdef _DCE_PROTO_
(
    rpc_protseq_id_t    pseq_id,
    rpc_addr_p_t        addr,
    rpc_socket_t        *sock
)
#else
(pseq_id, addr, sp)
rpc_protseq_id_t    pseq_id;
rpc_addr_p_t        addr;
rpc_socket_t        *sock;
#endif
{
    struct sockaddr_np * npsa = (struct sockaddr_np *)&addr->sa;
    RPC_LOG_SOCKET_OPEN_NTR;
    if (pseq_id != RPC_C_PROTSEQ_ID_NCACN_NP)
	    return rpc_s_cant_create_sock;
    *sock = rpc__namedpipe_open_cli(npsa->serv_name, npsa->pipe_name);
    RPC_LOG_SOCKET_OPEN_XIT;
    return ((*sock == -1) ? np_socket_error() : RPC_C_SOCKET_OK);
}



/*
 * R P C _ _ S O C K E T _ O P E N _ B A S I C
 *
 * A special version of socket_open that is used *only* by 
 * the low level initialization code when it is trying to 
 * determine what network services are supported by the host OS.
 */

PRIVATE rpc_socket_error_t rpc__socket_np_open_basic
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
    *sock = win32_socket((int) naf, (int) net_if, (int) net_prot);

    return ((*sock == -1) ? socket_error : RPC_C_SOCKET_OK);
}

/*
 * R P C _ _ S O C K E T _ C L O S E
 *
 * Close (destroy) a socket.
 *
 * (see BSD UNIX close(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_np_close
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
    serr = (rpc__namedpipe_close(sock) == -1) ? socket_error : RPC_C_SOCKET_OK;
    RPC_LOG_SOCKET_CLOSE_XIT;
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

PRIVATE rpc_socket_error_t rpc__socket_np_connect
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
            == -1) ? socket_error : RPC_C_SOCKET_OK;
    RPC_LOG_SOCKET_CONNECT_XIT;
    if (serr == RPC_C_SOCKET_EINTR)
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

PRIVATE rpc_socket_error_t rpc__socket_np_accept
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
        *newsock = accept
            ((int) sock, NULL, NULL);
    }
    else
    {
        RPC_SOCKET_FIX_ADDRLEN(addr);
        *newsock = accept
            ((int) sock, (struct sockaddr *) (&addr->sa), (int *) (&addr->len));
        RPC_SOCKET_FIX_ADDRLEN(addr);
    }
    serr = (*newsock == -1) ? socket_error : RPC_C_SOCKET_OK;
    RPC_LOG_SOCKET_ACCEPT_XIT;
    if (serr == RPC_C_SOCKET_EINTR)
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

PRIVATE rpc_socket_error_t rpc__socket_np_listen
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
    serr = (listen(sock, backlog) == -1) ? socket_error : RPC_C_SOCKET_OK;
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

PRIVATE rpc_socket_error_t rpc__socket_np_sendmsg
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

PRIVATE rpc_socket_error_t rpc__socket_np_recvfrom
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

PRIVATE rpc_socket_error_t rpc__socket_np_recvmsg
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
 * rpc__naf_desc_inq_addr().  rpc__socket_np_inq_endpoint() only has the
 * functionality of BSD UNIX getsockname() which doesn't (at least not
 * on all systems) return the local network portion of a socket's address.
 * rpc__naf_desc_inq_addr() returns the complete address for a socket.
 *
 * (see BSD UNIX getsockname(2)).
 */

PRIVATE rpc_socket_error_t rpc__socket_np_inq_endpoint
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
    serr = (getsockname(sock, (void*)&addr->sa, (int*)&addr->len) == -1) ? socket_error : RPC_C_SOCKET_OK;
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

PRIVATE rpc_socket_error_t rpc__socket_np_set_broadcast
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

    i = win32_setsockopt(sock, SOL_SOCKET, SO_BROADCAST, 
            &setsock_val, sizeof(setsock_val));
    if (i < 0) 
    {
        RPC_DBG_GPRINTF(("(rpc__socket_np_set_broadcast) error=%d\n", socket_error));
        return (socket_error);
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

PRIVATE rpc_socket_error_t rpc__socket_np_set_bufs
    
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
(("(rpc__socket_np_set_bufs) WARNING: set sndbuf (%d) failed - error = %d\n", 
                txsize, socket_error));
        }
    }

    rxsize = MIN(rxsize, RPC_C_SOCKET_MAX_RCVBUF);
    if (rxsize != 0)
    {
        e = win32_setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rxsize, sizeof(rxsize));
        if (e == -1)
        {
            RPC_DBG_GPRINTF
(("(rpc__socket_np_set_bufs) WARNING: set rcvbuf (%d) failed - error = %d\n", 
                rxsize, socket_error));
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
(("(rpc__socket_np_set_bufs) WARNING: get sndbuf failed - error = %d\n", socket_error));
        *ntxsize = RPC_C_SOCKET_GUESSED_SNDBUF;
    }

    *nrxsize = 0;
    sizelen = sizeof *nrxsize;
    e = getsockopt(sock, SOL_SOCKET, SO_RCVBUF, nrxsize, (int *)&sizelen);
    if (e == -1)
    {
        RPC_DBG_GPRINTF
(("(rpc__socket_np_set_bufs) WARNING: get rcvbuf failed - error = %d\n", socket_error));
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

PRIVATE rpc_socket_error_t rpc__socket_np_set_nbio
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock  __attribute__((__unused__))
)
#else
(sock)
rpc_socket_t        sock;
#endif
{
    return (RPC_C_SOCKET_OK);
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

PRIVATE rpc_socket_error_t rpc__socket_np_set_close_on_exec
#ifdef _DCE_PROTO_
(
    rpc_socket_t        sock  __attribute__((__unused__))
)
#else
(sock)
rpc_socket_t        sock;
#endif
{
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

PRIVATE rpc_socket_error_t rpc__socket_np_getpeername 
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
    serr = (getpeername(sock, (void*)&addr->sa, (int*)&addr->len) == -1) ? socket_error : RPC_C_SOCKET_OK;
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

PRIVATE rpc_socket_error_t rpc__socket_np_get_if_id 
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
                        &optlen) == -1  ? socket_error : RPC_C_SOCKET_OK);
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

PRIVATE rpc_socket_error_t rpc__socket_np_set_keepalive
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
        RPC_DBG_GPRINTF(("(rpc__socket_np_set_keepalive) error=%d\n", socket_error));
        return (socket_error);
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

PRIVATE rpc_socket_error_t rpc__socket_np_nowriteblock_wait
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
    FD_SET ((unsigned int)sock, &write_fds);
    nfds = sock + 1;
                  
    cs = sys_pthread_setcancel(CANCEL_ON);

    num_found = select(nfds, NULL, (void *)&write_fds, NULL, tmo);

    cs = sys_pthread_setcancel(cs);

    if (num_found < 0)
    {
        RPC_DBG_GPRINTF(("(rpc__socket_np_nowriteblock_wait) error=%d\n", socket_error));
        return socket_error;
    }

    if (num_found == 0)
    {
        RPC_DBG_GPRINTF(("(rpc__socket_np_nowriteblock_wait) timeout\n"));
        return RPC_C_SOCKET_ETIMEDOUT;
    }

    return RPC_C_SOCKET_OK;
}

static rpc_socket_error_t rpc__socket_np_nodelay (
		        rpc_socket_t  sock
			    )
{
    int                 delay = 1;

    /*
     * Assume this is a TCP socket and corresponding connection. If
     * not the setsockopt will fail.
     */
    if (setsockopt (sock,
                           IPPROTO_TCP,
                           TCP_NODELAY,
                           (char *) &delay,
                           sizeof (delay)) < 0)
    {
        return rpc_s_cannot_set_nodelay;
    }

    return rpc_s_ok;
}


static rpc_socket_epv_t np_sock_fns = 
{
	NULL, /* open: just... don't call it.  just don't! */
	rpc__socket_np_open_cli,
	rpc__socket_np_open_srv,
	NULL, /* open_basic: just... don't call it.  just don't! */
	rpc__socket_np_close,
	NULL, /* bind: just... don't call it.  just don't! */
	rpc__socket_np_connect,
	rpc__socket_np_accept,
	rpc__socket_np_listen,
	rpc__socket_np_sendmsg,
	rpc__socket_np_recvfrom,
	rpc__socket_np_recvmsg,
	rpc__socket_np_inq_endpoint,
	rpc__socket_np_set_broadcast,
	rpc__socket_np_set_bufs,
	rpc__socket_np_set_nbio,
	rpc__socket_np_set_close_on_exec,
	rpc__socket_np_getpeername,
	rpc__socket_np_get_if_id,
	rpc__socket_np_set_keepalive,
	rpc__socket_np_nowriteblock_wait,
	rpc__socket_np_nodelay
};


/*
 * R P C _ _ S O C K E T _ N O W R I T E B L O C K _ W A I T
 *
 * Wait until the a write on the socket should succede without
 * blocking.  If tmo is NULL, the wait is unbounded, otherwise
 * tmo specifies the max time to wait. RPC_C_SOCKET_ETIMEDOUT
 * if a timeout occurs.  This operation in not cancellable.
 */

PRIVATE void rpc__socket_np_init
(rpc_socket_epv_p_t *epv)
{
	*epv = &np_sock_fns;
}
