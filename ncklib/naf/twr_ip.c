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
#ifdef NAF_IP
/*
**
**  NAME
**
**      twr_ip.c
**
**  FACILITY:
**
**      Protocol Tower Services for the internet address family
**
**  ABSTRACT:
**
**      The protocol tower service provides routines that:
**      
**      o  convert a socket address to the lower floors of 
**         a protocol tower 
**
**      o  convert a protocol tower to a socket address
**
**
*/

#include <commonp.h>    /* Common declarations for all RPC runtime */
#include <com.h>        /* Common communications services */
#include <twrp.h>	/* Private tower services */

        
/*
 *  Include the Internet specific socket address
 */
#if defined(unix) || defined(__unix) || defined(__unix__)
#include <netinet/in.h>
#else
#include <in.h>
#endif

/*
**++
**
**  ROUTINE NAME:       twr_ip_lower_flrs_from_sa
**
**  SCOPE:              PUBLIC - declared in twr.idl
**
**  DESCRIPTION:
**
**  Creates the canonical representation of an internet protocol tower's 
**  lower floors from a sockadddr. The canonical form can be transmitted 
**  on the wire, or included in a DNS tower.
**
**  INPUTS:
**
**      trans_prot      Integer value specifying the transport layer 
**                      protocol for the Internet address family. 
**                      For address family RPC_C_NAF_ID_IP specify:
**
**                         RPC_C_NETWORK_PROTOCOL_ID_TCP for tcp
**                         RPC_C_NETWORK_PROTOCOL_ID_UDP for udp
**
**      sa              Internet-specific socket address data 
**                      structure.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      lower_flrs      Returns the lower tower floors in a twr_t 
**                      structure (includes the floor count in the 
**                      "tower_octet_string" member).
**
**      status          A value indicating the return status of the routine:
**                          twr_s_ok
**                          twr_s_unknown_sa
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     void
**
**  SIDE EFFECTS:       none
**
**--
**/

PUBLIC void twr_ip_lower_flrs_from_sa 
#ifdef _DCE_PROTO_
(
    unsigned32        trans_prot,
    sockaddr_p_t      sa,
    twr_p_t           *lower_flrs,
    unsigned32        *status
)
#else
(trans_prot, sa, lower_flrs, status)
unsigned32        trans_prot;
sockaddr_p_t      sa;
twr_p_t           *lower_flrs;
unsigned32        *status;
#endif
{
    unsigned8   protocol_id[TWR_C_NUM_IP_LOWER_FLRS];
    unsigned16  id_size = TWR_C_TOWER_PROT_ID_SIZE,
                floor_count,
                related_data_size[TWR_C_NUM_IP_LOWER_FLRS],
                twr_rep_16;
    unsigned32  count,
                twr_t_length;
    byte_p_t    related_data_ptr[TWR_C_NUM_IP_LOWER_FLRS],
                tmp_tower;

    CODING_ERROR (status);
    RPC_VERIFY_INIT ();

    /*
     * all depends on the network family to which this socket belongs
     */
    if (sa->family == RPC_C_NAF_ID_IP)
    {
        /* 
         * only two network protocols are supported for internet
         */
        if (trans_prot == RPC_C_NETWORK_PROTOCOL_ID_TCP)
        {
            protocol_id[0] = TWR_C_FLR_PROT_ID_TCP;
        }
        else
        {
            if ( trans_prot == RPC_C_NETWORK_PROTOCOL_ID_UDP )
            {
                protocol_id[0] = TWR_C_FLR_PROT_ID_UDP;
            }
            else 
            {
                *status = twr_s_unknown_sa;
                return;
            }
        }
        protocol_id[1] = TWR_C_FLR_PROT_ID_IP;
    }            
    else
    {
        *status = twr_s_unknown_sa;
        return;
    }

    /*
     * Since we now know the socket address we're dealing with, 
     * collect the sizes of each field to allocate memory, and 
     * remember the pointers to the fields so we can copy the 
     * data once we have allocated the tower string.
     */
    floor_count = TWR_C_NUM_IP_LOWER_FLRS;
            
    /*
     * Note that the port number and address are already
     * stored in big endian (network order) which is what
     * the architecture specifies, so no endian conversion 
     * is necessary.
     */
    related_data_size[0] = TWR_C_IP_PORT_SIZE;

    related_data_ptr[0] = 
        (byte_p_t) (&((struct sockaddr_in *)sa)->sin_port);
            
    related_data_size[1] = TWR_C_IP_ADDR_SIZE;
            
    related_data_ptr[1] = 
        (byte_p_t) (&((struct sockaddr_in *)sa)->sin_addr.s_addr);

    /*  
     * Calculate the length of the tower floors.
     */
     
    twr_t_length = TWR_C_TOWER_FLR_COUNT_SIZE;  /* to store floor count */

    for ( count = 0; count < floor_count; count++ )
    {
        twr_t_length += TWR_C_FLR_OVERHEAD;
        twr_t_length += related_data_size[count];
    }

    /* 
     * Next allocate space for the tower structure
     */
    RPC_MEM_ALLOC (
        *lower_flrs,
        twr_p_t,
        sizeof (twr_t) + (twr_t_length - 1),
        RPC_C_MEM_TOWER,
        RPC_C_MEM_WAITOK );

    /* 
     * Copy the length of the tower octet string into the tower structure
     */
    (*lower_flrs)->tower_length = twr_t_length;
    
    /* 
     * Copy the floor information into the tower octet string 
     */

    /*
     * Use a temporary for the octet string since we need
     * to increment the pointer.
     */
    tmp_tower = (*lower_flrs)->tower_octet_string;

    /* 
     * Copy the number of floors into the tower octet string 
     */
    twr_rep_16 = floor_count;
    RPC_RESOLVE_ENDIAN_INT16 (twr_rep_16);
    memcpy ((char *)tmp_tower, (char *)&twr_rep_16, 
            TWR_C_TOWER_FLR_COUNT_SIZE);

    tmp_tower += TWR_C_TOWER_FLR_COUNT_SIZE;

    /* 
     * Convert the protocol identifier size to its proper
     * representation for use in the following loop.
     */
    RPC_RESOLVE_ENDIAN_INT16 (id_size);

    for ( count = 0; count < floor_count; count++ )
    {
        /* 
         * Copy the length of the protocol identifier field into 
         * tower octet string.  (Converted before the loop.)
         */
        memcpy ((char *)tmp_tower, (char *)&id_size, 
                TWR_C_TOWER_FLR_LHS_COUNT_SIZE);

        tmp_tower += TWR_C_TOWER_FLR_LHS_COUNT_SIZE;


        /* 
         * Copy the protocol identifier into tower octet string
         * (1 byte so no need to convert endian representation).
         */
        memcpy ((char *)tmp_tower, (char *)&(protocol_id[count]),
                TWR_C_TOWER_PROT_ID_SIZE);

        tmp_tower += TWR_C_TOWER_PROT_ID_SIZE;

        /* 
         * Copy the length of the address data field into 
         * tower octet string.
         */
        twr_rep_16 = related_data_size[count];
        RPC_RESOLVE_ENDIAN_INT16 (twr_rep_16);
        memcpy ((char *)tmp_tower, (char *)&twr_rep_16,
                TWR_C_TOWER_FLR_RHS_COUNT_SIZE);
 
        tmp_tower += TWR_C_TOWER_FLR_RHS_COUNT_SIZE;

        /* 
         * If there is addressing data, copy the address data field into 
         * tower octet string
         */
        if (related_data_size[count])
        {
            memcpy ((char *)tmp_tower, (char *)related_data_ptr[count], 
                    related_data_size[count]);

            /*
             * Set up for the next floor.
             */
            tmp_tower += related_data_size[count];
        }
    }

    *status = twr_s_ok;
}

