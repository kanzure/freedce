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
**  NAME:
**
**      rpcdep.c
**
**  FACILITY:
**
**      RPC Daemon
**
**  ABSTRACT:
**
**  The DCE 1.0 Endpoint Map (ep_) Manager.
**
**
*/

#ifdef HAVE_OS_WIN32
typedef int pid_t;
#include <dce/dce_win32mem.h>
#endif

#include <commonp.h>
#include <com.h>
#include <dce/ep.h>

#include <rpcdp.h>
#include <rpcdepdb.h>

#ifdef RPC_LLB
#include <dce/glb.h>
#include <rpcdlbdb.h>
#endif



/* ====================================================================== */
/*
 * DCE 1.0 ept_ manager routines.
 */

GLOBAL ept_v3_0_epv_t ept_v3_0_mgr_epv = 
{
    ept_insert,
    ept_delete,
    ept_lookup,
    ept_map,
    ept_lookup_handle_free,
    ept_inq_object,
    ept_mgmt_delete
};

/*
 * If REMOTE_ENDPOINT_ACCESS is defined, this will preserve the
 * pre 1.0.3 semantics (and gaping security hole) which allows
 * an unauthenticated user to modify (or delete) the endpoint
 * map on a remote host.
 */

#ifndef REMOTE_ENDPOINT_ACCESS

INTERNAL int is_remote_handle( handle_t h, error_status_t *st )
{

    error_status_t status,status1;
    rpc_binding_vector_p_t bv;
    handle_t binding;
    unsigned_char_p_t stb,our_netaddr,client_netaddr;
    unsigned32 i;
    static unsigned_char_p_t *local_netaddr = NULL;
    static unsigned32 addr_count = 0;

/* Get client network address from binding handle (client_netaddr) */

    rpc_binding_server_from_client(h,&binding,&status);
    if (! STATUS_OK(&status))
    {
        *st = status;
        return(1);
    }
    rpc_binding_to_string_binding(binding,&stb,&status);

    if (! STATUS_OK(&status))
    {
        rpc_binding_free(&binding,&status1);
        *st = status;
        return(1);
    }
    rpc_binding_free(&binding,&status1);

    rpc_string_binding_parse(stb,NULL,NULL,&client_netaddr,NULL,NULL,&status);
    if (! STATUS_OK(&status))
    {
        rpc_string_free(&stb,&status1);
        *st = status;
        return(1);
    }
    rpc_string_free(&stb,&status1);

    /*
     * Lookup all of the addresses which this node answers to.
     * Cache these in static storage so we only do this work once.
     */
    if (addr_count == 0)
    {
        rpc_server_inq_bindings(&bv,&status);
        if (! STATUS_OK(&status))
        {
            rpc_string_free(&client_netaddr,&status1);
            *st = status;
            return(1);
        }

        addr_count = bv->count;
        local_netaddr = (unsigned_char_p_t *) sys_malloc( 
                        (size_t) (addr_count * sizeof(unsigned_char_p_t)));
        if (local_netaddr == NULL) 
        {
            rpc_string_free(&client_netaddr,&status1);
            rpc_binding_vector_free(&bv,&status1);
            *st = ept_s_no_memory;
            return(1);
        }

        for ( i=0; i < bv->count; i++ ) 
        {
            rpc_binding_to_string_binding(bv->binding_h[i],&stb,&status);
            if (! STATUS_OK(&status))
            {
                rpc_binding_vector_free(&bv,&status1);
                rpc_string_free(&client_netaddr,&status1);
                *st = status;
                return(1);
            }
            rpc_string_binding_parse(stb,NULL,NULL,
                                     &our_netaddr,NULL,NULL,&status);
            if (! STATUS_OK(&status))
            {
                rpc_binding_vector_free(&bv,&status1);
                rpc_string_free(&stb,&status1);
                rpc_string_free(&client_netaddr,&status1);
                *st = status;
                return(1);
            }

            local_netaddr[i] = our_netaddr;
            rpc_string_free(&stb,&status1);
        }
        rpc_binding_vector_free(&bv,&status1);
    }

    /*
     * Compare the addresses with the client address
     */
    *st = rpc_s_ok;
    for ( i=0; i < addr_count; i++ )
    {
        if(strcmp((char*)client_netaddr,(char*)local_netaddr[i]) == 0)
        {
            rpc_string_free(&client_netaddr,&status1);
            return(0);
        }
    }
    rpc_string_free(&client_netaddr,&status1);
    return(1);

}

#endif /* notdef REMOTE_ENDPOINT_ACCESS */

PRIVATE void ept_insert(h, num_ents, entries, replace, status)
handle_t            h;
unsigned32          num_ents;
ept_entry_t         entries[];
boolean32           replace;
error_status_t      *status;
{
    epdb_handle_t   epdb;
    ept_entry_t     *entp; 
    unsigned32             i;
    error_status_t  tmp_st;

    epdb_handle_from_ohandle(h, &epdb, status);
    if (! STATUS_OK(status))
        return;

#ifndef REMOTE_ENDPOINT_ACCESS
    if ( is_remote_handle(h,&tmp_st) )
    {
        *status = ept_s_cant_perform_op;
        return;
    }
#endif /* notdef REMOTE_ENDPOINT_ACCESS */

    for (i = 0, entp = &entries[0]; i < num_ents; i++, entp++)
    {
        epdb_insert(epdb, entp, replace, status);
        if (! STATUS_OK(status))
        {
            if (dflag) 
                show_st("ept_insert  Unable to update endpoint database", status);

            ept_delete(h, i, entries, &tmp_st);
            return;
        }
    }
}

