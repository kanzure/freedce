/*
 *
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
 * (c) Copyright 2001 PADL SOFTWARE PTY. LTD.
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *    		   permission to use, copy, modify, and distribute this
 * file for any purpose is hereby granted without fee, provided that
 * the above copyright notices and this notice appears in all source
 * code copies, and that none of the names of Open Software
 * Foundation, Inc., Hewlett-Packard Company, Digital Equipment
 * Corporation or PADL Software Pty. Ltd. be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  Neither Open Software
 * Foundation, Inc., Hewlett-Packard Company, Digital Equipment
 * Corporation nor PADL Software Pty. Ltd. makes any
 * representations about the suitability of this software for any
 * purpose.
 *
 */
/*
 */
/*
**  NAME
**
**      gssauthcn.c
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  The gssauth CN authentication module.
**
**
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define NCK_NEED_MARSHALLING 1

#include <dg.h>
#include <dce/rpc.h>
#include <dce/conv.h>
#include <dce/stubbase.h>
#include <dce/idl_es.h>

#include <pickle.h>
#include <gssauthcn.h>
#include <gssauth.h>

#include <ctype.h>
#include "crc32.h"

/* temporary prototype */
#ifdef DEBUG
extern void dump_data(int level, const char *buf, int len);
#endif /* DEBUG */

INTERNAL boolean32 rpc__gssauth_cn_three_way _DCE_PROTOTYPE_((void));