/*
**++
**
**  ROUTINE NAME:       twr_ip_lower_flrs_to_sa 
**
**  SCOPE:              PUBLIC - declared in twr.idl
**
**  DESCRIPTION:
**
**  Creates an internet sockaddr from the canonical representation of an 
**  internet protocol tower's lower floors.
**
**  INPUTS:
**
**      tower_octet_string
**                      The protocol tower to convert to a sockaddr.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      sa            Returns a pointer to the created sockaddr structure.
**      
**      sa_len          Returns the length, in bytes, of the returned 
**                      "sa" argument.
**
**      status          A value indicating the return status of the routine:
**                          twr_s_ok
**                          twr_s_unknown_tower
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     void
**
**  SIDE EFFECTS:       none
**
**--
**/

PUBLIC void twr_ip_lower_flrs_to_sa 
#ifdef _DCE_PROTO_
(
    byte_p_t          tower_octet_string, 
    sockaddr_p_t      *sa,
    unsigned32        *sa_len,
    unsigned32        *status
)
#else
( tower_octet_string, sa, sa_len, status )
byte_p_t          tower_octet_string; 
sockaddr_p_t      *sa;
unsigned32        *sa_len;
unsigned32        *status;
#endif
{
    unsigned8   id;
    byte_p_t    tower;
    unsigned16  count,
                floor_count,
                id_size,
                addr_size;
    unsigned32  length;

    CODING_ERROR (status);
    RPC_VERIFY_INIT ();

    /* 
     * Make sure we have a pointer to some data structure.
     */
    if ( !(tower = tower_octet_string)) 
    {
        *status = twr_s_unknown_tower;
        return;
    }

    /* 
     * Get the tower floor count 
     */
    memcpy ((char *)&floor_count, (char *)tower, TWR_C_TOWER_FLR_COUNT_SIZE);
    RPC_RESOLVE_ENDIAN_INT16 (floor_count);

    tower += TWR_C_TOWER_FLR_COUNT_SIZE;

    /*
     * Skip over the (application's) upper floors while we look for the 
     * beginning of the ip-specific lower floors.
     */
    for ( count = 0; count < floor_count; count++ )
    {
        /*
         * Get the length of this floor's protocol id field (don't advance
         * the pointer).
         */
        memcpy ((char *)&id_size, (char *)tower, 
                TWR_C_TOWER_FLR_LHS_COUNT_SIZE);
        RPC_RESOLVE_ENDIAN_INT16 (id_size);

        /*
         * Get the protocol id (don't advance the pointer).
         * Expect one byte; no need to convert.
         */
        memcpy ((char *)&id, (char *)(tower + TWR_C_TOWER_FLR_LHS_COUNT_SIZE),
                TWR_C_TOWER_PROT_ID_SIZE);

        /*
         * See if we support the protocol id.
         */
        if ( (id_size == TWR_C_TOWER_PROT_ID_SIZE) && 
             (id == TWR_C_FLR_PROT_ID_TCP || id ==  TWR_C_FLR_PROT_ID_UDP))
        {
            /*
             * Indicate we found the beginning of the ip floors.
             */
            *status = twr_s_ok;

            break;
        }
        else
        {
            /*
             * Skip this floor.  Get the address size in order
             * to know how much to skip.
             */
            memcpy ((char *)&addr_size, 
                    (char *)(tower + TWR_C_TOWER_FLR_LHS_COUNT_SIZE +
                            id_size), TWR_C_TOWER_FLR_RHS_COUNT_SIZE);
            RPC_RESOLVE_ENDIAN_INT16 (addr_size);

            tower += TWR_C_TOWER_FLR_LHS_COUNT_SIZE + id_size +
                     TWR_C_TOWER_FLR_RHS_COUNT_SIZE + addr_size;

            /*
             * For now, assume we don't find the floors we're looking for.
             */
            *status = twr_s_unknown_tower;
        }
    }

    if (*status != twr_s_ok)
    {
        return;
    }

    /*
     * Skip the floor's protocol id field length and protocol id
     * (now move the pointer).  We already know it's
     * TWR_C_FLR_PROT_ID_TCP or TWR_C_FLR_PROT_ID_UDP.
     */
    tower += (TWR_C_TOWER_FLR_LHS_COUNT_SIZE + id_size);


    /* 
     * Allocate space for ip sockaddr
     */
    length = sizeof(struct sockaddr_in);
                    
    RPC_MEM_ALLOC (
        *sa,
        sockaddr_p_t,
        length,
        RPC_C_MEM_SOCKADDR,
        RPC_C_MEM_WAITOK );
    
    *sa_len = length;
                    
    /*
     * make sure unused bytes are null 
     */
    memset ((char *) *sa, 0, length);
                    
    /* 
     * define this as an internet family socket
     */
    ((struct sockaddr_in *)(*sa))->sin_family = RPC_C_NAF_ID_IP;
                    
    /* 
     * Get the length of in_port
     */
    memcpy ((char *)&addr_size, (char *)tower, RPC_C_TOWER_FLR_RHS_COUNT_SIZE);
    RPC_RESOLVE_ENDIAN_INT16 (addr_size);
    tower += RPC_C_TOWER_FLR_RHS_COUNT_SIZE;

    /*
     * Copy the port number to the sockaddr.
     */
    memcpy ( &((struct sockaddr_in *)(*sa))->sin_port, tower, addr_size);
                    
    tower += addr_size;
                    
    /* 
     * Get the length of host address floor protocol id
     */
    memcpy ((char *)&id_size, (char *)tower, 
            TWR_C_TOWER_FLR_LHS_COUNT_SIZE);
    RPC_RESOLVE_ENDIAN_INT16 (id_size);
    tower  += TWR_C_TOWER_FLR_LHS_COUNT_SIZE;

    /* 
     * Get the protocol id
     * Expect one byte; no need to convert.
     */
    memcpy ((char *)&id, (char *)tower, TWR_C_TOWER_PROT_ID_SIZE);
    tower += id_size;

    if ( (id_size != TWR_C_TOWER_PROT_ID_SIZE) || 
         (id != TWR_C_FLR_PROT_ID_IP) )
    {
        *status = twr_s_unknown_tower;

        RPC_MEM_FREE (*sa, RPC_C_MEM_SOCKADDR);

        return;
    }
    
    /* 
     * Get the length of in_address
     */
    memcpy ((char *)&addr_size, (char *)tower, RPC_C_TOWER_FLR_RHS_COUNT_SIZE);
    RPC_RESOLVE_ENDIAN_INT16 (addr_size);
    tower += RPC_C_TOWER_FLR_RHS_COUNT_SIZE;

    /*
     * Copy the host address to the sockaddr
     */
    memcpy (&((struct sockaddr_in *)(*sa))->sin_addr.s_addr, 
            (char *)tower, addr_size);
                    
    *status = twr_s_ok;
}
#else 
static int _naf_ip_dummy_ = 0 ;
#endif /* NAF_IP */
