#include <commonp.h>
#include <com.h>
#include <comp.h>
#include <dce/sec_authn.h>

/*
 * Size of buffer used when asking for remote server's principal name
 */
#define MAX_SERVER_PRINC_NAME_LEN 500

#define rpc_c_authn_ntlmssp					0xa
#define dce_c_rpc_authn_protocol_ntlmssp	0x0a


GLOBAL unsigned32 rpc_g_ntlmauth_alloc_count = 0;
GLOBAL unsigned32 rpc_g_ntlmauth_free_count = 0;

INTERNAL rpc_auth_rpc_prot_epv_p_t rpc_g_ntlmauth_rpc_prot_epv[RPC_C_PROTOCOL_ID_MAX];
EXTERNAL rpc_auth_epv_t rpc_g_ntlmauth_epv;


typedef struct rpc_ntlmauth_info_t	{
	rpc_auth_info_t	auth_info;	/* must be first */
	rpc_mutex_t	lock;
	unsigned32 status;
} rpc_ntlmauth_info_t, *rpc_ntlmauth_info_p_t;


/*
 * Set auth info for a binding handle
 * */

PRIVATE void rpc__ntlmauth_bnd_set_auth 
(
        unsigned_char_p_t server_name,
        rpc_authn_level_t level,
        rpc_auth_identity_handle_t auth_ident,
        rpc_authz_protocol_id_t authz_prot,
        rpc_binding_handle_t binding_h,
        rpc_auth_info_p_t *infop,
        unsigned32 *stp
)
{
    int st;
    rpc_ntlmauth_info_p_t ntlmauth_info;

    rpc_g_ntlmauth_alloc_count++;
    RPC_MEM_ALLOC (ntlmauth_info, rpc_ntlmauth_info_p_t, sizeof (*ntlmauth_info), RPC_C_MEM_UTIL, RPC_C_MEM_WAITOK);

    if (authz_prot != rpc_c_authz_none) 
    {
        st = rpc_s_authn_authz_mismatch;
        goto poison;
    }

    if (level != rpc_c_authn_level_none)
    {
        st = rpc_s_unsupported_authn_level;
        goto poison;
    }

    /*
     * If no server principal name was specified, go ask for it.
     */
    if (server_name == NULL)
    {
        rpc_mgmt_inq_server_princ_name
            (binding_h, 
             dce_c_rpc_authn_protocol_krb5,
             &server_name,
             stp);
        if (*stp != rpc_s_ok)
            return;
    } else {
        server_name = rpc_stralloc(server_name);
    }
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 1, (
            "(rpc__ntlmauth_bnd_set_auth) %x created (now %d active)\n", 
            ntlmauth_info, rpc_g_ntlmauth_alloc_count - rpc_g_ntlmauth_free_count));
    
    memset (ntlmauth_info, 0, sizeof(*ntlmauth_info));
    
    RPC_MUTEX_INIT(ntlmauth_info->lock);
    
    ntlmauth_info->auth_info.server_princ_name = server_name;
    ntlmauth_info->auth_info.authn_level = level;
    ntlmauth_info->auth_info.authn_protocol = rpc_c_authn_dce_dummy;
    ntlmauth_info->auth_info.authz_protocol = authz_prot;
    ntlmauth_info->auth_info.is_server = 0;
    ntlmauth_info->auth_info.u.auth_identity = auth_ident;
    
    ntlmauth_info->auth_info.refcount = 1;

    *infop = &ntlmauth_info->auth_info;
    ntlmauth_info->status = rpc_s_ok;
    *stp = rpc_s_ok;
    return;
poison:
    *infop = (rpc_auth_info_p_t) &ntlmauth_info->auth_info;
    ntlmauth_info->status = st;
    *stp = st;
    return;
}    
 
/*
 * R P C _ _ N O A U T H _ I N I T
 *
 * Initialize the world.
 */

