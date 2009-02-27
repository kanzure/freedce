/*
 * 
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
 * (c) Copyright 2001 PADL SOFTWARE PTY. LTD.
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *                 permission to use, copy, modify, and distribute this
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
**
**  NAME
**
**      gssauth.c
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**      Client-side support of GSS-API module.
**
**
*/

#include <commonp.h>
#include <com.h>
#include <gssauthcn.h>
#include <gssauth.h>
#include <sec_id_pickle.h>
#include <unistd.h>
#include <ctype.h>

/*
 * Size of buffer used when asking for remote server's principal name
 */
#define MAX_SERVER_PRINC_NAME_LEN 500

GLOBAL unsigned32 rpc_g_gssauth_alloc_count = 0;
GLOBAL unsigned32 rpc_g_gssauth_free_count = 0;

INTERNAL rpc_auth_rpc_prot_epv_p_t rpc_g_gssauth_rpc_prot_epv[RPC_C_PROTOCOL_ID_MAX];

typedef struct rpc__gss_defaults {
    unsigned_char_p_t server_name;
    rpc_auth_key_retrieval_fn_t get_key_func;
    pointer_t arg;
} rpc__gss_defaults_t, *rpc__gss_defaults_p_t;

INTERNAL rpc__gss_defaults_p_t rpc_g_gssauth_defaults = NULL;

INTERNAL rpc_auth_epv_t rpc_g_gssauth_negotiate_epv =
{
    rpc__gssauth_bnd_set_auth_negotiate,
    rpc__gssauth_srv_reg_auth,
    rpc__gssauth_mgt_inq_def,
    rpc__gssauth_inq_my_princ_name,
    rpc__gssauth_free_info,
    rpc__gssauth_free_key,
    rpc__gssauth_resolve_identity,
    rpc__gssauth_release_identity
};

INTERNAL rpc_auth_epv_t rpc_g_gssauth_mskrb_epv =
{
    rpc__gssauth_bnd_set_auth_mskrb,
    rpc__gssauth_srv_reg_auth,
    rpc__gssauth_mgt_inq_def,
    rpc__gssauth_inq_my_princ_name,
    rpc__gssauth_free_info,
    rpc__gssauth_free_key,
    rpc__gssauth_resolve_identity,
    rpc__gssauth_release_identity
};

/*
 * R P C _ _ G S S A U T H _ B N D _ S E T _ A U T H
 *
 */

PRIVATE void rpc__gssauth_bnd_set_auth 
#ifdef _DCE_PROTO_
(
        unsigned_char_p_t server_name,
        rpc_authn_level_t level,
        rpc_authn_protocol_id_t authn_prot,
        rpc_auth_identity_handle_t auth_ident,
        rpc_authz_protocol_id_t authz_prot,
        rpc_binding_handle_t binding_h,
        rpc_auth_info_p_t *infop,
        unsigned32 *stp
)
#else
(server_name, level, authn_prot, auth_ident, authz_prot, binding_h, infop, stp)
    unsigned_char_p_t server_name;
    rpc_authn_level_t level;
    rpc_authn_protocol_id_t authn_prot;
    rpc_auth_identity_handle_t auth_ident;
    rpc_authz_protocol_id_t authz_prot;
    rpc_binding_handle_t binding_h;
    rpc_auth_info_p_t *infop;
    unsigned32 *stp;
