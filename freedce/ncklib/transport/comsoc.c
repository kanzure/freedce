#include <commonp.h>
#include <com.h>
#include <comprot.h>
#include <comnaf.h>
#include <comp.h>

/* lookup table of socket endpoints: allows different handlers
 * for different sockets: therefore you can emulate sockets
 * in userspace, hurrah.
 *
 * ThePlan(tm): to emulate ncacn_np sockets.  it also means
 * netbios can be emulated, userspace too, etc. blah blah,
 * and yippee ncalrpc using shared memory or something,
 * but with a socket interface.
 */
static rpc_socket_epv_p_t epvs[256];

extern void rpc__socket_bsd_init (rpc_socket_epv_p_t *epv);

#define SCKEPV(sock) \
	rpc_socket_epv_p_t epv; \
	if (sock == -1) \
		return rpc_s_socket_failure; \
	epv = epvs[sock]; \
	if (epv == NULL) \
		return rpc_s_socket_failure;

/*
 * R P C _ _ S O C K E T _ O P E N
 *
 * Create a new socket for the specified Protocol Sequence.
 * The new socket has blocking IO semantics.
 *
 * (see BSD UNIX socket(2)).
 */

rpc_socket_error_t rpc__socket_open (
        rpc_protseq_id_t pseq_id,
        rpc_socket_t *sock
    )
{
	rpc_socket_error_t err;
	rpc_socket_epv_p_t epv = NULL;

	rpc__socket_bsd_init(&epv); /* XXX: urr, yukk!!! */
	/*
        epv = RPC_PROTOCOL_INQ_SOCKET_EPV(
			RPC_PROTSEQ_INQ_PROT_ID(pseq_id));*/
	if (epv == NULL)
		return rpc_s_cant_create_socket;
        err = epv->sock_open(pseq_id, sock);

        if (err != rpc_s_ok)
		return err;

	epvs[*sock] = epv;

        return err;
}


/*
 * R P C _ _ S O C K E T _ O P E N _ B A S I C
 *
 * A special version of socket_open that is used *only* by 
 * the low level initialization code when it is trying to 
 * determine what network services are supported by the host OS.
 */