INTERNAL boolean32 rpc__gssauth_cn_context_valid _DCE_PROTOTYPE_((
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_create_info _DCE_PROTOTYPE_((
       rpc_authn_level_t				 /*authn_level*/,
       rpc_authn_protocol_id_t			protocol,
       rpc_auth_info_p_t				* /*auth_info*/,
       unsigned32					   * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_create_info_negotiate _DCE_PROTOTYPE_((
       rpc_authn_level_t				 /*authn_level*/,
       rpc_auth_info_p_t				* /*auth_info*/,
       unsigned32					   * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_create_info_mskrb _DCE_PROTOTYPE_((
       rpc_authn_level_t				 /*authn_level*/,
       rpc_auth_info_p_t				* /*auth_info*/,
       unsigned32					   * /*st*/
    ));

INTERNAL boolean32 rpc__gssauth_cn_cred_changed _DCE_PROTOTYPE_((
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_cred_refresh _DCE_PROTOTYPE_((
    	rpc_auth_info_p_t				/*auth_info*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_fmt_client_req _DCE_PROTOTYPE_((
    rpc_cn_assoc_sec_context_p_t	 /*assoc_sec*/,
    rpc_cn_sec_context_p_t		   /*sec*/,
    pointer_t						/*auth_value*/,
    unsigned32					  * /*auth_value_len*/,
    pointer_t					   * /*last_auth_pos*/,
    unsigned32					  * /*auth_len_remain*/,
    unsigned32					   /*old_server*/,
    unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_fmt_srvr_resp _DCE_PROTOTYPE_((
    	unsigned32					   /*verify_st*/,
    	rpc_cn_assoc_sec_context_p_t	 /*assoc_sec*/,
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	pointer_t						/*req_auth_value*/,
    	unsigned32					   /*req_auth_value_len*/,
    	pointer_t						/*auth_value*/,
    	unsigned32					  * /*auth_value_len*/
    ));

INTERNAL void rpc__gssauth_cn_free_prot_info _DCE_PROTOTYPE_((
    	rpc_auth_info_p_t				/*info*/,
    	rpc_cn_auth_info_p_t			* /*cn_info*/
    ));

INTERNAL void rpc__gssauth_cn_get_prot_info _DCE_PROTOTYPE_((
    	rpc_auth_info_p_t				/*info*/,
    	rpc_cn_auth_info_p_t			* /*cn_info*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_pre_call _DCE_PROTOTYPE_((
    	rpc_cn_assoc_sec_context_p_t	 /*assoc_sec*/,
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	pointer_t						/*auth_value*/,
    	unsigned32					  * /*auth_value_len*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_pre_send _DCE_PROTOTYPE_((
    	rpc_cn_assoc_sec_context_p_t	 /*assoc_sec*/,
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	rpc_socket_iovec_p_t			 /*iov*/,
    	unsigned32					   /*iovlen*/,
    	rpc_socket_iovec_p_t			 /*out_iov*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_recv_check _DCE_PROTOTYPE_((
    	rpc_cn_assoc_sec_context_p_t	 /*assoc_sec*/,
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	rpc_cn_common_hdr_p_t			/*pdu*/,
    	unsigned32					   /*pdu_len*/,
    	unsigned32					   /*cred_len*/,
    	rpc_cn_auth_tlr_p_t			  /*auth_tlr*/,
    	boolean32						/*unpack_ints*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_tlr_uuid_crc _DCE_PROTOTYPE_((
    	pointer_t				/*auth_value*/,
    	unsigned32			   /*auth_value_len*/,
    	unsigned32			  * /*uuid_crc*/
    ));

INTERNAL void rpc__gssauth_cn_tlr_unpack _DCE_PROTOTYPE_((
    	rpc_cn_packet_p_t		/*pkt_p*/,
    	unsigned32			   /*auth_value_len*/,
    	unsigned8			   * /*packed_drep*/
    ));

INTERNAL void rpc__gssauth_cn_vfy_client_req _DCE_PROTOTYPE_((
    	rpc_cn_assoc_sec_context_p_t	 /*assoc_sec*/,
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	pointer_t						/*auth_value*/,
    	unsigned32					   /*auth_value_len*/,
    	unsigned32				 /*old_client*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_cn_vfy_srvr_resp _DCE_PROTOTYPE_((
    	rpc_cn_assoc_sec_context_p_t	 /*assoc_sec*/,
    	rpc_cn_sec_context_p_t		   /*sec*/,
    	pointer_t						/*auth_value*/,
    	unsigned32					   /*auth_value_len*/,
    	unsigned32					  * /*st*/
    ));

INTERNAL void rpc__gssauth_init_sec_context _DCE_PROTOTYPE_((
    rpc_cn_assoc_sec_context_p_t assoc_sec,
    rpc_cn_sec_context_p_t sec,
    pointer_t req_auth_value,
    unsigned32 req_auth_value_len,
    pointer_t *auth_value,
    unsigned32 *auth_value_len,
    unsigned32 *st));

INTERNAL void rpc__gssauth_accept_sec_context _DCE_PROTOTYPE_((
    rpc_cn_assoc_sec_context_p_t assoc_sec,
    rpc_cn_sec_context_p_t sec,
    pointer_t req_auth_value,
    unsigned32 req_auth_value_len,
    pointer_t *auth_value,
    unsigned32 *auth_value_len,
    unsigned32 *st));

INTERNAL void rpc__gssauth_map_error _DCE_PROTOTYPE_((OM_uint32 min_stat, OM_uint32 maj_stat, unsigned32 *st));

#ifdef DEBUG
INTERNAL void rpc__gssauth_dbg_log_ex _DCE_PROTOTYPE_((OM_uint32 res, int type));
INTERNAL void rpc__gssauth_dbg_log _DCE_PROTOTYPE_((OM_uint32 res, OM_uint32 min_stat));
#endif

GLOBAL rpc_mutex_t rpc_g_gss_lock = RPC_MUTEX_INITIALIZER;

GLOBAL rpc_cn_auth_epv_t rpc_g_gssauth_negotiate_cn_epv =
{
    rpc__gssauth_cn_three_way,
    rpc__gssauth_cn_context_valid,
    rpc__gssauth_cn_create_info_negotiate,
    rpc__gssauth_cn_cred_changed,
    rpc__gssauth_cn_cred_refresh,
    rpc__gssauth_cn_fmt_client_req,
    rpc__gssauth_cn_fmt_srvr_resp,
    rpc__gssauth_cn_free_prot_info,
    rpc__gssauth_cn_get_prot_info,
    rpc__gssauth_cn_pre_call,
    rpc__gssauth_cn_pre_send,
    rpc__gssauth_cn_recv_check,
    rpc__gssauth_cn_tlr_uuid_crc,
    rpc__gssauth_cn_tlr_unpack,
    rpc__gssauth_cn_vfy_client_req,
    rpc__gssauth_cn_vfy_srvr_resp
};

GLOBAL rpc_cn_auth_epv_t rpc_g_gssauth_mskrb_cn_epv =
{
    rpc__gssauth_cn_three_way,
    rpc__gssauth_cn_context_valid,
    rpc__gssauth_cn_create_info_mskrb,
    rpc__gssauth_cn_cred_changed,
    rpc__gssauth_cn_cred_refresh,
    rpc__gssauth_cn_fmt_client_req,
    rpc__gssauth_cn_fmt_srvr_resp,
    rpc__gssauth_cn_free_prot_info,
    rpc__gssauth_cn_get_prot_info,
    rpc__gssauth_cn_pre_call,
    rpc__gssauth_cn_pre_send,
    rpc__gssauth_cn_recv_check,
    rpc__gssauth_cn_tlr_uuid_crc,
    rpc__gssauth_cn_tlr_unpack,
    rpc__gssauth_cn_vfy_client_req,
    rpc__gssauth_cn_vfy_srvr_resp
};

#ifdef DEBUG
INTERNAL void rpc__gssauth_dbg_log_ex(OM_uint32 res, int type)
{
    OM_uint32 maj_stat, min_stat;
    gss_buffer_desc msg_token;
    OM_uint32 msg_ctx;

    msg_ctx = 0;
    while (1) {
        maj_stat = gss_display_status(&min_stat, res, type,
            GSS_C_NULL_OID, &msg_ctx, &msg_token);
        RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_dbg_log_ex) %s\n", msg_token.value));
        gss_release_buffer(&min_stat, &msg_token);
        if (msg_ctx == 0)
           break;
    } 
}

INTERNAL void rpc__gssauth_dbg_log(OM_uint32 maj_stat, OM_uint32 min_stat)
{
    rpc__gssauth_dbg_log_ex(maj_stat, GSS_C_GSS_CODE);
    rpc__gssauth_dbg_log_ex(min_stat, GSS_C_MECH_CODE);
}
#endif

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_map_error
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Map a GSS-API status code to a RPC error code.
**
**  INPUTS:    		 Minor and major status from GSS-API
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		RPC status
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
INTERNAL void rpc__gssauth_map_error(OM_uint32 min_stat __attribute__((__unused__)),
    OM_uint32 maj_stat, unsigned32 *st)
{
    switch (GSS_ROUTINE_ERROR(maj_stat)) {
    	case GSS_S_BAD_MECH:
    		*st = rpc_s_unknown_authn_service;
    		break;
    	case GSS_S_BAD_NAME:
    		*st = rpc_s_incomplete_name;
    		break;
    	case GSS_S_BAD_NAMETYPE:
    		*st = rpc_s_invalid_name_syntax;
    		break;
    	case GSS_S_BAD_BINDINGS:
    		*st = rpc_s_invalid_binding;
    		break;
    	case GSS_S_BAD_STATUS:
    		*st = rpc_s_unknown_status_code;
    		break;
    	case GSS_S_BAD_SIG:
    		*st = rpc_s_auth_bad_integrity;
    		break;
    	case GSS_S_NO_CRED:
    	case GSS_S_DEFECTIVE_CREDENTIAL:
    		*st = rpc_s_invalid_credentials;
    		break;
    	case GSS_S_NO_CONTEXT:
    		*st = rpc_s_invalid_inquiry_context;
    		break;
    	case GSS_S_DEFECTIVE_TOKEN:
    		*st = rpc_s_authn_challenge_malformed;
    		break;
    	case GSS_S_CONTEXT_EXPIRED:
    		*st = rpc_s_auth_tkt_expired;
    		break;
    	case GSS_S_FAILURE:
    	case GSS_S_BAD_QOP:
    	case GSS_S_UNAUTHORIZED:
    	case GSS_S_DUPLICATE_ELEMENT:
    	case GSS_S_NAME_NOT_MN:
    	default:
    		*st = rpc_s_auth_mut_fail;
    		break;
    }
#ifdef DEBUG
    rpc__gssauth_dbg_log(maj_stat, min_stat);
#endif
}

/*****************************************************************************/
/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_three_way
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Determine whether the authentication protocol requires a
**      3-way authentication handshake. If true the client is expected to
**      provide an rpc_auth3 PDU before the security context is fully
**      established and a call can be made.
**
**  INPUTS:    		 none
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		none
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     boolean32
**
**      True if the authentication protocol requires a 3-way
**      authentication handshake.
**
**  SIDE EFFECTS:       none
**
**--
**/

INTERNAL boolean32 rpc__gssauth_cn_three_way (void)
{
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_three_way)\n"));

    /* Ah, DCE... */

    return (true);
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_context_valid
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Determine whether the established security context will be
**      valid (i. e. timely) for the next 300 seconds. If
**      not this routine will try to renew the context.
**      If it cannot be renewed false is returned. This is
**      called from the client side.
**
**  INPUTS:
**
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     boolean32
**
**      True if security context identified by the auth
**      information rep and RPC auth information rep will
**      still be valid in 300 seconds, false if not.
**
**  SIDE EFFECTS:       
**
**      The context may be renewed.
**
**--
**/

INTERNAL boolean32 rpc__gssauth_cn_context_valid 
#ifdef _DCE_PROTO_
(
    rpc_cn_sec_context_p_t		  sec,
    unsigned32					  *st
)
#else
(sec, st)
rpc_cn_sec_context_p_t    	  sec;
unsigned32    				  *st;
#endif
{
    rpc_gssauth_info_p_t		gssauth_info;
    rpc_gssauth_cn_info_p_t		gssauth_cn_info;
    OM_uint32					lifetime, maj_stat, min_stat;

    CODING_ERROR (st);

    gssauth_info = (rpc_gssauth_info_p_t) (sec->sec_info);
    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_context_valid)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_GENERAL,
    				("(rpc__gssauth_cn_context_valid) time->%x\n", time));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_context_valid) prot->%x level->%x key_id->%x\n",
    				sec->sec_info->authn_protocol,
    				sec->sec_info->authn_level,
    				sec->sec_key_id));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_CONTEXT_VALID))
    {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return false;
    }
#endif

    maj_stat = gss_inquire_cred(&min_stat, gssauth_info->server_creds,
    	NULL, &lifetime, NULL, NULL);
    if (GSS_ERROR(maj_stat)) {
    	rpc__gssauth_map_error(maj_stat, min_stat, st);
    	return false;
    }

    *st = rpc_s_ok;

    if (lifetime < 300) {
#if 0
    	maj_stat = gss_release_cred(&min_stat, &gssauth_info->server_creds);
    	if (GSS_ERROR(maj_stat)) {
    		rpc__gssauth_map_error(maj_stat, min_stat, st);
    		return false;
    	}
#endif
    	return false;
    }
    return true;
}


/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_create_info
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Create an auth information rep data structure with and
**      add a reference to it. This is called on the server
**      side. The fields will be initialized to NULL values.
**      The caller should fill them in based on information
**      decrypted from the authenticator passed in the bind
**      request.
**
**  INPUTS:
**
**      authn_level	 The authentication level to be applied over this
**    				  security context.
**
**      protocol	      The authentication protocol (SPNEGO or Kerberos) for
**    				  security context.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      info			A pointer to the auth information rep structure
**    				  containing RPC protocol indenpendent information.
**      st			  The return status of this routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       
**
**      The newly create auth info will have a reference count of 1.
**
**--
**/

INTERNAL void rpc__gssauth_cn_create_info 
#ifdef _DCE_PROTO_
(
    rpc_authn_level_t				authn_level,
    rpc_authn_protocol_id_t			protocol,
    rpc_auth_info_p_t				*auth_info,
    unsigned32					   *st
)
#else
(authn_level, protocol, auth_info, st)
rpc_authn_level_t    			authn_level;
rpc_authn_protocol_id_t    		protocol;
rpc_auth_info_p_t    			*auth_info;
unsigned32    				   *st;
#endif
{
    rpc_gssauth_info_p_t gssauth_info;

    CODING_ERROR (st);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_create_info)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_create_info) prot->%x level->%x\n",
    				protocol,
    				authn_level));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_CREATE_INFO))
    {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    /*
     * Allocate storage for a gssauth info structure from heap.
     */
    RPC_MEM_ALLOC (gssauth_info, 
    			   rpc_gssauth_info_p_t, 
    			   sizeof (rpc_gssauth_info_t), 
    			   RPC_C_MEM_GSSAUTH_INFO, 
    			   RPC_C_MEM_WAITOK);

    /*
     * Initialize it.
     */
    memset (gssauth_info, 0, sizeof(rpc_gssauth_info_t));
    RPC_MUTEX_INIT (gssauth_info->lock);

    /*
     * Initialize the common auth_info stuff.
     */
    gssauth_info->auth_info.refcount = 1;
    gssauth_info->auth_info.server_princ_name = NULL;
    gssauth_info->auth_info.authn_level = authn_level;
    gssauth_info->auth_info.authn_protocol = protocol;
    gssauth_info->auth_info.authz_protocol = rpc_c_authz_name;
    gssauth_info->auth_info.is_server = true;

    gssauth_info->creds_valid = 1; 
    gssauth_info->level_valid = 1;
    gssauth_info->client_valid = 1;
     
    gssauth_info->server_creds = GSS_C_NO_CREDENTIAL;

    *auth_info = (rpc_auth_info_t *) gssauth_info;
    *st = rpc_s_ok;
}


/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_create_info_negotiate
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Create an auth information rep data structure with and
**      add a reference to it. This is called on the server
**      side. The fields will be initialized to NULL values.
**      The caller should fill them in based on information
**      decrypted from the authenticator passed in the bind
**      request.
**
**    Creates an authentication rep for the GSS-SPNEGO flavour.
**
**  INPUTS:
**
**      authn_level	 The authentication level to be applied over this
**    				  security context.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      info			A pointer to the auth information rep structure
**    				  containing RPC protocol indenpendent information.
**      st			  The return status of this routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       
**
**      The newly create auth info will have a reference count of 1.
**
**--
**/
INTERNAL void rpc__gssauth_cn_create_info_negotiate
#ifdef _DCE_PROTO_
(
    rpc_authn_level_t				authn_level,
    rpc_auth_info_p_t				*auth_info,
    unsigned32						*st
)
#else
(authn_level, auth_info, st)
rpc_authn_level_t    			authn_level;
rpc_auth_info_p_t    			*auth_info;
unsigned32    				   *st;
#endif
{
    return rpc__gssauth_cn_create_info(authn_level, rpc_c_authn_gss_negotiate, auth_info, st);
}