#endif
{
    unsigned32 st;
    rpc_gssauth_info_p_t gssauth_info;
    OM_uint32 min_stat, maj_stat;

    rpc_g_gssauth_alloc_count++;
    RPC_MEM_ALLOC (gssauth_info, rpc_gssauth_info_p_t, sizeof (*gssauth_info),
        RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);

    RPC_DBG_PRINTF(rpc_e_dbg_auth, 20, (
            "(rpc__gssauth_bnd_set_auth) level: %d authz_prot: %d\n", 
        level, authz_prot));
    if (authz_prot == rpc_c_authz_dce) {
        st = rpc_s_authn_authz_mismatch;
        goto poison;
    }

    /*
     * If no server principal name was specified, go ask for it.
     */
    if (server_name == NULL) {
        rpc_mgmt_inq_server_princ_name
            (binding_h, 
             authz_prot,
             &server_name,
             stp);
        if (*stp != rpc_s_ok)
            return;
    } else {
        server_name = rpc_stralloc(server_name);
    }
                                    
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 1, (
            "(rpc__gssauth_bnd_set_auth) %x created (now %d active)\n", 
            gssauth_info, rpc_g_gssauth_alloc_count - rpc_g_gssauth_free_count));
    
    memset (gssauth_info, 0, sizeof(*gssauth_info));
    
    RPC_MUTEX_INIT(gssauth_info->lock);
    
    gssauth_info->auth_info.server_princ_name = server_name;
    gssauth_info->auth_info.authn_level = level;

    gssauth_info->auth_info.authn_protocol = authn_prot;
    gssauth_info->auth_info.authz_protocol = authz_prot;
    gssauth_info->auth_info.is_server = 0;
    gssauth_info->auth_info.refcount = 1;

    if (auth_ident != NULL) {
        maj_stat = gss_acquire_cred(&min_stat, (const gss_name_t)auth_ident,
            GSS_C_INDEFINITE, GSS_C_NO_OID_SET, GSS_C_ACCEPT,
            (gss_cred_id_t *)&gssauth_info->auth_info.u.auth_identity, 
            NULL, NULL);
        if (GSS_ERROR(maj_stat)) {
            st = rpc_s_invalid_credentials;
            goto poison;
        }
    } else {
        gssauth_info->auth_info.u.auth_identity = NULL;
    }

    memset(&gssauth_info->client_pac, 0, sizeof(gssauth_info->client_pac));

    gssauth_info->creds_valid = 1;       /* XXX what is this used for? */
    gssauth_info->level_valid = 1;
    gssauth_info->client_valid = 1;      /* sort of.. */

    gssauth_info->server_creds = GSS_C_NO_CREDENTIAL;

    *infop = &gssauth_info->auth_info;
    gssauth_info->status = rpc_s_ok;
    *stp = rpc_s_ok;
    return;

poison:
    *infop = (rpc_auth_info_p_t) &gssauth_info->auth_info;
    gssauth_info->status = st;
    *stp = st;
    return;
}    
 
PRIVATE void rpc__gssauth_bnd_set_auth_negotiate
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
    return rpc__gssauth_bnd_set_auth(server_name, level, rpc_c_authn_gss_negotiate,
        auth_ident, authz_prot, binding_h, infop, stp);
}

PRIVATE void rpc__gssauth_bnd_set_auth_mskrb
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
    return rpc__gssauth_bnd_set_auth(server_name, level, rpc_c_authn_gss_mskrb,
        auth_ident, authz_prot, binding_h, infop, stp);
}

/*
 * R P C _ _ G S S A U T H _ N E G O T I A T E _ I N I T
 *
 * Initialize the world.
 */

