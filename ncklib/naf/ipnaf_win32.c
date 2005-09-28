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
**  Copyright (c) 1989 by
**      Hewlett-Packard Company, Palo Alto, Ca. & 
**      Digital Equipment Corporation, Maynard, Mass.
**
**
**  NAME
**
**      ipnaf_win32
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  This module contains routines specific to the Internet Protocol,
**  the Internet Network Address Family extension service, for Win32
**
**  - taken from ipnaf_linux.c
**
**
**
*/

#include <commonp.h>
#include <com.h>
#include <comnaf.h>
#include <comsoc.h>
#include <ipnaf.h>
#include <ctype.h>

#define getsockname win32_getsockname

#define HACK_DEBUG
#undef DEBUG
#define DEBUG 1


/***********************************************************************
 *
 *  Internal prototypes and typedefs.
 */

INTERNAL void enumerate_interfaces _DCE_PROTOTYPE_ ((
        rpc_protseq_id_t         /*protseq_id*/,
        rpc_socket_t             /*desc*/,
	boolean                 /* want_bcast */,
        rpc_addr_vector_p_t     * /*rpc_addr_vec*/,
        rpc_addr_vector_p_t     * /*netmask_addr_vec*/,
        unsigned32              * /*st*/
    ));

#if 0
INTERNAL boolean get_broadcast_addr _DCE_PROTOTYPE_ ((
        int                      /*desc*/,
        struct ifreq            * /*ifr*/,
        unsigned32               /*if_flags*/,
        struct sockaddr         * /*if_addr*/,
        rpc_ip_addr_p_t          /*ip_addr*/,
        rpc_ip_addr_p_t          /*netmask_addr*/
    ));
#endif

#ifndef NO_SPRINTF
#  define RPC__IP_NETWORK_SPRINTF   sprintf
#else
#  define RPC__IP_NETWORK_SPRINTF   rpc__ip_network_sprintf
#endif

typedef struct
{
    unsigned32  num_elt;
    struct
    {
        unsigned32  addr;
        unsigned32  netmask;
    } elt[1];
} rpc_ip_s_addr_vector_t, *rpc_ip_s_addr_vector_p_t;

INTERNAL rpc_ip_s_addr_vector_p_t local_ip_addr_vec = NULL;