/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_create_info_mskrb
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Create an auth information rep data structure with and
**      add a reference to it. This is called on the server
**      side. The fields will be initialized to NULL values.
**      The caller should fill them in based on information
**      decrypted from the authenticator passed in the bind
**      request.
**
**    Creates an authentication rep for the Kerberos flavour.
**
**  INPUTS:
**
**      authn_level	 The authentication level to be applied over this
**    				  security context.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      info			A pointer to the auth information rep structure
**    				  containing RPC protocol indenpendent information.
**      st			  The return status of this routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       
**
**      The newly create auth info will have a reference count of 1.
**
**--
**/
INTERNAL void rpc__gssauth_cn_create_info_mskrb
#ifdef _DCE_PROTO_
(
    rpc_authn_level_t				authn_level,
    rpc_auth_info_p_t				*auth_info,
    unsigned32						*st
)
#else
(authn_level, auth_info, st)
rpc_authn_level_t    			authn_level;
rpc_auth_info_p_t    			*auth_info;
unsigned32    				   *st;
#endif
{
    return rpc__gssauth_cn_create_info(authn_level, rpc_c_authn_gss_mskrb, auth_info, st);
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_cred_changed
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Determine whether the client's credentials stored in the
**      security context are different from those in the auth info.
**      If they are not the same return true, else false.
**
**  INPUTS:
**
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     boolean32
**
**      True if the credentials identified by the auth
**      information rep and RPC auth information rep are different,
**      false if not.
**
**      The md5 checksum algorithm requires the use of the session key
**      to encrypt the CRC(assoc_uuid).  Since the session key will 
**      change when the credential changes, this routine sets the flag
**      indicating that a (potentially) valid encrypted crc is now 
**      invalid, forcing a recomputation.
**
**  SIDE EFFECTS:       none
**
**--
**/

INTERNAL boolean32 rpc__gssauth_cn_cred_changed 
#ifdef _DCE_PROTO_
(
    rpc_cn_sec_context_p_t		  sec,
    unsigned32					  *st
)
#else
(sec, st)
rpc_cn_sec_context_p_t    	  sec;
unsigned32    				  *st;
#endif
{
    rpc_gssauth_cn_info_p_t		gssauth_cn_info;
    rpc_gssauth_info_p_t		gssauth_info;
    boolean32				   different_creds;
    gss_name_t				   cred_name, ctx_name;
    OM_uint32				   maj_stat, min_stat;

    CODING_ERROR (st);

    gssauth_info = (rpc_gssauth_info_p_t) (sec->sec_info);
    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_cred_changed)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_cred_changed) prot->%x level->%x key_id->%x\n",
    				rpc_c_authn_dce_private,
    				sec->sec_info->authn_level,
    				sec->sec_key_id));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_CRED_CHANGED))
    {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return false;
    }
#endif

    RPC_MUTEX_LOCK(rpc_g_gss_lock);

    maj_stat = gss_inquire_cred(&min_stat,
    	gssauth_info->server_creds,
    	&cred_name,
    	NULL,
    	NULL,	
    	NULL);
    if (GSS_ERROR(maj_stat)) {
    	rpc__gssauth_map_error(maj_stat, min_stat, st);
        RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    	return false;
    }
    maj_stat = gss_inquire_context(&min_stat,
    	gssauth_cn_info->cn_context,
    	NULL,
    	&ctx_name,
    	NULL,
    	NULL,
    	NULL,
    	NULL,
    	NULL);
    if (GSS_ERROR(maj_stat)) {
    	rpc__gssauth_map_error(maj_stat, min_stat, st);
    	gss_release_name(&min_stat, &cred_name);
        RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    	return false;
    }
    maj_stat = gss_compare_name(&min_stat,
    	cred_name, ctx_name, (int *)&different_creds);

    rpc__gssauth_map_error(maj_stat, min_stat, st);
    gss_release_name(&min_stat, &cred_name);
    gss_release_name(&min_stat, &ctx_name);

    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);

    return different_creds;
}


/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_cred_refresh
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      Determine whether the client's credentials are still
**      valid. If not this routine will try to renew the credentials.
**      If they cannot be renewed an error is returned. This routine
**      is called from the client side. 
**
**  INPUTS:
**
**      auth_info	   A pointer to the auth information rep
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_cred_refresh 
#ifdef _DCE_PROTO_
(
    rpc_auth_info_p_t			   auth_info,
    unsigned32					  *st
)
#else
(auth_info, st)
rpc_auth_info_p_t    		   auth_info;
unsigned32    				  *st;
#endif
{
    rpc_gssauth_info_p_t gssauth_info;

    gssauth_info = (rpc_gssauth_info_p_t) (auth_info);

    CODING_ERROR (st);
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_cred_refresh)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_cred_refresh) prot->%x level->%x\n",
    				auth_info->authn_protocol,
    				auth_info->authn_level));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_CRED_REFRESH))
    {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    /*
     * Assume that cred is already valid.
     */
    *st = rpc_s_ok;
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_init_sec_context
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      A wrapper around gss_init_sec_context().
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      req_auth_value  A pointer to the auth_value field in the
**    				  rpc_bind or rpc_alter_context PDU authentication trailer.
**      req_auth_value_len The length, in bytes, of the
**    				  req_auth_value field.
**      auth_value	  A pointer to the auth_value field in the rpc_bind or
**    				  rpc_alter_context PDU authentication trailer.
**                    If NULL, a buffer is allocated for the caller.
**
**  INPUTS/OUTPUTS:     
**
**      auth_value_len  On input, the length, in bytes of the available space
**    				  for the auth_value field. On output, the length in
**    				  bytes used in encoding the auth_value field. Zero if
**    				  an error status is returned.
**
**  OUTPUTS:    		none
**
**  IMPLICIT INPUTS:    Caller has acquired rpc_g_gss_lock.
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       none
**
**--
**/
INTERNAL void rpc__gssauth_init_sec_context 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec __attribute__((__unused__)),
    rpc_cn_sec_context_p_t		  sec,
    pointer_t					   req_auth_value,
    unsigned32						req_auth_value_len,
    pointer_t						*auth_value,
    unsigned32					  *auth_value_len,
    unsigned32					  *st
)
#else
(assoc_sec, sec, req_auth_value, req_auth_value_len, auth_value_len, st)
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    		sec;
pointer_t    					req_auth_value;
unsigned32    					req_auth_value_len;
pointer_t    					*auth_value;
unsigned32    					*auth_value_len;
unsigned32    					*st;
#endif
{
    rpc_gssauth_cn_info_p_t		gssauth_cn_info;
    rpc_gssauth_info_p_t		gssauth_info;
    OM_uint32					min_stat, maj_stat;
    OM_uint32 					req_flags, ret_flags;
    gss_OID 					req_mech;
    gss_buffer_desc 			input_token, output_token;

    CODING_ERROR (st);

    input_token.value = req_auth_value;
    input_token.length = req_auth_value_len;

    output_token.value = NULL;
    output_token.length = 0;

    gssauth_info = (rpc_gssauth_info_p_t) (sec->sec_info);
    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_init_sec_context) state: %ld context: %0x\n",
    		 gssauth_cn_info->cn_state, gssauth_cn_info->cn_context));

    /*
     * Check that we're in the right place in the state machine.
     */
    switch (gssauth_cn_info->cn_state) {
        case rpc_gssauth_cn_state_none:
            gssauth_cn_info->cn_state = rpc_gssauth_cn_state_negotiate;
            break;
        case rpc_gssauth_cn_state_negotiate:
            break;
        default:
            *st = rpc_s_auth_badorder;
            break;
    }

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_init_sec_context) server: %s identity: %0x\n",
    		 sec->sec_info->server_princ_name, sec->sec_info->u.auth_identity));

    if (gssauth_info->server_name == GSS_C_NO_NAME) {
    	gss_buffer_desc server_name_token;

    	server_name_token.value = sec->sec_info->server_princ_name;
    	server_name_token.length = strlen(server_name_token.value);
    	maj_stat = gss_import_name(&min_stat, &server_name_token,
    		GSS_C_NT_HOSTBASED_SERVICE, &gssauth_info->server_name);
    	if (GSS_ERROR(maj_stat)) {
    		rpc__gssauth_map_error(maj_stat, min_stat, st);
    		return;
    	}
    }

#ifdef DEBUG
    maj_stat = gss_display_name(&min_stat, gssauth_info->server_name,
        &output_token, NULL);
    if (GSS_ERROR(maj_stat)) {
        rpc__gssauth_map_error(maj_stat, min_stat, st);
        return;
    }
    RPC_DBG_PRINTF(rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
        ("(rpc__gssauth_init_sec_context) parsed name: %s\n",
        output_token.value));
    gss_release_buffer(&min_stat, &output_token);
#endif

    req_flags = GSS_C_MUTUAL_FLAG | GSS_C_SEQUENCE_FLAG;
    switch (sec->sec_info->authn_level) {
    	case rpc_c_protect_level_pkt:
    	case rpc_c_protect_level_pkt_integ:
    		req_flags |= GSS_C_INTEG_FLAG;
    	case rpc_c_protect_level_pkt_privacy:
    		req_flags |= GSS_C_CONF_FLAG;
    		break;
    	default:
    		break;
    }

    switch (sec->sec_info->authn_protocol) {
#ifdef notyet
    	case rpc_c_authn_gss_negotiate:
    		req_mech = GSS_SPNEGO_MECHANISM;
    		break;
    	case rpc_c_authn_winnt:
    		req_mech = GSS_NTLM_MECHANISM;
    		break;
#endif
    	case rpc_c_authn_gss_mskrb:
    		req_mech = GSS_KRB5_MECHANISM;
    		break;
    	default:
    		*st = rpc_s_unknown_authn_service;
    		return;
    }

