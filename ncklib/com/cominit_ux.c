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

/* For now, we hard-code the list of module names; in the near
 * future we will need a config file listing the modules to load,
 * otherwise there is not much point to doing things this way... */
PRIVATE void rpc__load_modules(void)
{
#if HAVE_DLFCN_H
#include <dlfcn.h>
#include <comnafimage.h>
#include <comauthimage.h>
	const char * modules[] = { "libnaf_ip.so", NULL };
	int i;
	void * image;
	char buf[PATH_MAX];

	for (i=0; modules[i] != NULL; i++)	{
		sprintf(buf, "%s/%s", IMAGE_DIR, modules[i]);
		image = dlopen(buf, RTLD_LAZY);
		if (image)	{
			rpc_module_naf_strap_func naf_func;
			rpc_module_auth_strap_func auth_func;
			
			naf_func = dlsym(image, "rpc__module_init_naf_func");
			if (naf_func)	{
				rpc_naf_id_t naf_id;
				rpc_naf_init_fn_t naf_init;
				rpc_network_if_id_t if_id;

				(*naf_func)(&naf_id, &naf_init, &if_id);
				rpc_g_naf_id[naf_id].naf_id = naf_id;
				rpc_g_naf_id[naf_id].naf_init = naf_init;
				rpc_g_naf_id[naf_id].network_if_id = if_id;
			}

			auth_func = dlsym(image, "rpc__module_init_auth_func");
			if (auth_func)	{
				rpc_authn_protocol_id_t			authn_id;
				rpc_auth_init_fn_t 				auth_init;
				dce_rpc_authn_protocol_id_t	dce_authn_id;
				
				(*auth_func)(&authn_id, &auth_init, &dce_authn_id);

				rpc_g_authn_protocol_id[authn_id].authn_protocol_id = authn_id;
				rpc_g_authn_protocol_id[authn_id].auth_init = auth_init;
				rpc_g_authn_protocol_id[authn_id].dce_rpc_authn_protocol_id = dce_authn_id;
			}
		}
		else
			printf("failed to load module %s %s\n", buf, dlerror());
	}
#endif
}



/* 
 * Routines for loading Network Families and Protocol Families as shared
 * images.  i.e., VMS
 */

PRIVATE rpc_naf_init_fn_t  rpc__load_naf
(
  rpc_naf_id_elt_p_t      naf __attribute__((__unused__)),
  unsigned32              *status
)
{
	*status = rpc_s_ok;
	return((rpc_naf_init_fn_t)NULL);
}


PRIVATE rpc_prot_init_fn_t  rpc__load_prot
(
    rpc_protocol_id_elt_p_t prot __attribute__((__unused__)),
    unsigned32              *status
)
{
    *status = rpc_s_ok;
    return((rpc_prot_init_fn_t)NULL);
}

PRIVATE rpc_auth_init_fn_t  rpc__load_auth
(
    rpc_authn_protocol_id_elt_p_t auth __attribute__((__unused__)),
    unsigned32              *status
)
{
    *status = rpc_s_ok;
    return((rpc_auth_init_fn_t)NULL);
}