/*
**++
**
**  ROUTINE NAME:       enumerate_interfaces 
**
**  SCOPE:              INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**  Return a vector of IP RPC addresses.  Note that this function is
**  shared by both "rpc__ip_desc_inq_addr" and "rpc__ip_get_broadcast"
**  so that we have to have only one copy of all the gore (ioctl's)
**  associated with inquiring about network interfaces.  This routine
**  filters out all network interface information that doesn't correspond
**  to up, non-loopback, IP-addressed network interfaces.  The supplied
**  procedure pointer (efun) does the rest of the work.
**  
**
**  INPUTS:
**
**      protseq_id      Protocol Sequence ID representing a particular
**                      Network Address Family, its Transport Protocol,
**                      and type.
**
**      desc            Descriptor, indicating a socket that has been
**                      created on the local operating platform.
**
**      efun            Procedure pointer supplied to "do the rest of the work".
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      rpc_addr_vec    Returned vector of RPC addresses.
**
**      netmask_addr_vec Returned vector of netmask RPC addresses.
**
**      status          A value indicating the status of the routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       none
**
**--
**/
INTERNAL void enumerate_interfaces 
#ifdef _DCE_PROTO_
(
    rpc_protseq_id_t        protseq_id,
    rpc_socket_t            desc __attribute__((__unused__)),
    boolean                 want_bcast,
    rpc_addr_vector_p_t     *rpc_addr_vec,
    rpc_addr_vector_p_t     *netmask_addr_vec,
    unsigned32              *status
)
#else
(protseq_id, desc, efun, rpc_addr_vec, netmask_addr_vec, status)
rpc_protseq_id_t        protseq_id;
rpc_socket_t            desc;
enumerate_fn_p_t        efun;
rpc_addr_vector_p_t     *rpc_addr_vec;
rpc_addr_vector_p_t     *netmask_addr_vec;
unsigned32              *status;
#endif
{
    rpc_ip_addr_p_t         ip_addr;
    int                     n_ifs;
    int                     i;
    rpc_ip_addr_p_t         netmask_addr = NULL;

    void *hnd = NULL;

    CODING_ERROR (status);

    /*
     * Get the list of network interfaces.
     */

    n_ifs = win32_get_ifaces_hnd(&hnd);

#ifdef HACK_DEBUG
    printf("enumerate interfaces: %d %p\n", n_ifs, hnd);
#endif
    if (n_ifs == 0 || hnd == NULL)
    {
        *status = -2;   /* !!! */
        return;
    }

    /*
     * Figure out how many interfaces there must be and allocate an  
     * RPC address vector with the appropriate number of elements.
     * (We may ask for a few too many in case some of the interfaces
     * are uninteresting.)
     */
    RPC_DBG_PRINTF(rpc_e_dbg_general, 10,
        ("num interfaces: %d\n", n_ifs));

    RPC_MEM_ALLOC (
        *rpc_addr_vec,
        rpc_addr_vector_p_t,
        (sizeof **rpc_addr_vec) + ((n_ifs - 1) * (sizeof (rpc_addr_p_t))),
        RPC_C_MEM_RPC_ADDR_VEC,
        RPC_C_MEM_WAITOK);
    
    if (*rpc_addr_vec == NULL)
    {
        *status = rpc_s_no_memory;
	win32_free_ifaces_hnd(hnd);
        return;
    }
    if (netmask_addr_vec != NULL)
    {
        RPC_MEM_ALLOC (
            *netmask_addr_vec,
            rpc_addr_vector_p_t,
         (sizeof **netmask_addr_vec) + ((n_ifs - 1) * (sizeof (rpc_addr_p_t))),
            RPC_C_MEM_RPC_ADDR_VEC,
            RPC_C_MEM_WAITOK);
        
        if (*netmask_addr_vec == NULL)
        {
            *status = rpc_s_no_memory;
            RPC_MEM_FREE (*rpc_addr_vec, RPC_C_MEM_RPC_ADDR_VEC);
	    win32_free_ifaces_hnd(hnd);
            return;
        }

        (*netmask_addr_vec)->len = 0;
    }

    /*
     * Go through the interfaces and get the info associated with them.
     */
    (*rpc_addr_vec)->len = 0;

    for (i=0; i < n_ifs; i++)
    {
	unsigned long	addr;
	unsigned long	mask;
	unsigned long	bcast;
	unsigned long	idx;

        RPC_DBG_PRINTF(rpc_e_dbg_general, 10, ("interface %d:\n", i));

	win32_get_iface(hnd, i, &idx, &addr, &mask, &bcast);

#ifdef HACK_HARDCODED_IP_ADDRESS
	/* XXX: HACK ALERT!!! */
	if (addr == 0)
	{
		addr = 0x020014ac;
		mask = 0x00ffffff;
	}
#endif
        /*
         * Get the interface's flags.  If the flags say that the interface
         * is not up or is the loopback interface, skip it.  Do the
         * SIOCGIFFLAGS on a copy of the ifr so we don't lose the original
         * contents of the ifr.  (ifr's are unions that hold only one
         * of the interesting interface attributes [address, flags, etc.]
         * at a time.)
         */

#if 0
#ifndef USE_LOOPBACK
	/*
	 * Ignore the loopback interface: assume it's 127.0.0.1 for now (er)
	 */
        if (addr == 0x7f000001) continue;
#endif
#endif
	/* XXX: todo - get flags... */
#if 0
        if_flags = ifreq.ifr_flags;     /* Copy out the flags */
        RPC_DBG_PRINTF(rpc_e_dbg_general, 10, ("flags are %x\n", if_flags));

        /* 
         * Ignore interfaces which are not 'up'. 
         */
        if ((if_flags & IFF_UP) == 0)
 
	  continue;

#ifndef USE_LOOPBACK
	/*
	 * Ignore the loopback interface
	 */

        if (if_flags & IFF_LOOPBACK) continue;
#endif
	/*
	 * Ignore Point-to-Point interfaces (i.e. SLIP/PPP )
         * *** NOTE:  We need an Environment Variable Evaluation at
	 * some point so we can selectively allow RPC servers to
	 * some up with/without SLIP/PPP bindings. For Dynamic PPP/SLIP
	 * interfaces, this creates problems for now.
	 */

        if (if_flags & IFF_POINTOPOINT) continue;
#endif

        /*
         * Get the addressing stuff for this interface.
         */

        /*
         * Allocate and fill in an IP RPC address for this interface.
         */
        RPC_MEM_ALLOC (
            ip_addr,
            rpc_ip_addr_p_t,
            sizeof (rpc_ip_addr_t),
            RPC_C_MEM_RPC_ADDR,
            RPC_C_MEM_WAITOK);

        if (ip_addr == NULL)
        {
            *status = rpc_s_no_memory;
            goto FREE_IT;
        }

        ip_addr->rpc_protseq_id = protseq_id;
        ip_addr->len            = sizeof (struct sockaddr_in);
	ip_addr->sa.sin_family = AF_INET;

        if (netmask_addr_vec != NULL)
        {
            RPC_MEM_ALLOC (
                netmask_addr,
                rpc_ip_addr_p_t,
                sizeof (rpc_ip_addr_t),
                RPC_C_MEM_RPC_ADDR,
                RPC_C_MEM_WAITOK);
            
            if (netmask_addr == NULL)
            {
                *status = rpc_s_no_memory;
                RPC_MEM_FREE (ip_addr, RPC_C_MEM_RPC_ADDR);
                goto FREE_IT;
            }
            
            netmask_addr->rpc_protseq_id = protseq_id;
            netmask_addr->len            = sizeof (struct sockaddr_in);
	    netmask_addr->sa.sin_family = AF_INET;
        }

	if (netmask_addr != NULL)
	{
		netmask_addr->sa.sin_addr.s_addr = mask;
	}
	ip_addr->sa.sin_addr.s_addr = addr;

	if (want_bcast && netmask_addr != NULL)
	{
		ip_addr->sa.sin_addr.s_addr |= ~mask;
	}
#ifdef HACK_DEBUG
	printf("iface: %d %08lx %08lx %08lx\n", i, addr, mask, bcast);
#endif

        RPC_SOCKET_FIX_ADDRLEN(ip_addr);
        (*rpc_addr_vec)->addrs[(*rpc_addr_vec)->len++] = (rpc_addr_p_t) ip_addr;
        if (netmask_addr_vec != NULL && netmask_addr != NULL)
            (*netmask_addr_vec)->addrs[(*netmask_addr_vec)->len++]
                = (rpc_addr_p_t) netmask_addr;
    }

    if ((*rpc_addr_vec)->len == 0) 
    {
        *status = -5;   /* !!! */
        goto FREE_IT;
    }

    *status = rpc_s_ok;
    win32_free_ifaces_hnd(hnd);
    return;

FREE_IT:

    for (i = 0; i < (int)((*rpc_addr_vec)->len); i++)
    {
        RPC_MEM_FREE ((*rpc_addr_vec)->addrs[i], RPC_C_MEM_RPC_ADDR);
    }

    RPC_MEM_FREE (*rpc_addr_vec, RPC_C_MEM_RPC_ADDR_VEC);
    if (netmask_addr_vec != NULL)
    {
        for (i = 0; i < (int)(*netmask_addr_vec)->len; i++)
        {
            RPC_MEM_FREE ((*netmask_addr_vec)->addrs[i], RPC_C_MEM_RPC_ADDR);
        }
        RPC_MEM_FREE (*netmask_addr_vec, RPC_C_MEM_RPC_ADDR_VEC);
    }
    win32_free_ifaces_hnd(hnd);
}