#ifdef DEBUG
    if (input_token.value != NULL) {
        dump_data(30, input_token.value, input_token.length);
    }
#endif

    maj_stat = gss_init_sec_context(&min_stat,
    	(gss_cred_id_t)gssauth_info->auth_info.u.auth_identity,
    	&gssauth_cn_info->cn_context,
    	gssauth_info->server_name,
    	req_mech,
    	req_flags,
    	0,
    	GSS_C_NO_CHANNEL_BINDINGS,
    	(input_token.value == NULL) ? GSS_C_NO_BUFFER : &input_token,
    	NULL,
    	&output_token,
    	&ret_flags,
    	NULL);
    if (GSS_ERROR(maj_stat)) {
    	rpc__gssauth_map_error(maj_stat, min_stat, st);
        return;
    }

    *st = rpc_s_ok;

    if (maj_stat == GSS_S_CONTINUE_NEEDED) {
        /* If we were supplied a buffer, check for size. */
        if (*auth_value != NULL && *auth_value_len < output_token.length) {
        	*st = rpc_s_credentials_too_large;
        	return;
        }
        *auth_value_len = output_token.length;
        if (output_token.value != NULL) {
#ifdef DEBUG
	        dump_data(30, output_token.value, output_token.length);
#endif
        	/* Only allocate a buffer if we were not supplied with one. */
        	if (*auth_value == NULL) {
        		RPC_MEM_ALLOC(*auth_value, pointer_t, *auth_value_len,
        			RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);
        	}
        	memcpy(*auth_value, output_token.value, output_token.length);
        	gss_release_buffer(&min_stat, &output_token);
        }
    } else if (maj_stat == GSS_S_COMPLETE) {
    	gssauth_cn_info->cn_state = rpc_gssauth_cn_state_authenticated;
        if ((ret_flags & req_flags) == 0) {
            *st = rpc_s_protect_level_mismatch;
        }
    }
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_accept_sec_context
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      A wrapper around gss_accept_sec_context().
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      req_auth_value  A pointer to the auth_value field in the
**    				  rpc_bind or rpc_alter_context PDU authentication trailer.
**      req_auth_value_len The length, in bytes, of the
**    				  req_auth_value field.
**      auth_value	  A pointer to the auth_value field in the rpc_bind or
**    				  rpc_alter_context PDU authentication trailer.
**                    If NULL, a buffer is allocated for the caller.
**
**  INPUTS/OUTPUTS:     
**
**      auth_value_len  On input, the length, in bytes of the available space
**    				  for the auth_value field. On output, the length in
**    				  bytes used in encoding the auth_value field. Zero if
**    				  an error status is returned.
**
**  OUTPUTS:    		none
**
**  IMPLICIT INPUTS:    Caller has acquired rpc_g_gss_lock.
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     none
**
**  SIDE EFFECTS:       none
**
**--
**/

INTERNAL void rpc__gssauth_accept_sec_context 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec __attribute__((__unused__)),
    rpc_cn_sec_context_p_t		  sec,
    pointer_t					   req_auth_value,
    unsigned32						req_auth_value_len,
    pointer_t						*auth_value,
    unsigned32					  *auth_value_len,
    unsigned32					  *st
)
#else
(assoc_sec, sec, req_auth_value, req_auth_value_len, auth_value_len, st)
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    		sec;
pointer_t    					req_auth_value;
unsigned32    					req_auth_value_len;
pointer_t    					*auth_value;
unsigned32    					*auth_value_len;
unsigned32    					*st;
#endif
{
    rpc_gssauth_cn_info_p_t			gssauth_cn_info;
    rpc_gssauth_info_p_t			gssauth_info;
    OM_uint32						min_stat, maj_stat;
    gss_buffer_desc					input_token, output_token;
    gss_OID							mech_type;

    input_token.value = req_auth_value;
    input_token.length = req_auth_value_len;

    output_token.value = NULL;
    output_token.length = 0;

    gssauth_info = (rpc_gssauth_info_p_t) (sec->sec_info);
    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_accept_sec_context) state: %ld context: %0x\n",
    		 gssauth_cn_info->cn_state, gssauth_cn_info->cn_context));

    /*
     * Check that we're in the right place in the state machine.
     */
    switch (gssauth_cn_info->cn_state) {
        case rpc_gssauth_cn_state_none:
            gssauth_cn_info->cn_state = rpc_gssauth_cn_state_negotiate;
            break;
        case rpc_gssauth_cn_state_negotiate:
            break;
        default:
            *st = rpc_s_auth_badorder;
            break;
    }

    /*
     * Pick up the registered credentials.
     */
    if (gssauth_info->server_creds == GSS_C_NO_CREDENTIAL) {
        rpc__gssauth_get_acceptor_cred(gssauth_info, st);
        if (*st != rpc_s_ok)
            return;
    }

#ifdef DEBUG
    dump_data(30, input_token.value, input_token.length);
#endif

    mech_type = GSS_C_NO_OID;

    maj_stat = gss_accept_sec_context(&min_stat,
    	&gssauth_cn_info->cn_context,
    	gssauth_info->server_creds,
    	&input_token,
    	GSS_C_NO_CHANNEL_BINDINGS,
    	&gssauth_info->client_name,
    	&mech_type,
    	&output_token,
    	NULL,
    	NULL,
    	NULL);
    if (GSS_ERROR(maj_stat)) {
    	rpc__gssauth_map_error(maj_stat, min_stat, st);
        return;
    }

    *st = rpc_s_ok;

    if (maj_stat == GSS_S_CONTINUE_NEEDED) {
        /*
         * Check that we have enough room for the next token, if
         * we were provided with a fixed size buffer.
         */
        if (*auth_value != NULL && *auth_value_len < output_token.length) {
        	*st = rpc_s_credentials_too_large;
        	return;
        }
    
        *auth_value_len = output_token.length;

        /*
         * Return the next token to the client.
         */
        if (output_token.value != NULL) {
#ifdef DEBUG
	        dump_data(30, output_token.value, output_token.length);
#endif
        	if (*auth_value == NULL) {
        		RPC_MEM_ALLOC(*auth_value, pointer_t, *auth_value_len,
        			RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);
        	}
        	memcpy(auth_value, output_token.value, output_token.length);
        	gss_release_buffer(&min_stat, &output_token);
        }
    } else if (maj_stat == GSS_S_COMPLETE) {
        /*
         * If this was the last token, then we can advance to the
         * next state (authenticated) and extract the pickled PAC.
         */
    	gssauth_cn_info->cn_state = rpc_gssauth_cn_state_authenticated;

    	if (mech_type == GSS_KRB5_MECHANISM) {
    		idl_es_handle_t h = NULL;
    		krb5_ticket *ticket;
    		krb5_data *data;
            sec_id_pac_format_t pactype = -1;

    		ticket = gssauth_cn_info->cn_context->ticket;
            if (ticket->ticket.authorization_data->val == NULL) {
                goto out;
            }

            data = &ticket->ticket.authorization_data->val[0].ad_data;

 		    idl_es_decode_buffer((idl_void_p_t)data->data, data->length, &h, st);
   		    if (*st != rpc_s_ok) {
   			    goto out;
            }

            switch (ticket->ticket.authorization_data->val[0].ad_type) {
                case sec_id_authz_data_dce:
    			    sec__id_pac_format_v1_unpickle(h,
                        &gssauth_info->client_pac.pac.v1_pac, st);
                    pactype = sec_id_pac_format_v1;
                    break;
                case sec_id_authz_data_mspac:
                    /* XXX need to move pointer past PAC header */
    			    sec__id_pac_format_ms_unpickle(h,
                        &gssauth_info->client_pac.pac.ms_pac, st);
                    pactype = sec_id_pac_format_ms;
                    break;
                default:
                    *st = rpc_s_authn_authz_mismatch;
                    break;
            }

    		if (*st == rpc_s_ok) {
    			gssauth_info->client_pac.pac_type = pactype;
    			gssauth_info->client_pac.authenticated = false;
    			sec->sec_info->u.s.privs = &gssauth_info->client_pac;
    		} else {
    			sec->sec_info->u.s.privs = NULL;
    			*st = rpc_s_ok;
    		}
out:
    		if (h != NULL) {
    			idl_es_handle_free(&h, st);
    		}
    	}
    } 
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_fmt_client_req
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will format the auth_value field of
**      either an rpc_bind or rpc_alter_context PDU. This is
**      called from the client side association state machine.
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      auth_value	  A pointer to the auth_value field in the rpc_bind or
**    				  rpc_alter_context PDU authentication trailer.
**
**  INPUTS/OUTPUTS:     
**
**      auth_value_len  On input, the lenght, in bytes of the available space
**    				  for the auth_value field. On output, the lenght in
**    				  bytes used in encoding the auth_value field. Zero if
**    				  an error status is returned.
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_fmt_client_req 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec,
    rpc_cn_sec_context_p_t		  sec,
    pointer_t					   auth_value,
    unsigned32					  *auth_value_len,
    pointer_t					   *last_auth_pos,
    unsigned32					  *auth_len_remain,
    unsigned32					  old_server __attribute__((__unused__)),
    unsigned32					  *st
)
#else
(assoc_sec, sec, auth_value, auth_value_len, last_auth_pos, st)
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    	  sec;
pointer_t    				   auth_value;
unsigned32    				  *auth_value_len;
pointer_t    				   *last_auth_pos;
unsigned32    				  *auth_len_remain;
unsigned32    				  old_server;
unsigned32    				  *st;
#endif
{

    CODING_ERROR (st);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_fmt_client_req) old_srv: %ld\n",
    		 old_server));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_fmt_client_req) prot->%x level->%x key_id->%x assoc_uuid_crc->%x xmit_seq->%x recv_seq->%x\n",
    				sec->sec_info->authn_protocol,
    				sec->sec_info->authn_level,
    				sec->sec_key_id,
    				assoc_sec->assoc_uuid_crc,
    				assoc_sec->assoc_next_snd_seq,
    				assoc_sec->assoc_next_rcv_seq));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_FMT_CLIENT_REQ)) {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    assoc_sec->krb_message.data = NULL;
    assoc_sec->krb_message.length = 0;

    RPC_MUTEX_LOCK(rpc_g_gss_lock);
    rpc__gssauth_init_sec_context(assoc_sec,
    	sec,
    	NULL,
    	0,
    	&auth_value,
    	auth_value_len,
    	st);
    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);

    *last_auth_pos = auth_value + *auth_value_len;
    *auth_len_remain = 0;
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_fmt_srvr_resp
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will format the auth_value field of
**      either an rpc_bind_ack or rpc_alter_context_response
**      PDU. The authentication protocol encoding in the
**      credentials field of auth_value should correspond
**      to the status returned from rpc__auth_cn_vfy_
**      client_req()  routine. This credentials field, when
**      decoded by rpc__auth_cn_vfy_srvr_resp(),  should
**      result in the same error code being returned. If the
**      memory provided is not large enough an authentication
**      protocol specific error message will be encoded in
**      the credentials field indicating this error. This is
**      called from the server side association state machine.
**
**  INPUTS:
**
**      verify_st	   The status code returned by rpc__auth_cn_verify_
**    				  client_req().
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      req_auth_value  A pointer to the auth_value field in the
**    				  rpc_bind or rpc_alter_context PDU authentication trailer.
**      req_auth_value_len The length, in bytes, of the
**    				  req_auth_value field.
**      auth_value	  A pointer to the auth_value field in the rpc_bind or
**    				  rpc_alter_context PDU authentication trailer.
**
**  INPUTS/OUTPUTS:     
**
**      auth_value_len  On input, the length, in bytes of the available space
**    				  for the auth_value field. On output, the length in
**    				  bytes used in encoding the auth_value field. Zero if
**    				  an error status is returned.
**
**  OUTPUTS:    		none
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

