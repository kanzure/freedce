#ifndef _COMNAFIMAGE_H
#define _COMNAFIMAGE_H 1
/* for declaring a shared image */

typedef void (*rpc_module_naf_strap_func)(
		rpc_naf_id_t		 * naf_id,
		rpc_naf_init_fn_t  * naf_init,
		rpc_network_if_id_t	* network_if_id
		);

#define RPC_MODULE_INIT_NAF_FUNC(id, init_func, if_id )	\
		void rpc__module_init_naf_func(\
		rpc_naf_id_t * naf_id, \
		rpc_naf_init_fn_t  * naf_init, \
		rpc_network_if_id_t	* network_if_id)	{ \
			*naf_id = id; *naf_init = init_func; *network_if_id = if_id; } 


#endif