PRIVATE void rpc__gssauth_negotiate_init 
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
#ifdef AUTH_GSS_DG
    prot_id = rpc__gssauth_negotiate_dg_init (&prot_epv, st);
    if (*st == rpc_s_ok) {
        rpc_g_gssauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif
#ifdef AUTH_GSS_CN
    prot_id = rpc__gssauth_negotiate_cn_init (&prot_epv, st);
    if (*st == rpc_s_ok) {
        rpc_g_gssauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif

    /*
     * Return information for this (GSS) authentication service.
     */
    *epv = &rpc_g_gssauth_negotiate_epv;
    *rpc_prot_epv = rpc_g_gssauth_rpc_prot_epv;

    *st = 0;
}

/*
 * R P C _ _ G S S A U T H _ M S K R B _ I N I T
 *
 * Initialize the world.
 */

PRIVATE void rpc__gssauth_mskrb_init
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
#ifdef AUTH_GSS_DG
    prot_id = rpc__gssauth_mskrb_dg_init (&prot_epv, st);
    if (*st == rpc_s_ok) {
        rpc_g_gssauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif
#ifdef AUTH_GSS_CN
    prot_id = rpc__gssauth_mskrb_cn_init (&prot_epv, st);
    if (*st == rpc_s_ok) {
        rpc_g_gssauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif

    /*
     * Return information for this (GSS) authentication service.
     */
    *epv = &rpc_g_gssauth_mskrb_epv;
    *rpc_prot_epv = rpc_g_gssauth_rpc_prot_epv;

    *st = 0;
}

#include <comp.h>
void rpc__module_init_func(void)
{
    static rpc_authn_protocol_id_elt_t auth[2] =    {
    {
        rpc__gssauth_negotiate_init,
        rpc_c_authn_gss_negotiate,
        dce_c_rpc_authn_protocol_gss_negotiate,
        &rpc_g_gssauth_negotiate_epv,
        NULL
    },
    {
        rpc__gssauth_mskrb_init,
        rpc_c_authn_gss_mskrb,
        dce_c_rpc_authn_protocol_gss_mskrb,
        &rpc_g_gssauth_mskrb_epv,
        NULL
    }
    };
    rpc__register_authn_protocol(auth, 2);
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 3, (
        "(rpc__module_init_func) id {%d,%d}\n", 
        rpc_g_authn_protocol_id[rpc_c_authn_gss_negotiate].authn_protocol_id,
        rpc_g_authn_protocol_id[rpc_c_authn_gss_mskrb].authn_protocol_id));
}


/*
 * R P C _ _ G S S A U T H _ F R E E _ I N F O
 *
 * Free info.
 */

PRIVATE void rpc__gssauth_free_info 
#ifdef _DCE_PROTO_
(
        rpc_auth_info_p_t *info
)
#else
(info)
    rpc_auth_info_p_t *info;
#endif
{
    rpc_gssauth_info_p_t gssauth_info = (rpc_gssauth_info_p_t)*info ;
    unsigned32 tst;
    OM_uint32 min_stat;
#ifdef DEBUG
    char *info_type = (*info)->is_server?"server":"client";
#endif

    RPC_MUTEX_DELETE(gssauth_info->lock);

    if ((*info)->server_princ_name)
        rpc_string_free (&(*info)->server_princ_name, &tst);
    (*info)->u.s.privs = 0;

    if (gssauth_info->client_name != GSS_C_NO_NAME) {
        gss_release_name(&min_stat, &gssauth_info->client_name);
    }
    if (gssauth_info->server_creds != GSS_C_NO_CREDENTIAL) {
        gss_release_cred(&min_stat, &gssauth_info->server_creds);
    }
    if (gssauth_info->auth_info.u.auth_identity != NULL) {
        gss_release_name(&min_stat,
            (gss_name_t *)&gssauth_info->auth_info.u.auth_identity);
    }
    sec_id_pac_free (&gssauth_info->client_pac);

    memset (gssauth_info, 0x69, sizeof(*gssauth_info));
    RPC_MEM_FREE (gssauth_info, RPC_C_MEM_UTIL);
    rpc_g_gssauth_free_count++;
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 1, (
        "(rpc__gssauth_release) freeing %s auth_info (now %d active).\n", 
        info_type, rpc_g_gssauth_alloc_count - rpc_g_gssauth_free_count));
    *info = NULL;
}


/*
 * R P C _ _ G S S A U T H _ M G T _ I N Q _ D E F
 *
 * Return default authentication level
 *
 * !!! should read this from a config file.
 */

PRIVATE void rpc__gssauth_mgt_inq_def
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
 * R P C _ _ G S S A U T H _ S R V _ R E G _ A U T H
 *
 */

PRIVATE void rpc__gssauth_srv_reg_auth 
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
    if (rpc_g_gssauth_defaults == NULL) {
        rpc_string_free(&rpc_g_gssauth_defaults->server_name, stp);
        RPC_MEM_FREE(rpc_g_gssauth_defaults, RPC_C_MEM_UTIL);
    }

    RPC_MEM_ALLOC(rpc_g_gssauth_defaults, rpc__gss_defaults_p_t,
        sizeof(*rpc_g_gssauth_defaults), RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);

    rpc_g_gssauth_defaults->server_name = rpc__stralloc(server_name);
    rpc_g_gssauth_defaults->get_key_func = get_key_func;
    rpc_g_gssauth_defaults->arg = arg;

    *stp = rpc_s_ok;
}


/*
 * R P C _ _ G S S A U T H _ G E T _ A C C E P T O R _ C R E D
 */

PRIVATE void rpc__gssauth_get_acceptor_cred 
#ifdef _DCE_PROTO_
(
        rpc_gssauth_info_p_t gssauth_info,
        unsigned32 *st
)
#else
(info, stp)
    rpc_gssauth_info_p_t gssauth_info,
    unsigned32 *st;