INTERNAL void rpc__gssauth_cn_fmt_srvr_resp 
#ifdef _DCE_PROTO_
(
    unsigned32					  verify_st,
    rpc_cn_assoc_sec_context_p_t	assoc_sec,
    rpc_cn_sec_context_p_t		  sec,
    pointer_t					   req_auth_value,
    unsigned32					  req_auth_value_len,
    pointer_t					   auth_value,
    unsigned32					  *auth_value_len
)
#else
(verify_st, assoc_sec, sec, req_auth_value, req_auth_value_len, auth_value, auth_value_len)
unsigned32    				  verify_st;
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    	  sec;
pointer_t    				   req_auth_value;
unsigned32    				  req_auth_value_len;
pointer_t    				   auth_value;
unsigned32    				  *auth_value_len;
#endif
{
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_fmt_srvr_resp) req_auth_value_len %d req_auth_value %p\n",
    		 req_auth_value_len, req_auth_value));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_fmt_srvr_resp) prot->%x level->%x key_id->%x assoc_uuid_crc->%x xmit_seq->%x recv_seq->%x, vfy_client_st->%x\n",
    				sec->sec_info->authn_protocol,
    				sec->sec_info->authn_level,
    				sec->sec_key_id,
    				assoc_sec->assoc_uuid_crc,
    				assoc_sec->assoc_next_snd_seq,
    				assoc_sec->assoc_next_rcv_seq,
    				verify_st));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_FMT_SERVER_RESP))
    {
    	verify_st = RPC_S_CN_DBG_AUTH_FAILURE;
    }
#endif

    assert (verify_st == rpc_s_ok);

    assoc_sec->krb_message.data = NULL;
    assoc_sec->krb_message.length = 0;

    RPC_MUTEX_LOCK(rpc_g_gss_lock);
    rpc__gssauth_accept_sec_context(assoc_sec,
    	sec,
    	req_auth_value,
    	req_auth_value_len,
    	&auth_value,
    	auth_value_len,
    	&verify_st);
    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);

    if (verify_st != rpc_s_ok) {
    	*auth_value_len = 0;
    }
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_free_prot_info
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will free an NCA Connection RPC auth
**      information rep.
**
**  INPUTS:    		 
**
**      info			A pointer to the auth information rep structure
**    				  containing RPC protocol indenpendent information.
**
**  INPUTS/OUTPUTS:     
**
**      cn_info		 A pointer to the RPC auth information rep structure
**    				  containing NCA Connection specific
**    				  information. NULL on output. 
**
**  OUTPUTS:    		none
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

INTERNAL void rpc__gssauth_cn_free_prot_info 
#ifdef _DCE_PROTO_
(
    rpc_auth_info_p_t			   info __attribute__((__unused__)),
    rpc_cn_auth_info_p_t			*cn_info
)
#else
(info, cn_info)
rpc_auth_info_p_t    		   info;	
rpc_cn_auth_info_p_t    		*cn_info;
#endif
{
    OM_uint32 min_stat;
    rpc_gssauth_cn_info_p_t gssauth_cn_info;

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_free_prot_info)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_free_prot_info) prot->%x level->%x \n",
    				info->authn_protocol,
    				info->authn_level));

    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (*cn_info);

    if (gssauth_cn_info->cn_context != GSS_C_NO_CONTEXT) {
    	gss_delete_sec_context(&min_stat, &gssauth_cn_info->cn_context,
    		GSS_C_NO_BUFFER);
    }

#ifdef DEBUG
    memset (*cn_info, 0, sizeof (rpc_cn_auth_info_t));
