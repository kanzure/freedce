#ifndef _COMAUTHIMAGE_H
#define _COMAUTHIMAGE_H 1
/* for declaring a shared image */

typedef void (*rpc_module_auth_strap_func)(
		rpc_authn_protocol_id_t		* authn_id,
		rpc_auth_init_fn_t  			* auth_init,
		dce_rpc_authn_protocol_id_t	* dce_authn_id
		);

#define RPC_MODULE_INIT_AUTH_FUNC(id, init_func, dce_id )	\
		void rpc__module_init_auth_func(\
		rpc_authn_protocol_id_t		* authn_id,\
		rpc_auth_init_fn_t  			* auth_init,\
		dce_rpc_authn_protocol_id_t	* dce_authn_id) { \
			*authn_id = id; *auth_init = init_func; *dce_id = dce_authn_id; }

#endif