/*
**++
**
**  ROUTINE NAME:       rpc__ip_desc_inq_addr
**
**  SCOPE:              PRIVATE - declared in ipnaf.h
**
**  DESCRIPTION:
**      
**  Receive a socket descriptor which is queried to obtain family, endpoint
**  and network address.  If this information appears valid for an IP
**  address,  space is allocated for an RPC address which is initialized
**  with the information obtained from the socket.  The address indicating
**  the created RPC address is returned in rpc_addr.
**
**  INPUTS:
**
**      protseq_id      Protocol Sequence ID representing a particular
**                      Network Address Family, its Transport Protocol,
**                      and type.
**
**      desc            Descriptor, indicating a socket that has been
**                      created on the local operating platform.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      rpc_addr_vec
**
**      status          A value indicating the status of the routine.
**
**          rpc_s_ok               The call was successful.
**
**          rpc_s_no_memory         Call to malloc failed to allocate memory.
**
**          rpc_s_cant_inq_socket  Attempt to get info about socket failed.
**
**          Any of the RPC Protocol Service status codes.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       none
**
**--
**/

#ifdef HACK_DEBUG

static void print_asc(unsigned char  const *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		printf ("%c", isprint(buf[i]) ? buf[i] : '.');
	}
}

static void print_data(const char *buf1, int len)
{
	unsigned char const *buf = (unsigned char const *)buf1;
	int i = 0;

	if (buf == NULL)
	{
		printf ("dump_data: NULL, len=%d\n", len);
		return;
	}
	if (len < 0)
		return;
	if (len == 0)
	{
		printf ("\n");
		return;
	}

	printf ("[%03X] ", i);
	for (i = 0; i < len;)
	{
		printf ("%02X ", (int)buf[i]);
		i++;
		if (i % 8 == 0)
			printf (" ");
		if (i % 16 == 0)
		{
			print_asc(&buf[i - 16], 8);
			printf (" ");
			print_asc(&buf[i - 8], 8);
			printf ("\n");
			if (i < len)
				printf ("[%03X] ", i);
		}
	}

	if (i % 16 != 0)	/* finish off a non-16-char-length row */
	{
		int n;

		n = 16 - (i % 16);
		printf (" ");
		if (n > 8)
			printf (" ");
		while (n--)
			printf ("   ");

		n = MIN(8, i % 16);
		print_asc(&buf[i - (i % 16)], n);
		printf (" ");
		n = (i % 16) - n;
		if (n > 0)
			print_asc(&buf[i - n], n);
		printf ("\n");
	}
}

