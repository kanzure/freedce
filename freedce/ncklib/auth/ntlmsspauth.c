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
/*
**
**  NAME
**
**      ntlmsspauth.c
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**      Client-side support of NTLMSSP module.
**
**
*/

#include <commonp.h>
#include <com.h>
#include <ntlmsspauthcn.h>
#include <ntlmsspauth.h>
#include <sec_id_pickle.h>
#include <unistd.h>
#include <ctype.h>

/*
 * Size of buffer used when asking for remote server's principal name
 */
#define MAX_SERVER_PRINC_NAME_LEN 500


GLOBAL unsigned32 rpc_g_ntlmsspauth_alloc_count = 0;
GLOBAL unsigned32 rpc_g_ntlmsspauth_free_count = 0;

INTERNAL rpc_auth_rpc_prot_epv_p_t rpc_g_ntlmsspauth_rpc_prot_epv[RPC_C_PROTOCOL_ID_MAX];

INTERNAL rpc_auth_epv_t rpc_g_ntlmsspauth_epv =
{
    rpc__ntlmsspauth_bnd_set_auth,
    rpc__ntlmsspauth_srv_reg_auth,
    rpc__ntlmsspauth_mgt_inq_def,
    rpc__ntlmsspauth_inq_my_princ_name,
    rpc__ntlmsspauth_free_info,
    rpc__ntlmsspauth_free_key,
    rpc__ntlmsspauth_resolve_identity,
    rpc__ntlmsspauth_release_identity
};


/*
 * R P C _ _ N T L M S S P A U T H _ B N D _ S E T _ A U T H
 *
 */

PRIVATE void rpc__ntlmsspauth_bnd_set_auth 
#ifdef _DCE_PROTO_
(
        unsigned_char_p_t server_name,
        rpc_authn_level_t level,
        rpc_auth_identity_handle_t auth_ident,
        rpc_authz_protocol_id_t authz_prot,
        rpc_binding_handle_t binding_h,
        rpc_auth_info_p_t *infop,
        unsigned32 *stp
)
#else
(server_name, level, auth_ident, authz_prot, binding_h, infop, stp)
    unsigned_char_p_t server_name;
    rpc_authn_level_t level;
    rpc_auth_identity_handle_t auth_ident;
    rpc_authz_protocol_id_t authz_prot;
    rpc_binding_handle_t binding_h;
    rpc_auth_info_p_t *infop;
    unsigned32 *stp;
#endif
{
    int st;
    rpc_ntlmsspauth_info_p_t ntlmsspauth_info;

    rpc_g_ntlmsspauth_alloc_count++;
    RPC_MEM_ALLOC (ntlmsspauth_info, rpc_ntlmsspauth_info_p_t, sizeof (*ntlmsspauth_info), RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);

    RPC_DBG_PRINTF(rpc_e_dbg_auth, 20, (
            "(rpc__ntlmsspauth_bnd_set_auth) level: %d authz_prot: %d\n", 
	    level, authz_prot));
    if (authz_prot != rpc_c_authz_mspac)
    {
        st = rpc_s_authn_authz_mismatch;
        goto poison;
    }

    /*
    if (level != rpc_c_authn_level_pkt_privacy &&
		 level != rpc_c_authn_level_connect)
    {
        st = rpc_s_unsupported_authn_level;
        goto poison;
    }
    */
    /*
     * If no server principal name was specified, go ask for it.
     */
    if (server_name == NULL)
    {
        rpc_mgmt_inq_server_princ_name
            (binding_h, 
             dce_c_rpc_authn_protocol_winnt,
             &server_name,
             stp);
        if (*stp != rpc_s_ok)
            return;
    } else {
        server_name = rpc_stralloc(server_name);
    }
                                    
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 1, (
            "(rpc__ntlmsspauth_bnd_set_auth) %x created (now %d active)\n", 
            ntlmsspauth_info, rpc_g_ntlmsspauth_alloc_count - rpc_g_ntlmsspauth_free_count));
    
    memset (ntlmsspauth_info, 0, sizeof(*ntlmsspauth_info));
    
    RPC_MUTEX_INIT(ntlmsspauth_info->lock);
    
    ntlmsspauth_info->auth_info.server_princ_name = server_name;
    ntlmsspauth_info->auth_info.authn_level = level;
    ntlmsspauth_info->auth_info.authn_protocol = rpc_c_authn_winnt;
    ntlmsspauth_info->auth_info.authz_protocol = authz_prot;
    ntlmsspauth_info->auth_info.is_server = 0;
    ntlmsspauth_info->auth_info.u.auth_identity = auth_ident;
    
    ntlmsspauth_info->auth_info.refcount = 1;

    ntlmsspauth_info->creds_valid = 1;       /* XXX what is this used for? */
    ntlmsspauth_info->level_valid = 1;
    ntlmsspauth_info->client_valid = 1;      /* sort of.. */

    *infop = &ntlmsspauth_info->auth_info;
    ntlmsspauth_info->status = rpc_s_ok;
    *stp = rpc_s_ok;
    return;
poison:
    *infop = (rpc_auth_info_p_t) &ntlmsspauth_info->auth_info;
    ntlmsspauth_info->status = st;
    *stp = st;
    return;
}    
 