rpc_socket_error_t rpc__socket_open_basic (
        rpc_naf_id_t  naf,
        rpc_network_if_id_t  net_if,
        rpc_network_protocol_id_t  net_prot,
        rpc_socket_t * sock
    )
{
	/*
	if (naf != RPC_C_NAF_ID_UXD && naf != RPC_C_NAF_ID_IP)
		return rpc_s_cant_create_socket;
		*/
	rpc_socket_error_t err;
	rpc_socket_epv_p_t epv = NULL;

	rpc__socket_bsd_init(&epv); /* XXX: urr, yukk!!! */
	if (epv == NULL)
		return rpc_s_cant_create_socket;

        err = epv->sock_open_basic(naf, net_if, net_prot, sock);

        if (err != rpc_s_ok)
		return err;

	epvs[*sock] = epv;

        return err;
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

extern rpc_socket_error_t rpc__socket_accept (
        rpc_socket_t  sock,
        rpc_addr_p_t  addr,
        rpc_socket_t * newsock
    )
{
        rpc_socket_error_t err;
	SCKEPV(sock);

        err = epv->sock_accept(sock, addr, newsock);

        if (err != rpc_s_ok)
		return err;
	epvs[*newsock] = epvs[sock];
        return err;
}



rpc_socket_error_t rpc__socket_close (
        rpc_socket_t sock
    )
{
	SCKEPV(sock);
	epvs[sock] = NULL;
	return epv->sock_close(sock);
}


/*
 * R P C _ _ S O C K E T _ B I N D
 *
 * Bind a socket to a specified local address.
 *
 * (see BSD UNIX bind(2)).
 */

rpc_socket_error_t rpc__socket_bind (
        rpc_socket_t sock,
        rpc_addr_p_t addr
    )
{
	SCKEPV(sock);
	return epv->sock_bind(sock, addr);
}


/*
 * R P C _ _ S O C K E T _ C O N N E C T
 *
 * Connect a socket to a specified peer's address.
 * This is used only by Connection oriented Protocol Services.
 *
 * (see BSD UNIX connect(2)).
 */

rpc_socket_error_t rpc__socket_connect (
        rpc_socket_t  sock,
        rpc_addr_p_t addr
    )
{
	SCKEPV(sock);
	return epv->sock_connect(sock, addr);
}


/*
 * R P C _ _ S O C K E T _ L I S T E N
 *
 * Listen for a connection on a socket.
 * This is used only by Connection oriented Protocol Services.
 *
 * (see BSD UNIX listen(2)).
 */

rpc_socket_error_t rpc__socket_listen (
        rpc_socket_t sock,
        int backlog
    )
{
	SCKEPV(sock);
	return epv->sock_listen(sock, backlog);
}


/*
 * R P C _ _ S O C K E T _ S E N D M S G
 *
 * Send a message over a given socket.  An error code as well as the
 * actual number of bytes sent are returned.
 *
 * (see BSD UNIX sendmsg(2)).
 */

rpc_socket_error_t rpc__socket_sendmsg (
        rpc_socket_t  sock,
        rpc_socket_iovec_p_t  iov,   /* array of bufs of data to send */
        int  iov_len,        /* number of bufs */
        rpc_addr_p_t  addr,  /* addr of receiver */
        int * cc             /* returned number of bytes actually sent */
    )
{
	SCKEPV(sock);
	return epv->sock_sendmsg(sock, iov, iov_len, addr, cc);
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

rpc_socket_error_t rpc__socket_recvfrom (
        rpc_socket_t  sock,
        byte_p_t  buf,       /* buf for rcvd data */
        int  len,            /* len of above buf */
        rpc_addr_p_t  from,  /* addr of sender */
        int * cc             /* returned number of bytes actually rcvd */
    )
{
	SCKEPV(sock);
	return epv->sock_recvfrom(sock, buf, len, from, cc);
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

rpc_socket_error_t rpc__socket_recvmsg (
        rpc_socket_t  sock,
        rpc_socket_iovec_p_t  iov,   /* array of bufs for rcvd data */
        int  iov_len,        /* number of bufs */
        rpc_addr_p_t  addr,  /* addr of sender */
        int * cc             /* returned number of bytes actually rcvd */
    )
{
	SCKEPV(sock);
	return epv->sock_recvmsg(sock, iov, iov_len, addr, cc);
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

rpc_socket_error_t rpc__socket_inq_endpoint (
        rpc_socket_t  sock,
        rpc_addr_p_t addr
    )
{
	SCKEPV(sock);
	return epv->sock_inq_endpoint(sock, addr);
}


/*
 * R P C _ _ S O C K E T _ S E T _ B R O A D C A S T
 *
 * Enable broadcasting for the socket (as best it can).
 * Used only by Datagram based Protocol Services.
 */

rpc_socket_error_t rpc__socket_set_broadcast (
        rpc_socket_t sock
    )
{
	SCKEPV(sock);
	return epv->sock_set_broadcast(sock);
}


/*
 * R P C _ _ S O C K E T _ S E T _ B U F S
 *
 * Set the socket's send and receive buffer sizes and return the new
 * values.
 * 
 * (similar to BSD UNIX setsockopt()).
 */

rpc_socket_error_t rpc__socket_set_bufs (
        rpc_socket_t  sock, 
        unsigned32  txsize, 
        unsigned32  rxsize, 
        unsigned32 * ntxsize, 
        unsigned32 * nrxsize
    )
{
	SCKEPV(sock);
	return epv->sock_set_bufs (sock, txsize, rxsize, ntxsize, nrxsize);
}


/*
 * R P C _ _ S O C K E T _ S E T _ N B I O
 *
 * Set a socket to non-blocking mode.
 *
 * (see BSD UNIX fcntl(sock, F_SETFL, O_NDELAY))
 */

rpc_socket_error_t rpc__socket_set_nbio (
        rpc_socket_t sock
    )
{
	SCKEPV(sock);
	return epv->sock_set_nbio(sock);
}


/*
 * R P C _ _ S O C K E T _ S E T _ C L O S E _ O N _ E X E C
 *
 * Set a socket to a mode whereby it is not inherited by a spawned process
 * executing some new image. This is possibly a no-op on some systems.
 *
 * (see BSD UNIX fcntl(sock, F_SETFD, 1))
 */

rpc_socket_error_t rpc__socket_set_close_on_exec (
        rpc_socket_t sock
    )
{
	SCKEPV(sock);
	return epv->sock_set_close_on_exec(sock);
}

/*
 * R P C _ _ S O C K E T _ G E T P E E R N A M E
 *
 * Get name of connected peer.
 * This is used only by Connection oriented Protocol Services.
 *
 * (see BSD UNIX getpeername(2)).
 */

rpc_socket_error_t rpc__socket_getpeername (
        rpc_socket_t  sock,
        rpc_addr_p_t addr
    )
{
	SCKEPV(sock);
	return epv->sock_getpeername(sock, addr);
}

/*
 * R P C _ _ S O C K E T _ G E T _ I F _ I D
 *
 * Get socket network interface id (socket type).
 *
 * (see BSD UNIX getsockopt(2)).
 */

rpc_socket_error_t rpc__socket_get_if_id (
        rpc_socket_t         sock,
        rpc_network_if_id_t * network_if_id
    )
{
	SCKEPV(sock);
	return epv->sock_get_if_id(sock, network_if_id);
}

/*
 * R P C _ _ S O C K E T _ S E T _ K E E P A L I V E.
 *
 * Set keepalive option for connection.
 * Used only by Connection based Protocol Services.
 *
 * (see BSD UNIX setsockopt(2)).
 */

rpc_socket_error_t rpc__socket_set_keepalive (
        rpc_socket_t        sock
    )
{
	SCKEPV(sock);
	return epv->sock_set_keepalive(sock);
}

/*
 * R P C _ _ S O C K E T _ N O W R I T E B L O C K _ W A I T
 *
 * Wait until the a write on the socket should succede without
 * blocking.  If tmo is NULL, the wait is unbounded, otherwise
 * tmo specifies the max time to wait. rpc_c_socket_etimedout
 * if a timeout occurs.  This operation in not cancellable.
 */

rpc_socket_error_t rpc__socket_nowriteblock_wait (
        rpc_socket_t  sock,
        struct timeval * tmo
    )
{
	SCKEPV(sock);
	return epv->sock_nowriteblock_wait(sock, tmo);
}

rpc_socket_error_t rpc__socket_nodelay (
        rpc_socket_t  sock
    )
{
	SCKEPV(sock);
	return epv->sock_nodelay(sock);
}

