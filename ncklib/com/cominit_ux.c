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

#if HAVE_DLFCN_H
#include <dirent.h>
/* load protocols before naf, then auth */
static int sort_modules(const void* a, const void* b)
{
	int pri_a, pri_b;

	switch((*(const struct dirent**)a)->d_name[3])	{
		case 'p': pri_a = 1; break;
		case 'n': pri_a = 2; break;
		case 'a': pri_a = 3; break;
		default: pri_a = 4;
	}
	switch((*(const struct dirent**)b)->d_name[3])	{
		case 'p': pri_b = 1; break;
		case 'n': pri_b = 2; break;
		case 'a': pri_b = 3; break;
		default: pri_b = 4;
	}
	if (pri_a == pri_b)
		return 0;
	if (pri_a < pri_b)
		return -1;
	return 1;
}
static int select_module(const struct dirent * dirent)
{
	int len = strlen(dirent->d_name);
	const char * module_types[] = {"libnaf_", "libauth_", "libprot_", NULL};
	int i;

	for (i=0; module_types[i] != NULL; i++)	{
		if (strncmp(dirent->d_name, module_types[i], strlen(module_types[i])) == 0)	{
			/* prefix matches; now check for a matching suffix */
			if (strcmp(&dirent->d_name[len-3], ".so") == 0)
				return 1;
		}
	}
	/* reject */
	return 0;
}
#endif

/* register an auth protocol */
PRIVATE void rpc__register_authn_protocol(rpc_authn_protocol_id_elt_p_t auth, int number)
{
	int i;
	for (i=0; i<number; i++)	{
		RPC_DBG_PRINTF(rpc_es_dbg_general, 1, ("Register authn protocol 0x%0x\n", auth[i].authn_protocol_id));	

		memcpy(&rpc_g_authn_protocol_id[auth[i].authn_protocol_id],
				&auth[i],
				sizeof(rpc_authn_protocol_id_elt_t)
				);
	}
}

/* register a protocol sequence with the runtime */
PRIVATE void rpc__register_protseq(rpc_protseq_id_elt_p_t elt, int number)
{
	int i;
	for (i=0; i<number; i++)	{
		RPC_DBG_PRINTF(rpc_es_dbg_general, 1, ("Register protseq 0x%0x %s\n", elt[i].rpc_protseq_id, elt[i].rpc_protseq));	
		memcpy(&rpc_g_protseq_id[elt[i].rpc_protseq_id],
				&elt[i],
				sizeof(rpc_protseq_id_elt_t));
	}
}

/* register a tower protocol id */
PRIVATE void rpc__register_tower_prot_id(rpc_tower_prot_ids_p_t tower_prot, int number)
{
	int i;
	for (i=0; i<number; i++) {
		rpc_tower_prot_ids_p_t tower = &tower_prot[i];
		
		RPC_DBG_PRINTF(rpc_es_dbg_general, 1,
				("Register tower protocol for %s\n",
				 	rpc_g_protseq_id[tower->rpc_protseq_id].rpc_protseq
				)
		);	

		memcpy(&rpc_g_tower_prot_ids[rpc_g_tower_prot_id_number],
				tower, sizeof(rpc_tower_prot_ids_t));

		rpc_g_tower_prot_id_number++;
	}
}

PRIVATE void rpc__register_protocol_id(rpc_protocol_id_elt_p_t prot, int number)
{
	int i;
	for (i=0; i<number; i++)	{
		RPC_DBG_PRINTF(rpc_es_dbg_general, 1,
				("Register protocol id 0x%x\n", prot[i].rpc_protocol_id));	


		memcpy(&rpc_g_protocol_id[prot[i].rpc_protocol_id],
				&prot[i],
				sizeof(rpc_protocol_id_elt_t));
	}
}

PRIVATE void rpc__register_naf_id(rpc_naf_id_elt_p_t naf, int number)
{
	int i;
	for (i=0; i < number; i++)	{
		RPC_DBG_PRINTF(rpc_es_dbg_general, 1,
				("Register network address family id 0x%x\n", naf[i].naf_id));	


		memcpy(&rpc_g_naf_id[naf[i].naf_id],
				&naf[i],
				sizeof(rpc_naf_id_elt_t));
	}
}

#ifdef HAVE_OS_WIN32
extern void rpc__ipnaf_module_init_func(void);
#endif

#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif

PRIVATE void rpc__load_modules(void)
{
#if HAVE_DLFCN_H
	struct dirent ** namelist;
	int i, n;
	void * image;
	char buf[PATH_MAX];
#endif

#ifdef HACK_DEBUG
	printf("in rpc__load_modules\n");
#endif
	memset(rpc_g_protseq_id, 0, sizeof(rpc_g_protseq_id));
	memset(rpc_g_naf_id, 0, sizeof(rpc_g_naf_id));
	memset(rpc_g_authn_protocol_id, 0, sizeof(rpc_g_authn_protocol_id));

#ifdef HAVE_OS_WIN32
	/* Fake loading ipnaf module */
	rpc__ipnaf_module_init_func();
#endif

	/* Fake loading no auth */
	rpc_g_authn_protocol_id[rpc_c_authn_none].authn_protocol_id = rpc_c_authn_none;
	rpc_g_authn_protocol_id[rpc_c_authn_none].dce_rpc_authn_protocol_id = dce_c_rpc_authn_protocol_none;
	
#if HAVE_DLFCN_H
	n = scandir(IMAGE_DIR, &namelist, select_module, sort_modules);
	for(i = 0; i < n; i++)	{
		
		sprintf(buf, "%s/%s", IMAGE_DIR, namelist[i]->d_name);

		RPC_DBG_PRINTF(rpc_es_dbg_general, 1, ("Loading module %s\n", buf));	
		
		image = dlopen(buf, RTLD_LAZY);
		if (image)	{
			void (*func)(void);
			
			func = dlsym(image, "rpc__module_init_func");
			if (func)	
				(*func)();
			else			
				dlclose(image);
		}
		else
			RPC_DBG_PRINTF(rpc_es_dbg_general, 1, ("failed to load module %s %s\n", buf, dlerror()));
	}
	free(namelist);
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