PRIVATE void rpc__ntlmauth_init 
(
        rpc_auth_epv_p_t *epv,
        rpc_auth_rpc_prot_epv_tbl_t *rpc_prot_epv,
        unsigned32 *st
)
{
#if 0
    unsigned32                  prot_id;
    rpc_auth_rpc_prot_epv_t     *prot_epv;
#endif
    /*
     * Initialize the RPC-protocol-specific EPVs for the RPC protocols
     * we work with (ncadg and ncacn).
     */
#ifdef AUTH_DUMMY_DG
    prot_id = rpc__ntlmauth_dg_init (&prot_epv, st);
    if (*st == rpc_s_ok)
    {
        rpc_g_ntlmauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif
#ifdef AUTH_DUMMY_CN
    prot_id = rpc__ntlmauth_cn_init (&prot_epv, st);
    if (*st == rpc_s_ok)
    {
        rpc_g_ntlmauth_rpc_prot_epv[prot_id] = prot_epv;
    }
#endif

    /*
     * Return information for this authentication service.
     */
    *epv = &rpc_g_ntlmauth_epv;
    *rpc_prot_epv = rpc_g_ntlmauth_rpc_prot_epv;

    *st = 0;
}
#include <comp.h>
void rpc__module_init_func(void)
{
	static rpc_authn_protocol_id_elt_t auth[1] =	{
		{                               /* 0 */
        rpc__ntlmauth_init, 
        rpc_c_authn_ntlmssp,
        dce_c_rpc_authn_protocol_ntlmssp, 
        NULL,
		  NULL
    }	
	};
	rpc__register_authn_protocol(auth, 1);
}



/*
 * R P C _ _ N O A U T H _ F R E E _ I N F O
 *
 * Free info.
 */

PRIVATE void rpc__ntlmauth_free_info 
(
        rpc_auth_info_p_t *info
)
{
    rpc_ntlmauth_info_p_t ntlmauth_info = (rpc_ntlmauth_info_p_t)*info ;
    char *info_type = (*info)->is_server?"server":"client";
    unsigned32 tst;

    RPC_MUTEX_DELETE(ntlmauth_info->lock);

    if ((*info)->server_princ_name)
        rpc_string_free (&(*info)->server_princ_name, &tst);
    (*info)->u.s.privs = 0;
   // sec_id_pac_util_free (&ntlmauth_info->client_pac);

    memset (ntlmauth_info, 0x69, sizeof(*ntlmauth_info));
    RPC_MEM_FREE (ntlmauth_info, RPC_C_MEM_UTIL);
    rpc_g_ntlmauth_free_count++;
    RPC_DBG_PRINTF(rpc_e_dbg_auth, 1, (
        "(rpc__ntlmauth_release) freeing %s auth_info (now %d active).\n", 
        info_type, rpc_g_ntlmauth_alloc_count - rpc_g_ntlmauth_free_count));
    *info = NULL;
}


/*
 * R P C _ _ N O A U T H _ M G T _ I N Q _ D E F
 *
 * Return default authentication level
 *
 * !!! should read this from a config file.
 */

PRIVATE void rpc__ntlmauth_mgt_inq_def
(
        unsigned32 *authn_level,
        unsigned32 *stp
)
{
    *authn_level = rpc_c_authn_level_none;
    *stp = rpc_s_ok;
}


/*
 * R P C _ _ N O A U T H _ S R V _ R E G _ A U T H
 *
 */

PRIVATE void rpc__ntlmauth_srv_reg_auth 
(
        unsigned_char_p_t server_name __attribute__((unused)),
        rpc_auth_key_retrieval_fn_t get_key_func __attribute__((unused)),
        pointer_t arg __attribute__((unused)),
        unsigned32 *stp
)
{
    *stp = rpc_s_ok;
}


/*
 * R P C _ _ N O A U T H _ I N Q _ M Y _ P R I N C _ N A M E
 *
 * All this doesn't matter for this module, but we need the placebo.
 */

PRIVATE void rpc__ntlmauth_inq_my_princ_name 
(
        unsigned32 name_size,
        unsigned_char_p_t name,
        unsigned32 *stp
)
{
    if (name_size > 0) {
        rpc__strncpy(name, (unsigned char *)"", name_size - 1);
    }
    *stp = rpc_s_ok;
}

INTERNAL rpc_auth_epv_t rpc_g_ntlmauth_epv =
{
    rpc__ntlmauth_bnd_set_auth,
    rpc__ntlmauth_srv_reg_auth,
    rpc__ntlmauth_mgt_inq_def,
    rpc__ntlmauth_inq_my_princ_name,
	 rpc__ntlmauth_free_info,
	 NULL, /* free key */
	 NULL, /* auth_resolve_identity_fn_t get cc */
	 NULL /* auth_release_identity_fn_t cc free */
};