/*
 * R P C _ _ N T L M S S P A U T H _ I N I T
 *
 * Initialize the world.
 */

PRIVATE void rpc__ntlmsspauth_init 
#ifdef _DCE_PROTO_
(
        rpc_auth_epv_p_t *epv,
        rpc_auth_rpc_prot_epv_tbl_t *rpc_prot_epv,
        unsigned32 *st
)
#else
(epv, rpc_prot_epv, st)
    rpc_auth_epv_p_t *epv;
    rpc_auth_rpc_prot_epv_tbl_t *rpc_prot_epv;
    unsigned32 *st;
#endif
{
    unsigned32                  prot_id;
    rpc_auth_rpc_prot_epv_t     *prot_epv;

    /*
     * Initialize the RPC-protocol-specific EPVs for the RPC protocols
     * we work with (ncadg and ncacn).
     */
#ifdef AUTH_NTLMSSP_DG
    prot_id = rpc__ntlmsspauth_dg_init (&prot_epv, st);
    if (*st == rpc_s_ok)
    {
        rpc_g_ntlmsspauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif
#ifdef AUTH_NTLMSSP_CN
    prot_id = rpc__ntlmsspauth_cn_init (&prot_epv, st);
    if (*st == rpc_s_ok)
    {
        rpc_g_ntlmsspauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif

    /*
     * Return information for this (NTLMSSP) authentication service.
     */
    *epv = &rpc_g_ntlmsspauth_epv;
    *rpc_prot_epv = rpc_g_ntlmsspauth_rpc_prot_epv;

    *st = 0;
}

#include <comp.h>
void rpc__module_init_func(void)
{
	static rpc_authn_protocol_id_elt_t auth[1] =	{
		{                               /* 0 */
        rpc__ntlmsspauth_init, 
        rpc_c_authn_winnt,	/* FIXME: probably incorrect */ 
        dce_c_rpc_authn_protocol_winnt, 
        &rpc_g_ntlmsspauth_epv,
		  NULL
    }	
	};
	rpc__register_authn_protocol(auth, 1);
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 3, (
        "(rpc__module_init_func) id %d\n", 
	 rpc_g_authn_protocol_id[rpc_c_authn_winnt].authn_protocol_id));
}


/*
 * R P C _ _ N T L M S S P A U T H _ F R E E _ I N F O
 *
 * Free info.
 */

PRIVATE void rpc__ntlmsspauth_free_info 
#ifdef _DCE_PROTO_
(
        rpc_auth_info_p_t *info
)
#else
(info)
    rpc_auth_info_p_t *info;
#endif
{
    rpc_ntlmsspauth_info_p_t ntlmsspauth_info = (rpc_ntlmsspauth_info_p_t)*info ;
    char *info_type = (*info)->is_server?"server":"client";
    unsigned32 tst;

    RPC_MUTEX_DELETE(ntlmsspauth_info->lock);

    if ((*info)->server_princ_name)
        rpc_string_free (&(*info)->server_princ_name, &tst);
    (*info)->u.s.privs = 0;
    if (ntlmsspauth_info->client_name)
        rpc_string_free (&ntlmsspauth_info->client_name, &tst);
#if 0
    sec_id_pac_util_free (&ntlmsspauth_info->client_pac);
#endif

    memset (ntlmsspauth_info, 0x69, sizeof(*ntlmsspauth_info));
    RPC_MEM_FREE (ntlmsspauth_info, RPC_C_MEM_UTIL);
    rpc_g_ntlmsspauth_free_count++;
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 1, (
        "(rpc__ntlmsspauth_release) freeing %s auth_info (now %d active).\n", 
        info_type, rpc_g_ntlmsspauth_alloc_count - rpc_g_ntlmsspauth_free_count));
    *info = NULL;
}


/*
 * R P C _ _ N T L M S S P A U T H _ M G T _ I N Q _ D E F
 *
 * Return default authentication level
 *
 * !!! should read this from a config file.
 */

PRIVATE void rpc__ntlmsspauth_mgt_inq_def
#ifdef _DCE_PROTO_
(
        unsigned32 *authn_level,
        unsigned32 *stp
)
#else
(authn_level, stp)
    unsigned32 *authn_level;
    unsigned32 *stp;
#endif
{
    *authn_level = rpc_c_authn_level_none;
    *stp = rpc_s_ok;
}


/*
 * R P C _ _ N T L M S S P A U T H _ S R V _ R E G _ A U T H
 *
 */

PRIVATE void rpc__ntlmsspauth_srv_reg_auth 
#ifdef _DCE_PROTO_
(
        unsigned_char_p_t server_name,
        rpc_auth_key_retrieval_fn_t get_key_func,
        pointer_t arg,
        unsigned32 *stp
)
#else
(server_name, get_key_func, arg, stp)
    unsigned_char_p_t server_name;
    rpc_auth_key_retrieval_fn_t get_key_func;
    pointer_t arg;
    unsigned32 *stp;
