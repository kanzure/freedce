/* (c) Copyright 2001 Luke Kenneth Casson Leighton
 * NTLMSSP Authentication Implementation (see ISBN 1578701503)
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
 */
#ifndef _NTLMSSPAUTHCN_H
#define _NTLMSSPAUTHCN_H 	1

/*
**  NAME
**
**      ntlmsspauthcn.h
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  The ntlmsspauth CN authentication module interface.
**
**
*/

#include <cn.h> 

typedef struct 
{
    rpc_cn_auth_info_t  cn_info;

    /*
     * NTLMSSPAUTH specific fields here.
     */

    int auth_sequence; /* client sets to 1 to send AUTH3 */
                       /* see rpc__ntlmsspauth_fmt_client_req */

} rpc_ntlmsspauth_cn_info_t, *rpc_ntlmsspauth_cn_info_p_t;

EXTERNAL rpc_cn_auth_epv_t rpc_g_ntlmsspauth_cn_epv;

#endif /* _NTLMSSPAUTHCN_H */