PRIVATE void ept_delete(h, num_ents, entries, status)
handle_t            h;
unsigned32          num_ents;
ept_entry_t         entries[];
error_status_t      *status;

{
    epdb_handle_t   epdb;
    ept_entry_t     *entp; 
    unsigned32             i;
    error_status_t  tmp_st;

    epdb_handle_from_ohandle(h, &epdb, status);
    if (! STATUS_OK(status))
        return;

#ifndef REMOTE_ENDPOINT_ACCESS
    if ( is_remote_handle(h,&tmp_st) )
    {
        *status = ept_s_cant_perform_op;
        return;
    }
#endif /* notdef REMOTE_ENDPOINT_ACCESS */

    for (i = 0, entp = &entries[0]; i < num_ents; i++, entp++)
    {
        epdb_delete(epdb, entp, status);
        if (! STATUS_OK(status))
        {
            if (dflag) 
                show_st("ept_delete  Unable to update endpoint database", status);
            return;
        }
    }
}

PRIVATE void ept_lookup(h, inquiry_type, object, interface, vers_option, entry_handle,
                      max_ents, num_ents, entries, status)
handle_t            h;
unsigned32          inquiry_type;
uuid_p_t            object;
rpc_if_id_p_t       interface;
unsigned32          vers_option;
ept_lookup_handle_t *entry_handle;
unsigned32          max_ents;
unsigned32          *num_ents;
ept_entry_t         entries[];
error_status_t      *status;
{
    epdb_handle_t epdb;

    *num_ents = 0;

    epdb_handle_from_ohandle(h, &epdb, status);
    if (! STATUS_OK(status))
        return;

    epdb_lookup(epdb, inquiry_type, object, interface, vers_option, entry_handle,
               max_ents, num_ents, entries, status);

    if (dflag)
        printf("ept_lookup  entry_handle %p  *entry_handle %p  *num_ents %d\n", 
            entry_handle, *entry_handle, *num_ents);
}

PRIVATE void ept_map(h, object, map_tower, entry_handle,
                        max_towers, num_towers, towers, status)
handle_t            h;
uuid_p_t            object;
twr_p_t             map_tower;
ept_lookup_handle_t *entry_handle;
unsigned32          max_towers;
unsigned32          *num_towers;
twr_t               *towers[];
error_status_t      *status;
{
    epdb_handle_t epdb;

    *num_towers = 0;

    epdb_handle_from_ohandle(h, &epdb, status);
    if (! STATUS_OK(status))
        return;

    epdb_map(epdb, object, map_tower, entry_handle,
               max_towers, num_towers, towers, status);

#ifdef RPC_LLB
    if ((*status == ept_s_not_registered) ||
        (*status == ept_s_invalid_context) ||
    /*  
     * If finished with ept dbase, search llb dbase
     */
        ((*status == rpc_s_ok) && 
         ((*num_towers < max_towers) ||
          ((entry_handle != NULL) && (*entry_handle == NULL)) )) )
    {
        h = lbdb_inq_handle();
        lbdb_map(h, object, map_tower, entry_handle,
               max_towers, num_towers, towers, status);
                        
    }
#endif

    if (dflag)
        printf("ept_map  entry_handle %p  *entry_handle %p  *num_towers %d\n", 
            entry_handle, *entry_handle, *num_towers);
}

PRIVATE void ept_lookup_handle_free(h, entry_handle, status)
handle_t            h __attribute__((__unused__));
ept_lookup_handle_t *entry_handle;
error_status_t      *status;
{
    epdb_handle_t epdb;

    epdb = epdb_inq_handle();
    epdb_delete_lookup_handle(epdb, entry_handle);

    SET_STATUS_OK(status);
}

PRIVATE void ept_inq_object(h, object, status)
handle_t            h;
uuid_t              *object;
error_status_t      *status;
{
    epdb_handle_t epdb;

    epdb_handle_from_ohandle(h, &epdb, status);
    if (! STATUS_OK(status))
        return;

    epdb_inq_object(epdb, object, status);
}

PRIVATE void ept_mgmt_delete(h, object_speced, object, tower, status)
handle_t            h;
boolean32           object_speced;
uuid_p_t            object;
twr_p_t             tower;
error_status_t      *status;
{
    epdb_handle_t epdb;
    error_status_t tmp_st;

    epdb_handle_from_ohandle(h, &epdb, status);
    if (! STATUS_OK(status))
        return;

#ifndef REMOTE_ENDPOINT_ACCESS
    if ( is_remote_handle(h,&tmp_st) )
    {
        *status = ept_s_cant_perform_op;
        return;
    }
#endif /* notdef REMOTE_ENDPOINT_ACCESS */

    epdb_mgmt_delete(epdb, object_speced, object, tower, status);
}

PRIVATE void ept_lookup_handle_t_rundown(entry_handle)
ept_lookup_handle_t entry_handle;
{
    epdb_handle_t epdb;

    epdb = epdb_inq_handle();
    epdb_delete_lookup_handle(epdb,  &entry_handle);
}