#endif
{
    if (server_name != NULL)
    {
	    RPC_DBG_PRINTF(rpc_e_dbg_auth, 20, (
		    "(rpc__ntlmsspauth_srv_reg_auth) srv: %s\n", 
	            server_name));
    }

    get_key_func = NULL;
    arg = NULL;

    *stp = rpc_s_ok;
}


/*
 * R P C _ _ N T L M S S P A U T H _ I N Q _ M Y _ P R I N C _ N A M E
 *
 * name is host name, upper-cased. 
 */

PRIVATE void rpc__ntlmsspauth_inq_my_princ_name 
#ifdef _DCE_PROTO_
(
        unsigned32 name_size,
        unsigned_char_p_t name,
        unsigned32 *stp
)
#else
(name_size, name, stp)
    unsigned32 name_size;
    unsigned_char_p_t name;
    unsigned32 *stp;
#endif
{
	unsigned32 i;
    if (name_size > 0) {
		gethostname(name, name_size - 1);
		fprintf(stderr, "hostname: %s\n", name);
		if (gethostname(name, name_size - 1) != 0)
		{
			*stp = rpc_s_string_too_long;
			return;
		}
		for (i = 0; i < name_size - 1; i++)
		{
			name[i] = toupper(name[i]);
		}

        rpc__strncpy(name, (unsigned char *)"", name_size - 1);
    }
    *stp = rpc_s_ok;
}

/*
 * sec_ntlmsspauth_resolve_identity: create reference to credential cache.
 *
 * This just copies the information, if any, and allocates memory for it.
 */

error_status_t rpc__ntlmsspauth_resolve_identity (auth_ident, out_identity)
    rpc_auth_identity_handle_t auth_ident;
    rpc_auth_identity_handle_t *out_identity;
{
	auth_ident = NULL;
#if 0
    cchp crp;
    unsigned32 uid, pag;

    if (auth_ident == NULL) {
        uid = krpc_osi_GetUID(krpc_osi_getucred());
	pag = uid;
    } else {
        unsigned32 *ppagp = (unsigned32 *)auth_ident;
        pag = ppagp[0];
        uid = ppagp[1];
    }

    RPC_MUTEX_LOCK(credlist_lock);
    for (crp = sec_ntlmsspauth_cred_list; crp; crp = crp->next) 
    {
	if ((crp->pag == pag) && (crp->uid == uid)) 
	{
	    /* bump refcount */
	    crp->refcount++;
	    break;
	}
    }
    
    if (crp == NULL) 
    {
	RPC_MEM_ALLOC(crp, cchp, sizeof(cch),
		      RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);
	crp->next = sec_ntlmsspauth_cred_list;
	crp->uid = uid;
	crp->pag = pag;
	crp->refcount = 1;
	sec_ntlmsspauth_cred_list = crp;
    }
    RPC_MUTEX_UNLOCK(credlist_lock);
    *ccache = (sec_ntlmsspauth_ccache) crp;
#endif
    *out_identity = (rpc_auth_identity_handle_t) 1;
    return error_status_ok;
}

/*
 * rpc__ntlmsspauth_release_identity: drop reference to credential cache.
 *
 * Free memory used, if any.
 */

void rpc__ntlmsspauth_release_identity (auth_identity)
    rpc_auth_identity_handle_t *auth_identity;
{
    if (auth_identity == NULL)
	return;
    *auth_identity = NULL;
#if 0
    cchp crp = (cchp)*ccache;
    cchp crpp;
    int ref;

    if (crp == NULL)
	return;
    *ccache = NULL;
    RPC_MUTEX_LOCK(credlist_lock);
    ref = crp->refcount-1;
    crp->refcount = ref;
    if (ref != 0)
	crp = NULL;		/* don't free crp.. */
    else {
	if (crp == sec_ntlmsspauth_cred_list)
	    sec_ntlmsspauth_cred_list = crp->next;
	else 
	{
	    for (crpp = sec_ntlmsspauth_cred_list; crpp; crpp=crpp->next)
		if (crpp->next == crp)
		    break;
	    if ((!crpp) || (crpp->next != crp))
		panic("sec_authn_krpc: cred list botch");
	    crpp->next = crp->next;
	}
    }
    RPC_MUTEX_UNLOCK(credlist_lock);
    if (crp)
	RPC_MEM_FREE(crp, RPC_C_MEM_UTIL);
#endif
}
    
/*
 * RPC__NTLMSSPAUTH_FREE_KEY
 *
 */

PRIVATE void rpc__ntlmsspauth_free_key 
#ifdef _DCE_PROTO_
(
        rpc_key_info_p_t *key
)
#else
(key)
    rpc_key_info_p_t *key;
#endif
{
    RPC_MEM_FREE(key, RPC_C_MEM_UTIL);
#if 0
    rpc_ntlmsspauth_info_p_t ntlmsspauth_key = (rpc_ntlmsspauth_info_p_t) *key;

    rpc__auth_info_release (&ntlmsspauth_key->c.auth_info);

    /* Zero out encryption keys */
    memset (ntlmsspauth_key, 0, sizeof(*ntlmsspauth_key));
    
    RPC_MEM_FREE(ntlmsspauth_key, RPC_C_MEM_UTIL);
#endif
}