#endif

    RPC_MEM_FREE (*cn_info, RPC_C_MEM_GSSAUTH_CN_INFO);
    *cn_info = NULL;
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_get_prot_info
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will create and return an NCA Connection
**      RPC auth information rep.
**
**  INPUTS:
**
**      info			A pointer to the auth information rep structure
**    				  containing RPC protocol indenpendent information.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      cn_info		 A pointer to the RPC auth information rep structure
**    				  containing NCA Connection specific information.
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_get_prot_info 
#ifdef _DCE_PROTO_
(
    rpc_auth_info_p_t			   info __attribute__((__unused__)),
    rpc_cn_auth_info_p_t			*cn_info,
    unsigned32					  *st
)
#else
(info, cn_info, st)
rpc_auth_info_p_t    		   info;
rpc_cn_auth_info_p_t    		*cn_info;
unsigned32    				  *st;
#endif
{
    rpc_gssauth_cn_info_t		*gssauth_cn_info;

    CODING_ERROR (st);
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_get_prot_info)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_get_prot_info) prot->%x level->%x \n",
    				info->authn_protocol,
    				info->authn_level));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_GET_PROT_INFO))
    {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    /*
     * Allocate storage for a gssauth cn info structure from heap.
     */
    RPC_MEM_ALLOC (gssauth_cn_info, 
    			   rpc_gssauth_cn_info_p_t, 
    			   sizeof (rpc_gssauth_cn_info_t), 
    			   RPC_C_MEM_GSSAUTH_CN_INFO, 
    			   RPC_C_MEM_WAITOK);

    /*
     * Initialize it.
     */
    memset (gssauth_cn_info, 0, sizeof(rpc_gssauth_cn_info_t));
    gssauth_cn_info->cn_state = rpc_gssauth_cn_state_none;
    gssauth_cn_info->cn_context = GSS_C_NO_CONTEXT;

    *cn_info = (rpc_cn_auth_info_t *)gssauth_cn_info;
    *st = rpc_s_ok;
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_pre_call
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will format the auth_value field of
**      a call level PDU, namely an rpc_request, rpc_response
**      rpc_fault, rpc_remote_alert or rpc_orphaned PDU. It will
**      also format the auth_value field of the association level
**      rpc_shutdown PDU. This does
**      not include generating any checksums in the auth_value_field
**      or encrypting of data corresponding to the authentication
**      level. That will be done in the rpc__cn_auth_pre_send
**      routine. This is called on both client and server when a the
**      data structures and header template for a call is being set up.
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      auth_value	  A pointer to the auth_value field in the rpc_bind or
**    				  rpc_alter_context PDU authentication trailer.
**
**  INPUTS/OUTPUTS:     
**
**      auth_value_len  On input, the lenght, in bytes of the available space
**    				  for the auth_value field. On output, the lenght in
**    				  bytes used in encoding the auth_value field. Zero if
**    				  an error status is returned.
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_pre_call 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec __attribute__((__unused__)),
    rpc_cn_sec_context_p_t		  sec,
    pointer_t					   auth_value __attribute__((__unused__)),
    unsigned32					  *auth_value_len __attribute__((__unused__)),
    unsigned32					  *st
)
#else
(assoc_sec, sec, auth_value, auth_value_len, st)
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    	  sec;
pointer_t    				   auth_value;
unsigned32    				  *auth_value_len;
unsigned32    				  *st;
#endif
{
    rpc_gssauth_cn_info_p_t gssauth_cn_info;
    rpc_gssauth_info_p_t gssauth_info;

    CODING_ERROR(st);

    gssauth_info = (rpc_gssauth_info_p_t) (sec->sec_info);
    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_pre_call)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_pre_call) prot->%x level->%x key_id->%x assoc_uuid_crc->%x xmit_seq->%x recv_seq->%x\n",
    				sec->sec_info->authn_protocol,
    				sec->sec_info->authn_level,
    				sec->sec_key_id,
    				assoc_sec->assoc_uuid_crc,
    				assoc_sec->assoc_next_snd_seq,
    				assoc_sec->assoc_next_rcv_seq));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_PRE_CALL)) {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    *st = rpc_s_ok;
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_pre_send
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will perform per-packet security
**      processing on a packet before it is sent. This
**      includes checksumming and encryption.
**
**      Note that in some cases, the data is copied to
**      a contiguous buffer for checksumming and 
**      encryption.  In these cases, the contiguous
**      iov element should be used instead of the original 
**      iovector.
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      iov			 A pointer to the iovector containing the PDU
**    				  about to be sent. The appropriate per-packet security
**    				  services will be applied to it. 
**      iovlen		  The length, in bytes, of the PDU.
**      out_iov		 An iovector element.  This iovector element
**    				  will describe packet if the original iov
**    				  had to be copied.  If the original iov was
**    				  copied, out_iov->base will be non-NULL.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_pre_send 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec __attribute__((__unused__)),
    rpc_cn_sec_context_p_t		  sec,
    rpc_socket_iovec_p_t			iov,
    unsigned32					  iovlen,
    rpc_socket_iovec_p_t			out_iov,
    unsigned32					  *st
)
#else
(assoc_sec, sec, iov, iovlen, out_iov, st)
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    	  sec;
rpc_socket_iovec_p_t    		iov;
unsigned32    				  iovlen;
rpc_socket_iovec_p_t    		out_iov;
unsigned32    				  *st;
#endif
{
    unsigned32		  			ptype;
    rpc_cn_common_hdr_t			*pdu;
    rpc_gssauth_info_p_t		gssauth_info;
    rpc_gssauth_cn_info_p_t		gssauth_cn_info;
    OM_uint32					maj_stat, min_stat;
    gss_buffer_desc				input_token, output_token;
    unsigned_char_p_t 			auth_value;

    gssauth_info = (rpc_gssauth_info_p_t) (sec->sec_info);
    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    CODING_ERROR (st);
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_pre_send)\n"));

    pdu = (rpc_cn_common_hdr_t *)(iov[0].iov_base);

    ptype = pdu->ptype;

    /* Really, really, important to make sure this is initialized. */
    out_iov->iov_base = NULL;

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_GENERAL,
    				("(rpc__gssauth_cn_pre_send) authn level->%x packet type->%x iovlen->%d\n", sec->sec_info->authn_level, ptype, iovlen));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_pre_send) prot->%x level->%x key_id->%x assoc_uuid_crc->%x xmit_seq->%x recv_seq->%x ptype->%x\n",
    				sec->sec_info->authn_protocol,
    				sec->sec_info->authn_level,
    				sec->sec_key_id,
    				assoc_sec->assoc_uuid_crc,
    				assoc_sec->assoc_next_snd_seq,
    				assoc_sec->assoc_next_rcv_seq,
    				ptype));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_PRE_SEND)) {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    switch (ptype) {
    	case RPC_C_CN_PKT_REQUEST:
    	case RPC_C_CN_PKT_RESPONSE: {
    		unsigned32 i;
    		OM_uint32 max_input_size;
    		boolean32 conf_req_flag = false;
    		unsigned32 header_size;

            switch (sec->sec_info->authn_level) {
                case rpc_c_protect_level_pkt:
                case rpc_c_protect_level_pkt_integ:
                    break;
                case rpc_c_protect_level_pkt_privacy:
    			    conf_req_flag = true;
                    break;
                default:
                    *st = rpc_s_ok;
                    return;
                    break;
            }

    		if (ptype == RPC_C_CN_PKT_REQUEST) {
    			header_size = RPC_CN_PKT_SIZEOF_RQST_HDR;
    		} else {
    			header_size = RPC_CN_PKT_SIZEOF_RESP_HDR;
    		}

            RPC_MUTEX_LOCK(rpc_g_gss_lock);

    		if (conf_req_flag) {
    			unsigned_char_p_t serial_frag_buf, serial_frag_p;
    			unsigned32 serial_frag_len;

    			/* Figure out how much memory to seal just stub data &c */
    			maj_stat = gss_wrap_size_limit(&min_stat,
    				gssauth_cn_info->cn_context,
    				(int)conf_req_flag,
    				GSS_C_QOP_DEFAULT,
    				pdu->frag_len - header_size - pdu->auth_len,
    				&max_input_size);
    			if (GSS_ERROR(maj_stat)) {
    				rpc__gssauth_map_error(maj_stat, min_stat, st);
                    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    				return;
    			}
    			RPC_MEM_ALLOC (out_iov->iov_base, unsigned_char_p_t, 
    								   max_input_size + header_size + pdu->auth_len,
    								   RPC_C_MEM_CN_ENCRYPT_BUF, 
    								   RPC_C_MEM_WAITOK);
    			serial_frag_buf = NULL;
    			serial_frag_len = 0;

    			if (iovlen > 1) {
    				unsigned_char_p_t val;

    				RPC_MEM_ALLOC(serial_frag_buf, unsigned_char_p_t,
    					pdu->frag_len, RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);
    				val = serial_frag_buf;
    				for (i = 0; i < iovlen; i++) {
    					memcpy(val, iov[i].iov_base, iov[i].iov_len);
    					val += iov[i].iov_len;
    					serial_frag_len += iov[i].iov_len;
    				}
    				serial_frag_p = serial_frag_buf;
    			} else {
    				serial_frag_p = iov[0].iov_base;
    				serial_frag_len = iov[0].iov_len;
    			}
    			if (serial_frag_len < pdu->frag_len) {
    				RPC_MEM_FREE(out_iov->iov_base, RPC_C_MEM_CN_ENCRYPT_BUF);
                    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    				*st = rpc_m_frag_toobig;
    				return;
    			}
    			/* Copy PDU header */
    			memcpy(out_iov->iov_base, serial_frag_p, header_size);
    			serial_frag_p += header_size;

    			/* Generate encrypted payload */
    			input_token.value = serial_frag_p;
    			input_token.length = pdu->frag_len - header_size - pdu->auth_len;

    			maj_stat = gss_wrap(&min_stat,
    				gssauth_cn_info->cn_context,
    				1,
    				GSS_C_QOP_DEFAULT,
    				&input_token,
    				NULL,
    				&output_token);
    			if (GSS_ERROR(maj_stat)) {
    				RPC_MEM_FREE(out_iov->iov_base, RPC_C_MEM_CN_ENCRYPT_BUF);
    				rpc__gssauth_map_error(maj_stat, min_stat, st);
                    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    				return;
    			}
    			/* Copy encrypted PDU payload (stub data) */
    			if (max_input_size < output_token.length) {
    				RPC_MEM_FREE(out_iov->iov_base, RPC_C_MEM_CN_ENCRYPT_BUF);
                    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    				*st = rpc_m_frag_toobig;
    				return;
    			}
    			memcpy(out_iov->iov_base + header_size, output_token.value,
    				output_token.length);
    			gss_release_buffer(&min_stat, &output_token);

    			if (serial_frag_buf != NULL) {
    				RPC_MEM_FREE(serial_frag_buf, RPC_C_MEM_UTIL);
    			}

    			pdu = (rpc_cn_common_hdr_t *)out_iov->iov_base;
    		} else {
    			/* Do everything in-place. */
    			out_iov->iov_base = NULL;
    		}

    		/* Get a pointer to the auth value. */
    		auth_value = (unsigned_char_p_t)pdu + pdu->frag_len - pdu->auth_len;

#ifdef DEBUG
    		dump_data(20, (char *)pdu, pdu->frag_len);
#endif

    		/* Generate the PDU checksum */
    		input_token.value = pdu;
    		input_token.length = pdu->frag_len - pdu->auth_len;

    		maj_stat = gss_get_mic(&min_stat,
    			gssauth_cn_info->cn_context,
    			GSS_C_QOP_DEFAULT,
    			&input_token,
    			&output_token);
    		if (GSS_ERROR(maj_stat)) {
    			rpc__gssauth_map_error(maj_stat, min_stat, st);
                RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    			return;
    		}

            RPC_MUTEX_UNLOCK(rpc_g_gss_lock);

    		if (pdu->auth_len < output_token.length) {
    			*st = rpc_s_credentials_too_large;
    			return;
    		}
    		/* Put the MIC in the verifier part of PDU. */
    		pdu->auth_len = output_token.length;
    		memcpy(auth_value, output_token.value, output_token.length);
    		gss_release_buffer(&min_stat, &output_token);

#ifdef DEBUG
    		dump_data(20, (char *)auth_value, pdu->auth_len);
#endif
    		break;
    	}
    	case RPC_C_CN_PKT_FAULT: {
    		break;
    	}

    	case RPC_C_CN_PKT_AUTH3: {
    		/*
    		 * hopefully this is the correct way to do 3 way auth.
             * We enqueue the token elsewhere, in krb_message.
    		 */
    		if (assoc_sec->krb_message.data != NULL) {
    			if (pdu->auth_len < assoc_sec->krb_message.length) {
    				*st = rpc_s_credentials_too_large;
    			} else {
    				auth_value = (unsigned_char_p_t)pdu + pdu->frag_len - pdu->auth_len;
    				memcpy(auth_value, assoc_sec->krb_message.data,
    					assoc_sec->krb_message.length);
    				pdu->auth_len = (unsigned16)assoc_sec->krb_message.length;
#ifdef DEBUG
    				dump_data(10, auth_value, pdu->auth_len);
#endif
    				*st = rpc_s_ok;
    			}
    			RPC_MEM_FREE(assoc_sec->krb_message.data, RPC_C_MEM_UTIL);
    			assoc_sec->krb_message.data = NULL;
    			assoc_sec->krb_message.length = 0;
    		}
    		break;
    	}

    	case RPC_C_CN_PKT_BIND:
    	case RPC_C_CN_PKT_BIND_ACK:
    	case RPC_C_CN_PKT_BIND_NAK:
    	case RPC_C_CN_PKT_ALTER_CONTEXT:
    	case RPC_C_CN_PKT_ALTER_CONTEXT_RESP:
    	case RPC_C_CN_PKT_SHUTDOWN:
    	case RPC_C_CN_PKT_REMOTE_ALERT:
    	case RPC_C_CN_PKT_ORPHANED:
    	default: {
    		break;
    	}
    }

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_GENERAL,
    				("(rpc__gssauth_cn_pre_send) successful return"));

    *st = rpc_s_ok;
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_recv_check
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will perform per-packet security
**      processing on a packet after it is received. This
**      includes decryption and verification of checksums.
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      pdu			 A pointer to the PDU about to be sent (huh?). The appropriate
**    				  per-packet security services will be applied to it.
**      pdu_len		 The length, in bytes, of the PDU.
**      cred_len		The length, in bytes, of the credentials.
**    NB: cred_len is useless for this mechanism because it is computed
**    from rpc_cn_bind_auth_value_priv_t which Microsoft ignore!
**      auth_tlr		A pointer to the auth trailer.
**      unpack_ints	 A boolean indicating whether the integer rep
**    				  of fields in the pdu need to be adjusted for
**    				  endian differences.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_recv_check 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec,
    rpc_cn_sec_context_p_t		  sec,
    rpc_cn_common_hdr_p_t		   pdu,
    unsigned32					  pdu_len,
    unsigned32					  cred_len __attribute__((__unused__)),
    rpc_cn_auth_tlr_p_t			 auth_tlr,
    boolean32					   unpack_ints __attribute__((__unused__)),
    unsigned32					  *st
)
#else
(assoc_sec, sec, pdu, pdu_len, cred_len, auth_tlr, unpack_ints, st)
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    	  sec;
rpc_cn_common_hdr_p_t    	   pdu;
unsigned32    				  pdu_len;
unsigned32    				  cred_len;
rpc_cn_auth_tlr_p_t    		 auth_tlr;
boolean32    				   unpack_ints;
unsigned32    				  *st;
#endif
{
    unsigned32				ptype;
    unsigned32				authn_level;
    unsigned32				assoc_uuid_crc;
    rpc_gssauth_info_p_t	gssauth_info;
    rpc_gssauth_cn_info_p_t	gssauth_cn_info;
    gss_buffer_desc			input_token, output_token, mic_token;
    OM_uint32				maj_stat, min_stat;

    gssauth_info = (rpc_gssauth_info_p_t) (sec->sec_info);
    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    CODING_ERROR (st);
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_recv_check)\n"));

    ptype = pdu->ptype;
    authn_level = auth_tlr->auth_level;
    assoc_uuid_crc = assoc_sec->assoc_uuid_crc;

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_GENERAL,
    				("(rpc__gssauth_cn_recv_check) authn level->%x packet type->%x\n", authn_level, ptype));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_recv_check) prot->%x level->%x key_id->%x assoc_uuid_crc->%x xmit_seq->%x recv_seq->%x ptype->%x pdulen->%d, cred_len->%d unpack_ints->%d\n",
    				sec->sec_info->authn_protocol,
    				authn_level,
    				sec->sec_key_id,
    				assoc_uuid_crc,
    				assoc_sec->assoc_next_snd_seq,
    				assoc_sec->assoc_next_rcv_seq,
    				ptype, pdu_len, cred_len, unpack_ints));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_RECV_CHECK)) {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    /*
     * Check we are at the right part of the state
     * machine.
     */
    if (ptype != RPC_C_CN_PKT_AUTH3 &&
    	gssauth_cn_info->cn_state != rpc_gssauth_cn_state_authenticated) {
    	*st = rpc_s_auth_badorder;
    	return;
    }

    switch (ptype) {
    	case RPC_C_CN_PKT_REQUEST:
    	case RPC_C_CN_PKT_RESPONSE: {
    		boolean32 conf_req_flag = false, conf_state = false;
    		unsigned32 header_size;

    		if (sec->sec_info->authn_level >= rpc_c_protect_level_pkt_privacy) {
    			conf_req_flag = true;
    		}

    		if (ptype == RPC_C_CN_PKT_REQUEST) {
    			header_size = RPC_CN_PKT_SIZEOF_RQST_HDR;
    		} else {
    			header_size = RPC_CN_PKT_SIZEOF_RESP_HDR;
    		}

#ifdef DEBUG
    		dump_data(10, auth_tlr->auth_value, pdu->auth_len);
#endif

            RPC_MUTEX_LOCK(rpc_g_gss_lock);

    		/*
    		 * We encrypted the data first, then signed it; so, do
    		 * the reverse here: verify the signature and then
    		 * decrypt if necessary.
    		 */
    		input_token.value = pdu;
    		input_token.length = pdu_len - pdu->auth_len;
    		mic_token.value = auth_tlr->auth_value;
    		mic_token.length = pdu->auth_len;

    		maj_stat = gss_verify_mic(&min_stat,
    			gssauth_cn_info->cn_context,
    			&input_token,
    			&mic_token,
    			NULL);
    		if (GSS_ERROR(maj_stat)) {
    			rpc__gssauth_map_error(maj_stat, min_stat, st);
                RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    			return;
    		}

    		if (conf_req_flag) {
    			input_token.value = pdu + header_size;
    			input_token.length = pdu_len - header_size - pdu->auth_len;

    			maj_stat = gss_unwrap(&min_stat,
    				gssauth_cn_info->cn_context,
    				&input_token,
    				&output_token,
    				(int *)&conf_state,
    				NULL);
    			if (GSS_ERROR(maj_stat)) {
    				rpc__gssauth_map_error(maj_stat, min_stat, st);
                    RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    				return;
    			}

                RPC_MUTEX_UNLOCK(rpc_g_gss_lock);
    			if (conf_state == false) {
    				*st = rpc_s_auth_modified;
    				return;
    			}
    			if (input_token.length < output_token.length) {
    				/* can't decrypt in-place, bail. */
    				*st = rpc_m_frag_toobig;
    				gss_release_buffer(&min_stat, &output_token);
    				return;
    			}
    			/* Recall, input_token is a pointer into the PDU. */
    			memset(input_token.value, 0, input_token.length);
    			memcpy(input_token.value, output_token.value, output_token.length);
#ifdef DEBUG
    			dump_data(10, input_token.value, input_token.length);
#endif
    			gss_release_buffer(&min_stat, &output_token);
    		}
    	}
    	case RPC_C_CN_PKT_FAULT:
    	case RPC_C_CN_PKT_BIND:
    	case RPC_C_CN_PKT_BIND_ACK:
    	case RPC_C_CN_PKT_BIND_NAK:
    	case RPC_C_CN_PKT_ALTER_CONTEXT:
    	case RPC_C_CN_PKT_ALTER_CONTEXT_RESP: {
    		break;
    	}

    	case RPC_C_CN_PKT_AUTH3: 
    	case RPC_C_CN_PKT_SHUTDOWN:
    	case RPC_C_CN_PKT_REMOTE_ALERT:
    	case RPC_C_CN_PKT_ORPHANED:
    	default: {
    		break;
    	}
    }

    *st = rpc_s_ok;
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_tlr_uuid_crc
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will locate and return the association
**      UUID CRC contained in the auth_value field of an
**      authentication trailer of an rpc_bind, rpc_bind_ack,
**      rpc_alter_context or rpc_alter_context_response PDU.
**
**  INPUTS:
**
**      auth_value	  A pointer to the auth_value field in an authentication
**    				  trailer.
**      auth_value_len  The length, in bytes, of the auth_value field.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      assoc_uuid_crc  The association UUID CRC contained in the auth_value
**    				  field.
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

INTERNAL void rpc__gssauth_cn_tlr_uuid_crc 
#ifdef _DCE_PROTO_
(
    pointer_t			   auth_value,
    unsigned32			  auth_value_len,
    unsigned32			  *uuid_crc
)
#else
(auth_value, auth_value_len, uuid_crc)
pointer_t    		   auth_value;
unsigned32    		  auth_value_len;
unsigned32    		  *uuid_crc;
#endif
{
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_tlr_uuid_crc)\n"));

    assert (auth_value_len >= RPC_CN_PKT_SIZEOF_BIND_AUTH_VAL);
    *uuid_crc = crc32_calc_buffer(auth_value_len, auth_value);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_GENERAL,
    				("(rpc__gssauth_cn_tlr_uuid_crc) assoc_uuid_crc->%x\n", *uuid_crc));
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_tlr_unpack
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will byte swap all the appropriate fields
**      of the the auth_value field of an authentication
**      trailer. It will also convert any characters from
**      the remote representation into the local, for example,
**      ASCII to EBCDIC.
**
**  INPUTS:
**
**      auth_value_len  The length, in bytes, of the auth_value field.
**      packed_drep	 The packed Network Data Representation, (see NCA RPC
**    				  RunTime Extensions Specification Version OSF TX1.0.9
**    				  pre 1003 for details), of the remote machine.
**
**  INPUTS/OUTPUTS:     
**
**      pkt_p		   A pointer to the entire packet.
**
**  OUTPUTS:    		none
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

