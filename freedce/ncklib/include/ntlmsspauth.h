/* (c) Copyright 2001 Luke Kenneth Casson Leighton
 * NTLMSSP Authentication Implementation (see ISBN 1578701503)
 */

/*
 * 
 * (c) Copyright 1991 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1991 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1991 DIGITAL EQUIPMENT CORPORATION
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
#ifndef _NTLMSSPAUTH_H
#define _NTLMSSPAUTH_H	1
/*
**
**  NAME
**
**      ntlmsspauthp.h
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**      Types and routines private to the "ntlmsspauth" pseudo-authentication
**      module.
**
**
*/

#include <commonp.h>
#include <com.h>
#include <comp.h>
#include <dce/id_base.h>
#include <dce/sec_authn.h>

/*
 * This bogosity is necessary because of a collision
 * between wtypes.idl and the SAMBA code. Yuck, yuck.
 */
#define _BOOL
#define True (true)
#define False (false)

#include <includes.h>
#include <ntlmssp_api.h>
    


/*
 * For various reasons, it's painful to get at the NDR tag of the
 * underlying data, so we cheat and just encode it in big-endian order.
 */

#define rpc_marshall_be_long_int(mp, bei) \
{       long temp = htonl(bei);            \
        rpc_marshall_long_int (mp, temp);  \
}
      
#define rpc_convert_be_long_int(mp, bei) \
{                                       \
    rpc_unmarshall_long_int(mp, bei);   \
    bei = ntohl(bei);                   \
}

#define rpc_marshall_be_short_int(mp, bei) \
{       short temp = htons(bei);            \
        rpc_marshall_short_int (mp, temp);  \
}
      
#define rpc_convert_be_short_int(mp, bei) \
{                                       \
    rpc_unmarshall_short_int(mp, bei);   \
    bei = ntohs(bei);                   \
}
/*
 * We allow a little flexibility in whether we support one or both RPC
 * protocols for NTLMSSP authentication.  To simplify Makefiles, etc.
 * if neither RPC-protocol-specific symbol is defined, just assume we
 * want both.
 */

#if defined(AUTH_NTLMSSP) 

/* lkcl: disabled because i've never seen it on-wire so
 * have pretty much zero understanding of how to implement it
 * #define AUTH_NTLMSSP_DG
 */
#define AUTH_NTLMSSP_CN

#endif


/*
 * Max number of keys kept at once on each end of the conversation.
 * This assumes that keys are changed in an interval >> than the round
 * trip time between client and server.
 */

#define RPC__NTLMSSPAUTH_NKEYS 3

/*
 * State block containing all the state of one end of an authenticated
 * connection.
 */

typedef struct rpc_ntlmsspauth_info_t {
    rpc_auth_info_t auth_info;  /* This must be the first element. */
    rpc_mutex_t lock;
    unsigned32 status;          /* "poison" status. */

    unsigned_char_p_t client_name; /* client string name, if any */
    /* this should be NETLOGON_INFO_3, here.  but not now. */
    sec_id_pac_t client_pac;   /* client PAC */

    /* FAKE-EPAC */
    rpc_authz_cred_handle_t  client_creds;  /* 1.1 epac-style cred handle */
    
    int creds_valid: 1;         /* credentials valid */
    int level_valid: 1;         /* level valid */
    int client_valid: 1;        /* is client valid? */

    /* put addl flags here. */
    ntlmssp_sec_state_t ntlmssp; /* ntlmssp sign/seal state */
    
} rpc_ntlmsspauth_info_t, *rpc_ntlmsspauth_info_p_t;

/*
 * Locking macros.
 */

#define RPC_KRB_INFO_LOCK(info) RPC_MUTEX_LOCK ((info)->lock)
#define RPC_KRB_INFO_UNLOCK(info) RPC_MUTEX_UNLOCK ((info)->lock)

/*
 * Prototypes for PRIVATE routines.
 */

PRIVATE rpc_protocol_id_t       rpc__ntlmsspauth_cn_init _DCE_PROTOTYPE_ ((
         rpc_auth_rpc_prot_epv_p_t      * /*epv*/,
         unsigned32                     * /*st*/
    ));

#if 0
PRIVATE rpc_protocol_id_t       rpc__ntlmsspauth_dg_init _DCE_PROTOTYPE_ ((
         rpc_auth_rpc_prot_epv_p_t      * /*epv*/,
         unsigned32                     * /*st*/
    ));
#endif


/*
 * Prototypes for API EPV routines.
 */

void rpc__ntlmsspauth_bnd_set_auth _DCE_PROTOTYPE_ ((
        unsigned_char_p_t                   /* in  */    /*server_princ_name*/,
        rpc_authn_level_t                   /* in  */    /*authn_level*/,
        rpc_auth_identity_handle_t          /* in  */    /*auth_identity*/,
        rpc_authz_protocol_id_t             /* in  */    /*authz_protocol*/,
        rpc_binding_handle_t                /* in  */    /*binding_h*/,
        rpc_auth_info_p_t                   /* out */   * /*auth_info*/,
        unsigned32                          /* out */   * /*st*/
    ));

void rpc__ntlmsspauth_srv_reg_auth _DCE_PROTOTYPE_ ((
        unsigned_char_p_t                   /* in  */    /*server_princ_name*/,
        rpc_auth_key_retrieval_fn_t         /* in  */    /*get_key_func*/,
        pointer_t                           /* in  */    /*arg*/,
        unsigned32                          /* out */   * /*st*/
    ));

void rpc__ntlmsspauth_mgt_inq_def _DCE_PROTOTYPE_ ((
        unsigned32                          /* out */   * /*authn_level*/,
        unsigned32                          /* out */   * /*st*/
    ));

void rpc__ntlmsspauth_inq_my_princ_name _DCE_PROTOTYPE_ ((
        unsigned32                          /* in */     /*princ_name_size*/,
        unsigned_char_p_t                   /* out */    /*princ_name*/,
        unsigned32                          /* out */   * /*st*/
    ));

void rpc__ntlmsspauth_free_info _DCE_PROTOTYPE_((                         
        rpc_auth_info_p_t                   /* in/out */ * /*info*/
    ));

void rpc__ntlmsspauth_free_key _DCE_PROTOTYPE_ ((
        rpc_key_info_p_t                    /* in/out */ * /*key_info*/
    ));

error_status_t rpc__ntlmsspauth_resolve_identity _DCE_PROTOTYPE_ ((
        rpc_auth_identity_handle_t                 /* in */ /*in_identity*/,
        rpc_auth_identity_handle_t                 /* out */ *out_identity
    ));
    
void rpc__ntlmsspauth_release_identity _DCE_PROTOTYPE_ ((
        rpc_auth_identity_handle_t                 /* in/out */ * /*identity*/
    ));
    
/*
 * Miscellaneous internal entry points.
 */

sec_id_pac_t *rpc__ntlmsspauth_get_pac _DCE_PROTOTYPE_((void));

#endif /* _NTLMSSPAUTH_H */