#endif
{
    gss_buffer_desc server_name_token;
    OM_uint32 maj_stat, min_stat;
    unsigned_char_t namebuf[MAX_SERVER_PRINC_NAME_LEN];

    if (rpc_g_gssauth_defaults == NULL) {
        rpc__gssauth_inq_my_princ_name(sizeof(namebuf), namebuf, st);
        if (*st != rpc_s_ok)
            return;
        server_name_token.value = namebuf;
    } else {
        server_name_token.value = rpc_g_gssauth_defaults->server_name;
    }

    /*
     * Refresh cached server name.
     */
    if (gssauth_info->server_name != GSS_C_NO_NAME) {
        maj_stat = gss_release_name(&min_stat, &gssauth_info->server_name);
        gssauth_info->server_name = GSS_C_NO_NAME;
    }

    server_name_token.length = strlen(server_name_token.value);
    maj_stat = gss_import_name(&min_stat, &server_name_token,
        GSS_C_NT_HOSTBASED_SERVICE, &gssauth_info->server_name);
    if (GSS_ERROR(maj_stat)) {
        *st = rpc_s_invalid_name_syntax;
        return;
    }

    /*
     * Acquire server credentials and cache them.
     */
    if (gssauth_info->server_creds != GSS_C_NO_CREDENTIAL) {
        maj_stat = gss_release_cred(&min_stat, &gssauth_info->server_creds);
        gssauth_info->server_creds = GSS_C_NO_CREDENTIAL;
    }
    maj_stat = gss_acquire_cred(&min_stat,
        gssauth_info->server_name,
        GSS_C_INDEFINITE,
        GSS_C_NO_OID_SET,
        GSS_C_ACCEPT,
        &gssauth_info->server_creds,
        NULL,
        NULL);
    if (GSS_ERROR(maj_stat)) {
        *st = rpc_s_invalid_credentials;
        return;
    }
}

/*
 * R P C _ _ G S S A U T H _ I N Q _ M Y _ P R I N C _ N A M E
 *
 * Return the server principal name.
 */

PRIVATE void rpc__gssauth_inq_my_princ_name 
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
    if (name_size > 6) { 
        name_size -= 6;

        rpc__strncpy(name, "host/", 5);
        if (gethostname(name + 5, name_size) != 0) {
            *stp = rpc_s_string_too_long;
            return;
        }
        *stp = rpc_s_ok;
    } else {
        *stp = rpc_s_string_too_long;
    }
}

/*
 * rpc__gssauth_resolve_identity: create reference to credential cache.
 *
 * This just copies the information, if any, and allocates memory for it.
 * XXX: no portable between GSS-API implementations (requires Heimdal)
 */

error_status_t rpc__gssauth_resolve_identity (auth_ident, out_identity)
    rpc_auth_identity_handle_t auth_ident;
    rpc_auth_identity_handle_t *out_identity;
{
    OM_uint32 maj_stat, min_stat;

    /*
     * This casting depends on the fact that the internal representation
     * of an authentication identity handle is a gss_cred_id_t, and that
     * under Heimdal the first member of a gss_cred_id_t is the
     * gss_name_t that is the principal name.
     *
     * This would have to be reworked to use another GSS library such
     * as Heimdal but for the purposes of this implementation it 
     * is sufficient to mandate Heimdal (as we need it for authorization
     * data and LDAP support.
     */

    if (auth_ident != NULL) {
        maj_stat = gss_acquire_cred(&min_stat, (const gss_name_t)auth_ident,
            GSS_C_INDEFINITE, GSS_C_NO_OID_SET, GSS_C_ACCEPT,
            (gss_cred_id_t *)out_identity, NULL, NULL);
        if (GSS_ERROR(maj_stat)) {
            return rpc_s_invalid_credentials;
        }
    } else {
        *out_identity = NULL; /* GSS_C_NO_CREDENTIAL */
    }
        
    return error_status_ok;
}

/*
 * rpc__gssauth_release_identity: drop reference to credential cache.
 *
 * Free memory used, if any.
 */

void rpc__gssauth_release_identity (auth_identity)
    rpc_auth_identity_handle_t *auth_identity;
{
    OM_uint32 maj_stat, min_stat;

    maj_stat = gss_release_cred(&min_stat, (gss_cred_id_t *)auth_identity);
}
    
/*
 * RPC__GSSAUTH_FREE_KEY
 *
 */

PRIVATE void rpc__gssauth_free_key 
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
    rpc_gssauth_info_p_t gssauth_key = (rpc_gssauth_info_p_t) *key;

    rpc__auth_info_release (&gssauth_key->c.auth_info);

    /* Zero out encryption keys */
    memset (gssauth_key, 0, sizeof(*gssauth_key));
    
    RPC_MEM_FREE(gssauth_key, RPC_C_MEM_UTIL);
#endif
}