INTERNAL void rpc__gssauth_cn_tlr_unpack 
#ifdef _DCE_PROTO_
(
    rpc_cn_packet_p_t	   pkt_p __attribute__((__unused__)),
    unsigned32			  auth_value_len __attribute__((__unused__)),
    unsigned8			   *packed_drep __attribute__((__unused__))
)
#else
(pkt_p, auth_value_len, packed_drep) 
rpc_cn_packet_p_t       pkt_p;
unsigned32    		  auth_value_len;
unsigned8    		   *packed_drep;
#endif
{
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_tlr_unpack)\n"));
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_tlr_unpack), pkt->%d auth_value_len->%ld packed_drep->%p\n",
    		 pkt_p, auth_value_len, packed_drep));
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_vfy_client_req
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will decode the auth_value field of
**      either an rpc_bind or rpc_alter_context PDU. Any
**      error encountered while authenticating the client
**      will result in an error status return. The contents
**      of the credentials field includes the authorization
**      data. This is called from the server side association
**      state machine. Note that upon successful return
**      the auth information rep will contain the client's
**      authorization protocol and data.
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      auth_value	  A pointer to the auth_value field in the rpc_bind or
**    				  rpc_alter_context PDU authentication trailer.
**      auth_value_len  The length, in bytes, of auth_value.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_vfy_client_req 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec,
    rpc_cn_sec_context_p_t		  sec,
    pointer_t					   auth_value,
    unsigned32					  auth_value_len,
    unsigned32					old_client __attribute__((__unused__)),
    unsigned32					  *st
)
#else
(assoc_sec, sec, auth_value, auth_value_len, old_client, st)
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    	  sec;
pointer_t    				   auth_value;
unsigned32    				  auth_value_len;
unsigned32    			old_client;
unsigned32    				  *st;
#endif
{
    rpc_gssauth_cn_info_p_t		gssauth_cn_info;

    CODING_ERROR (st);

    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_vfy_client_req)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_vfy_client_req) prot->%x level->%x key_id->%x assoc_uuid_crc->%x xmit_seq->%x recv_seq->%x\n",
    				sec->sec_info->authn_protocol,
    				sec->sec_info->authn_level,
    				sec->sec_key_id,
    				assoc_sec->assoc_uuid_crc,
    				assoc_sec->assoc_next_snd_seq,
    				assoc_sec->assoc_next_rcv_seq));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_vfy_client_req) auth_value->%p auth_value_len->%lx, old_client->%lx\n",
    				auth_value, auth_value_len, old_client));

