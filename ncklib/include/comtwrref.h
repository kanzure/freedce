/*
 * 
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1990 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1990 DIGITAL EQUIPMENT CORPORATION
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
#ifndef _COMTWRREF_H
#define _COMTWRREF_H
/*
**
**
**  NAME:
**
**      nsrttwr.h
**
**  FACILITY:
**
**      Remote Procedure Call (RPC)
**  
**  ABSTRACT:
**
**      Header file containing macros, definitions, typedefs and prototypes of
**      exported routines from the nsrttwr.c module.
**  
**
**/

#ifndef _DCE_PROTOTYPE_
#include <dce/dce.h>
#endif


/*
 * Type Definitions
 */

/*
 * Protocol Sequence ID Table
 * 
 * This table contains the valid combination of protocol ids
 * for upper floor 3 and the lower tower floors.
 * This table maps each combination to the appropriate 
 * RPC protocol id sequence.  
 * The field num_floors provides for the number of significant
 * floors comprising the RPC protocol sequence.
 */
typedef struct
{
    unsigned8               prefix;
    uuid_t                  uuid;
} rpc_flr_prot_id_t, *rpc_flr_prot_id_p_t;

typedef struct
{
    rpc_protseq_id_t        rpc_protseq_id;
    unsigned8               num_floors;
    rpc_flr_prot_id_t       floor_prot_ids[RPC_C_MAX_NUM_NETWORK_FLOORS + 1];
} rpc_tower_prot_ids_t, *rpc_tower_prot_ids_p_t;

/*
 * Prototypes
 */

#ifdef __cplusplus
extern "C" {
#endif


PRIVATE void rpc__tower_ref_add_floor _DCE_PROTOTYPE_ ((
    unsigned32           /*floor_number*/,
    rpc_tower_floor_p_t  /*floor*/,
    rpc_tower_ref_t     * /*tower_ref*/,
    unsigned32          * /*status*/ 
));

PRIVATE void rpc__tower_ref_alloc _DCE_PROTOTYPE_ ((
    unsigned8           * /*tower_octet_string*/,
    unsigned32           /*num_flrs*/,
    unsigned32           /*start_flr*/,
    rpc_tower_ref_p_t   * /*tower_ref*/,
    unsigned32          * /*status*/ 
));

PRIVATE void rpc__tower_ref_copy _DCE_PROTOTYPE_ ((
    rpc_tower_ref_p_t    /*source_tower*/,
    rpc_tower_ref_p_t   * /*dest_tower*/,
    unsigned32          * /*status*/ 
));

PRIVATE void rpc__tower_ref_free _DCE_PROTOTYPE_ ((
    rpc_tower_ref_p_t       * /*tower_ref*/,
    unsigned32              * /*status*/ 
));

PRIVATE void rpc__tower_ref_inq_protseq_id _DCE_PROTOTYPE_ ((
    rpc_tower_ref_p_t    /*tower_ref*/,
    rpc_protseq_id_t    * /*protseq_id*/,
    unsigned32          * /*status*/ 
));

PRIVATE boolean rpc__tower_ref_is_compatible _DCE_PROTOTYPE_ ((
    rpc_if_rep_p_t           /*if_spec*/,
    rpc_tower_ref_p_t        /*tower_ref*/,
    unsigned32              * /*status*/ 
));

PRIVATE void rpc__tower_ref_vec_free _DCE_PROTOTYPE_ ((
    rpc_tower_ref_vector_p_t    * /*tower_vector*/,
    unsigned32                  * /*status*/ 
));

PRIVATE void rpc__tower_ref_vec_from_binding _DCE_PROTOTYPE_ ((
    rpc_if_rep_p_t               /*if_spec*/,
    rpc_binding_handle_t         /*binding*/,
    rpc_tower_ref_vector_p_t    * /*tower_vector*/,
    unsigned32                  * /*status*/ 
));

#endif /* _COMTWRREF_H */