#endif

PRIVATE void rpc__ip_desc_inq_addr 
#ifdef _DCE_PROTO_
(
    rpc_protseq_id_t        protseq_id,
    rpc_socket_t            desc,
    rpc_addr_vector_p_t     *rpc_addr_vec,
    unsigned32              *status
)
#else
(protseq_id, desc, rpc_addr_vec, status)
rpc_protseq_id_t        protseq_id;
rpc_socket_t            desc;
rpc_addr_vector_p_t     *rpc_addr_vec;
unsigned32              *status;
#endif
{
    rpc_ip_addr_p_t         ip_addr;
    rpc_ip_addr_t           loc_ip_addr;


    CODING_ERROR (status);

    /*
     * Do a "getsockname" into a local IP RPC address.  If the network
     * address part of the result is non-zero, then the socket must be
     * bound to a particular IP address and we can just return a RPC
     * address vector with that one address (and endpoint) in it.
     * Otherwise, we have to enumerate over all the local network
     * interfaces the local host has and construct an RPC address for
     * each one of them.
     */
    loc_ip_addr.len = sizeof (rpc_ip_addr_t);
    RPC_SOCKET_FIX_ADDRLEN(&loc_ip_addr);

    if (getsockname (desc, (struct sockaddr *)&loc_ip_addr.sa, (int *)&loc_ip_addr.len) < 0)
    {
#ifdef HACK_DEBUG
	printf("getsockname failed\n");
#endif
        *status = -1;   /* !!! */
        return;
    }

    RPC_SOCKET_FIX_ADDRLEN(&loc_ip_addr);
	
#ifdef HACK_DEBUG
    	printf("rpc__ip_desc_inq_addr: getsockname returned %0lx\n",
	    loc_ip_addr.sa.sin_addr.s_addr);
    print_data((char*)&loc_ip_addr.sa, loc_ip_addr.len);
#endif

    if (loc_ip_addr.sa.sin_addr.s_addr == 0)
    {
	unsigned16              i;
        enumerate_interfaces
            (protseq_id, desc, 1, rpc_addr_vec, NULL, status);

        if (*status != rpc_s_ok)
        {
            return; 
        }
        for (i = 0; i < (*rpc_addr_vec)->len; i++)
        {
            ((rpc_ip_addr_p_t) (*rpc_addr_vec)->addrs[i])->sa.sin_port = loc_ip_addr.sa.sin_port;
        }
    }
    else
    {
        RPC_MEM_ALLOC (
            ip_addr,
            rpc_ip_addr_p_t,
            sizeof (rpc_ip_addr_t),
            RPC_C_MEM_RPC_ADDR,
            RPC_C_MEM_WAITOK);

        if (ip_addr == NULL)
        {
            *status = rpc_s_no_memory;
            return;
        }

        RPC_MEM_ALLOC (
            *rpc_addr_vec,
            rpc_addr_vector_p_t,
            sizeof **rpc_addr_vec,
            RPC_C_MEM_RPC_ADDR_VEC,
            RPC_C_MEM_WAITOK);
    
        if (*rpc_addr_vec == NULL)
        {
            RPC_MEM_FREE (ip_addr, RPC_C_MEM_RPC_ADDR);
            *status = rpc_s_no_memory;
            return;
        }

        ip_addr->rpc_protseq_id = protseq_id;
        ip_addr->len            = sizeof (struct sockaddr_in);
        ip_addr->sa             = loc_ip_addr.sa;

        (*rpc_addr_vec)->len = 1;
        (*rpc_addr_vec)->addrs[0] = (rpc_addr_p_t) ip_addr;

        *status = rpc_s_ok;
        return;
    }
}