#ifdef DEBUG
    dump_data(RPC_C_CN_DBG_AUTH_PKT, auth_value, auth_value_len);

    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_VFY_CLIENT_REQ)) {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    if (gssauth_cn_info->cn_state < rpc_gssauth_cn_state_authenticated) {
    	/* have a buffer allocated for us. */
    	assoc_sec->krb_message.data = NULL;
    	assoc_sec->krb_message.length = 0;
   
        RPC_MUTEX_LOCK(rpc_g_gss_lock); 
    	rpc__gssauth_accept_sec_context(assoc_sec,
    		sec,
    		auth_value,
    		auth_value_len,
    		(pointer_t) &assoc_sec->krb_message.data,
    		(unsigned32 *) &assoc_sec->krb_message.length,
    		st);
        RPC_MUTEX_UNLOCK(rpc_g_gss_lock); 
    } else {
    	*st = rpc_s_ok;
    }
}

/*****************************************************************************/
/*
**++
**
**  ROUTINE NAME:       rpc__gssauth_cn_vfy_srvr_resp
**
**  SCOPE:    		  INTERNAL - declared locally
**
**  DESCRIPTION:
**      
**      This routine will decode auth_value field either an
**      rpc_bind_ack or rpc_alter_context_response PDU. If the
**      credentials field of the auth_value field contains an
**      authentication protocol specific encoding of an error
**      this will be returned as an error status code. This is
**      called from the client side association state machine.
**      Note that upon successful return the auth information
**      rep will contain the client's authorization protocol
**      and data.
**
**  INPUTS:
**
**      assoc_sec	   A pointer to per-association security context
**    				  including association UUID CRC and sequence numbers.
**      sec			 A pointer to security context element which includes
**    				  the key ID, auth information rep and RPC auth
**    				  information rep.
**      auth_value	  A pointer to the auth_value field in the rpc_bind or
**    				  rpc_alter_context PDU authentication trailer.
**      auth_value_len  The length, in bytes, of auth_value.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:    		
**
**      st			  The return status of this routine.
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

INTERNAL void rpc__gssauth_cn_vfy_srvr_resp 
#ifdef _DCE_PROTO_
(
    rpc_cn_assoc_sec_context_p_t	assoc_sec,
    rpc_cn_sec_context_p_t		  sec,
    pointer_t					   auth_value,
    unsigned32					  auth_value_len,
    unsigned32					  *st
)
#else
(assoc_sec, sec, auth_value, auth_value_len, st) 
rpc_cn_assoc_sec_context_p_t    assoc_sec;
rpc_cn_sec_context_p_t    	  sec;
pointer_t    				   auth_value;
unsigned32    				  auth_value_len;
unsigned32    				  *st;
#endif
{
    rpc_gssauth_cn_info_p_t		gssauth_cn_info;

    CODING_ERROR (st);

    gssauth_cn_info = (rpc_gssauth_cn_info_p_t) (sec->sec_cn_info);

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_cn_vfy_srvr_resp)\n"));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_vfy_server_resp) prot->%x level->%x key_id->%x assoc_uuid_crc->%x xmit_seq->%x recv_seq->%x\n",
    				sec->sec_info->authn_protocol,
    				sec->sec_info->authn_level,
    				sec->sec_key_id,
    				assoc_sec->assoc_uuid_crc,
    				assoc_sec->assoc_next_snd_seq,
    				assoc_sec->assoc_next_rcv_seq));

    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_PKT,
    				("(rpc__gssauth_cn_vfy_server_resp) auth_value->%p auth_value_len->%lx\n",
    				auth_value, auth_value_len));

#ifdef DEBUG
    if (RPC_DBG_EXACT(rpc_es_dbg_cn_errors,
    				  RPC_C_CN_DBG_AUTH_VFY_SERVER_RESP)) {
    	*st = RPC_S_CN_DBG_AUTH_FAILURE;
    	return;
    }
#endif

    if (gssauth_cn_info->cn_state < rpc_gssauth_cn_state_authenticated) {
    	/* have a buffer allocated for us. */
    	assoc_sec->krb_message.data = NULL;
    	assoc_sec->krb_message.length = 0;

        RPC_MUTEX_LOCK(rpc_g_gss_lock); 
    	rpc__gssauth_init_sec_context(assoc_sec,
    		sec,
    		auth_value,
    		auth_value_len,
    		(pointer_t) &assoc_sec->krb_message.data,
    		(unsigned32 *)&assoc_sec->krb_message.length,
    		st);
        RPC_MUTEX_UNLOCK(rpc_g_gss_lock); 
    } else {
    	*st = rpc_s_ok;
    }
}


PRIVATE rpc_protocol_id_t       rpc__gssauth_negotiate_cn_init
#ifdef _DCE_PROTO_
(
    rpc_auth_rpc_prot_epv_p_t	   *epv,
    unsigned32					  *st
)
#else
(epv, st)
rpc_auth_rpc_prot_epv_p_t       *epv;
unsigned32    				  *st;
#endif
{
    CODING_ERROR (st);
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_negotiate_cn_init)\n"));

    *epv = (rpc_auth_rpc_prot_epv_p_t) (&rpc_g_gssauth_negotiate_cn_epv);
    *st = rpc_s_ok;
    return (RPC_C_PROTOCOL_ID_NCACN);
}

PRIVATE rpc_protocol_id_t       rpc__gssauth_mskrb_cn_init
#ifdef _DCE_PROTO_
(
    rpc_auth_rpc_prot_epv_p_t	   *epv,
    unsigned32					  *st
)
#else
(epv, st)
rpc_auth_rpc_prot_epv_p_t       *epv;
unsigned32    				  *st;
#endif
{
    CODING_ERROR (st);
    RPC_DBG_PRINTF (rpc_e_dbg_auth, RPC_C_CN_DBG_AUTH_ROUTINE_TRACE,
    				("(rpc__gssauth_mskrb_cn_init)\n"));

    *epv = (rpc_auth_rpc_prot_epv_p_t) (&rpc_g_gssauth_mskrb_cn_epv);
    *st = rpc_s_ok;
    return (RPC_C_PROTOCOL_ID_NCACN);
}
