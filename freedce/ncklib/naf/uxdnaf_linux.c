/* (c) Copyright 2001 Luke Kenneth Casson Leighton
 * Unix Domain Socket NAF extensions.  based on uxdnaf_linux.c.
 */

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
**      uxdnaf_bsd
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  This module contains routines specific to the Internet Protocol,
**  the Internet Network Address Family extension service, and the
**  Linux.
**
**  - taken from uxdnaf_bsd.c
**
**
**
*/

#include <commonp.h>
#include <com.h>
#include <comnaf.h>
#include <comsoc.h>
#include <uxdnaf.h>

#include <sys/ioctl.h>


/***********************************************************************
 *
 *  Internal prototypes and typedefs.
 */

#ifndef NO_SPRINTF
#  define RPC__IP_NETWORK_SPRINTF   sprintf
#else
#  define RPC__IP_NETWORK_SPRINTF   rpc__uxd_network_sprintf
#endif


/*
**++
**
**  ROUTINE NAME:       rpc__uxd_init_local_addr_vec
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
**      Update local_uxd_addr_vec
**
**--
**/

PRIVATE void rpc__uxd_init_local_addr_vec
#ifdef _DCE_PROTO_
(
    unsigned32 *status
)
#else
(status)
unsigned32 *status; 
#endif
{
    CODING_ERROR (status);

    *status = rpc_s_ok;
    return;
}

/*
**++
**
**  ROUTINE NAME:       rpc__uxd_desc_inq_addr
**
**  SCOPE:              PRIVATE - declared in uxdnaf.h
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

PRIVATE void rpc__uxd_desc_inq_addr 
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
    rpc_uxd_addr_p_t         ip_addr;
    rpc_uxd_addr_t           loc_uxd_addr;


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
    loc_uxd_addr.len = sizeof (rpc_uxd_addr_t);
    RPC_SOCKET_FIX_ADDRLEN(&loc_uxd_addr);

    if (getsockname (desc, (struct sockaddr *)&loc_uxd_addr.sa,
                     (unsigned int *)&loc_uxd_addr.len) < 0)
    {
        *status = -1;   /* !!! */
        return;
    }

    RPC_SOCKET_FIX_ADDRLEN(&loc_uxd_addr);

	RPC_MEM_ALLOC (
		ip_addr,
		rpc_uxd_addr_p_t,
		sizeof (rpc_uxd_addr_t),
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
	ip_addr->len            = sizeof (struct sockaddr_un);
	ip_addr->sa             = loc_uxd_addr.sa;

	(*rpc_addr_vec)->len = 1;
	(*rpc_addr_vec)->addrs[0] = (rpc_addr_p_t) ip_addr;

	*status = rpc_s_ok;
	return;
}

/*
**++
**
**  ROUTINE NAME:       rpc__uxd_get_broadcast
**
**  SCOPE:              PRIVATE - EPV declared in uxdnaf.h
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

PRIVATE void rpc__uxd_get_broadcast 
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
	naf_id = 0;
	protseq_id = 0;
	rpc_addr_vec = NULL;

    CODING_ERROR (status);

    *status = rpc_s_cant_inq_socket;   
    return;
}
/*
**++
**
**  ROUTINE NAME:       rpc__uxd_is_local_network
**
**  SCOPE:              PRIVATE - declared in uxdnaf.h
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
PRIVATE boolean32 rpc__uxd_is_local_network
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
    CODING_ERROR (status);

    if (rpc_addr == NULL)
    {
        *status = rpc_s_invalid_arg;
        return false;
    }

    *status = rpc_s_ok;

    return true;
}

/*
**++
**
**  ROUTINE NAME:       rpc__uxd_is_local_addr
**
**  SCOPE:              PRIVATE - declared in uxdnaf.h
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

PRIVATE boolean32 rpc__uxd_is_local_addr
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
    CODING_ERROR (status);

    if (rpc_addr == NULL)
    {
        *status = rpc_s_invalid_arg;
        return false;
    }

    *status = rpc_s_ok;

    return true;
}