/*
**++
**
**  ROUTINE NAME:       rpc__ip_get_broadcast
**
**  SCOPE:              PRIVATE - EPV declared in ipnaf.h
**
**  DESCRIPTION:
**      
**  Return a vector of RPC addresses that represent all the address
**  required so that sending on all of them results in broadcasting on
**  all the local network interfaces.
**
**
**  INPUTS:
**
**      naf_id          Network Address Family ID serves
**                      as index into EPV for IP routines.
**
**      rpc_protseq_id
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:                        
**
**      rpc_addr_vec
**
**      status          A value indicating the status of the routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       none
**
**--
**/

PRIVATE void rpc__ip_get_broadcast 
#ifdef _DCE_PROTO_
(
    rpc_naf_id_t            naf_id __attribute__((__unused__)),
    rpc_protseq_id_t        protseq_id,
    rpc_addr_vector_p_t     *rpc_addr_vec,
    unsigned32              *status 
)
#else
(naf_id, protseq_id, rpc_addr_vec, status)
rpc_naf_id_t            naf_id;
rpc_protseq_id_t        protseq_id;
rpc_addr_vector_p_t     *rpc_addr_vec;
unsigned32              *status; 
#endif
{
    int                     desc;

    CODING_ERROR (status);


    /*
     * Open a socket to pass to "enumerate_interface".
     */
    desc = win32_socket(AF_INET, SOCK_DGRAM, 0);

    if (desc < 0) 
    {
        *status = -7;   /* !!! */
        return;
    }

    enumerate_interfaces
        (protseq_id, desc, 1, rpc_addr_vec, NULL, status);
    win32_close(desc);
}

/*
**++
**
**  ROUTINE NAME:       rpc__ip_init_local_addr_vec
**
**  SCOPE:              PRIVATE - declared in ipnaf.h
**
**  DESCRIPTION:
**      
**  Initialize the local address vectors.
**
**
**  INPUTS:             none
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:                        
**
**      status          A value indicating the status of the routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:
**
**      Update local_ip_addr_vec
**
**--
**/

