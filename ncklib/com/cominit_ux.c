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
**  NAME
**
**      cominit_ux.c
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  Initialization Service support routines for Unix platforms.
**
*/

#include <commonp.h>
#include <com.h>
#include <comprot.h>
#include <comnaf.h>
#include <comp.h>
#include <cominitp.h>


/* 
 * Routines for loading Network Families and Protocol Families as shared
 * images.  i.e., VMS
 */

PRIVATE rpc_naf_init_fn_t  rpc__load_naf
#ifdef _DCE_PROTO_
(
  rpc_naf_id_elt_p_t      naf __attribute__((__unused__)),
  unsigned32              *status
)
#else
(naf, status)
rpc_naf_id_elt_p_t      naf;
unsigned32              *status;
#endif
{
    *status = rpc_s_ok;
    return((rpc_naf_init_fn_t)NULL);
}


PRIVATE rpc_prot_init_fn_t  rpc__load_prot
#ifdef _DCE_PROTO_
(
    rpc_protocol_id_elt_p_t prot __attribute__((__unused__)),
    unsigned32              *status
)
#else
(prot, status)
rpc_protocol_id_elt_p_t prot;
unsigned32              *status;
#endif
{
    *status = rpc_s_ok;
    return((rpc_prot_init_fn_t)NULL);
}

PRIVATE rpc_auth_init_fn_t  rpc__load_auth
#ifdef _DCE_PROTO_
(
    rpc_authn_protocol_id_elt_p_t auth __attribute__((__unused__)),
    unsigned32              *status
)
#else
(auth, status)
rpc_authn_protocol_id_elt_p_t auth;
unsigned32              *status;
#endif
{
    *status = rpc_s_ok;
    return((rpc_auth_init_fn_t)NULL);
}