PRIVATE void rpc__ip_init_local_addr_vec
#ifdef _DCE_PROTO_
(
    unsigned32 *status
)
#else
(status)
unsigned32 *status; 
#endif
{
    int                     desc;
    unsigned32              i;
    rpc_addr_vector_p_t     rpc_addr_vec = NULL;
    rpc_addr_vector_p_t     netmask_addr_vec = NULL;

    CODING_ERROR (status);

#ifdef HACK_DEBUG
    printf("rpc__ip_init_local_addr_vec - dunno what this does\n");
#endif
    /*
     * Open a socket to pass to "enumerate_interface".
     */
    desc = win32_socket(AF_INET, SOCK_DGRAM, 0);

    if (desc < 0) 
    {
        *status = rpc_s_cant_create_socket;   /* !!! */
#ifdef HACK_DEBUG
	    printf("rpc__ip_init_local_addr_vec - can't create socket\n");
#endif
        return;
    }

    enumerate_interfaces
        (RPC_C_PROTSEQ_ID_NCADG_IP_UDP, desc, 0,
         &rpc_addr_vec, &netmask_addr_vec, status);
    win32_close(desc);

#ifdef HACK_DEBUG
    printf("rpc__ip_init_local_addr_vec - enumerated interfaces: %lx\n",
		    (unsigned long)*status);
#endif

    if (*status != rpc_s_ok)
    {
        return;
    }

    /*
     * Do some sanity check.
     */

    if (rpc_addr_vec == NULL
        || netmask_addr_vec == NULL
        || rpc_addr_vec->len != netmask_addr_vec->len
        || rpc_addr_vec->len == 0)
    {
        RPC_DBG_GPRINTF(("(rpc__ip_init_local_addr_vec) no local address\n"));
        *status = rpc_s_no_addrs;
        goto free_rpc_addrs;
    }

    if (local_ip_addr_vec != NULL)
    {
      RPC_MEM_REALLOC (
        local_ip_addr_vec,
        rpc_ip_s_addr_vector_p_t,
        (sizeof *local_ip_addr_vec)
            + ((rpc_addr_vec->len - 1) * (sizeof (local_ip_addr_vec->elt[0]))),
        RPC_C_MEM_UTIL,
        RPC_C_MEM_WAITOK);
    }else
    {
      RPC_MEM_ALLOC (
        local_ip_addr_vec,
        rpc_ip_s_addr_vector_p_t,
        (sizeof *local_ip_addr_vec)
            + ((rpc_addr_vec->len - 1) * (sizeof (local_ip_addr_vec->elt[0]))),
        RPC_C_MEM_UTIL,
        RPC_C_MEM_WAITOK);
    }

    if (local_ip_addr_vec == NULL)
    {
        *status = rpc_s_no_memory;
        goto free_rpc_addrs;
    }

    local_ip_addr_vec->num_elt = rpc_addr_vec->len;

    for (i = 0; i < rpc_addr_vec->len; i++)
    {
        local_ip_addr_vec->elt[i].addr =
            ((rpc_ip_addr_p_t) rpc_addr_vec->addrs[i])->sa.sin_addr.s_addr;
        local_ip_addr_vec->elt[i].netmask =
            ((rpc_ip_addr_p_t) netmask_addr_vec->addrs[i])->sa.sin_addr.s_addr;
#ifdef HACK_DEBUG
        {
            char         buff[16], mbuff[16];
            unsigned8    *p, *mp;

            p = (unsigned8 *) &(local_ip_addr_vec->elt[i].addr);
            mp = (unsigned8 *) &(local_ip_addr_vec->elt[i].netmask);
            RPC__IP_NETWORK_SPRINTF(buff, "%d.%d.%d.%d",
                                    UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
            RPC__IP_NETWORK_SPRINTF(mbuff, "%d.%d.%d.%d",
                                    UC(mp[0]), UC(mp[1]), UC(mp[2]), UC(mp[3]));
            printf("(rpc__ip_init_local_addr_vec) local network [%s] netmask [%s]\n",
                            buff, mbuff);
        }
#endif
#ifdef DEBUG
        if (RPC_DBG2(rpc_e_dbg_general, 10))
        {
            char         buff[16], mbuff[16];
            unsigned8    *p, *mp;

            p = (unsigned8 *) &(local_ip_addr_vec->elt[i].addr);
            mp = (unsigned8 *) &(local_ip_addr_vec->elt[i].netmask);
            RPC__IP_NETWORK_SPRINTF(buff, "%d.%d.%d.%d",
                                    UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
            RPC__IP_NETWORK_SPRINTF(mbuff, "%d.%d.%d.%d",
                                    UC(mp[0]), UC(mp[1]), UC(mp[2]), UC(mp[3]));
            RPC_DBG_PRINTF(rpc_e_dbg_general, 10,
            ("(rpc__ip_init_local_addr_vec) local network [%s] netmask [%s]\n",
                            buff, mbuff));
        }
#endif
    }

free_rpc_addrs:
    if (rpc_addr_vec != NULL)
    {
        for (i = 0; i < rpc_addr_vec->len; i++)
        {
            RPC_MEM_FREE (rpc_addr_vec->addrs[i], RPC_C_MEM_RPC_ADDR);
        }
        RPC_MEM_FREE (rpc_addr_vec, RPC_C_MEM_RPC_ADDR_VEC);
    }
    if (netmask_addr_vec != NULL)
    {
        for (i = 0; i < netmask_addr_vec->len; i++)
        {
            RPC_MEM_FREE (netmask_addr_vec->addrs[i], RPC_C_MEM_RPC_ADDR);
        }
        RPC_MEM_FREE (netmask_addr_vec, RPC_C_MEM_RPC_ADDR_VEC);
    }
}

/*
**++
**
**  ROUTINE NAME:       rpc__ip_is_local_network
**
**  SCOPE:              PRIVATE - declared in ipnaf.h
**
**  DESCRIPTION:
**      
**  Return a boolean value to indicate if the given RPC address is on
**  the same IP subnet.
**
**
**  INPUTS:
**
**      rpc_addr        The address that forms the path of interest
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:                        
**
**      status          A value indicating the status of the routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:
**
**      result          true => the address is on the same subnet.
**                      false => not.
**
**  SIDE EFFECTS:       none
**
**--
**/
PRIVATE boolean32 rpc__ip_is_local_network
#ifdef _DCE_PROTO_
(
    rpc_addr_p_t rpc_addr,
    unsigned32   *status
)
#else
(rpc_addr, status)
rpc_addr_p_t rpc_addr;
unsigned32   *status; 
#endif
{
    rpc_ip_addr_p_t         ip_addr = (rpc_ip_addr_p_t) rpc_addr;
    unsigned32              addr1;
    unsigned32              addr2;
    unsigned32              i;

    CODING_ERROR (status);

    if (rpc_addr == NULL)
    {
        *status = rpc_s_invalid_arg;
        return false;
    }

    *status = rpc_s_ok;

    if (local_ip_addr_vec == NULL)
    {
        /*
         * We should call rpc__ip_init_local_addr_vec() here. But, it
         * requires the mutex lock for local_ip_addr_vec. For now just return
         * false.
         */
        return false;
    }

    /*
     * Compare addresses.
     */
    for (i = 0; i < local_ip_addr_vec->num_elt; i++)
    {
        if (ip_addr->sa.sin_family != AF_INET)
        {
            continue;
        }

        addr1 = ip_addr->sa.sin_addr.s_addr & local_ip_addr_vec->elt[i].netmask;
        addr2 = local_ip_addr_vec->elt[i].addr & local_ip_addr_vec->elt[i].netmask;

        if (addr1 == addr2)
        {
            return true;
        }
    }

    return false;
}

/*
**++
**
**  ROUTINE NAME:       rpc__ip_is_local_addr
**
**  SCOPE:              PRIVATE - declared in ipnaf.h
**
**  DESCRIPTION:
**      
**  Return a boolean value to indicate if the given RPC address is the
**  the local IP address.
**
**
**  INPUTS:
**
**      rpc_addr        The address that forms the path of interest
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:                        
**
**      status          A value indicating the status of the routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:
**
**      result          true => the address is local.
**                      false => not.
**
**  SIDE EFFECTS:       none
**
**--
**/

PRIVATE boolean32 rpc__ip_is_local_addr
#ifdef _DCE_PROTO_
(
    rpc_addr_p_t rpc_addr,
    unsigned32   *status
)
#else
(rpc_addr, status)
rpc_addr_p_t rpc_addr;
unsigned32   *status; 
#endif
{
    rpc_ip_addr_p_t         ip_addr = (rpc_ip_addr_p_t) rpc_addr;
    unsigned32              i;

    CODING_ERROR (status);

    if (rpc_addr == NULL)
    {
        *status = rpc_s_invalid_arg;
        return false;
    }

    *status = rpc_s_ok;

    if (local_ip_addr_vec == NULL)
    {
        /*
         * We should call rpc__ip_init_local_addr_vec() here. But, it
         * requires the mutex lock for local_ip_addr_vec. For now just return
         * false.
         */
        return false;
    }

    /*
     * Compare addresses.
     */
    for (i = 0; i < local_ip_addr_vec->num_elt; i++)
    {
        if (ip_addr->sa.sin_family != AF_INET)
        {
            continue;
        }

        if (ip_addr->sa.sin_addr.s_addr == local_ip_addr_vec->elt[i].addr)
        {
            return true;
        }
    }

    return false;
}
