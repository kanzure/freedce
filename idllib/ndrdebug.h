#ifdef DEBUG_INTERP

#ifndef __NDR_DEBUG__
#define __NDR_DEBUG__

#include <commonp.h>
#include <rpcdbg.h>

#define RPC_C_CN_DBG_VERBOSE_NDR 20

#define RPC_DBG_NDR(pargs) \
   RPC_DBG_PRINTF(rpc_es_dbg_verbose_ndr, RPC_C_CN_DBG_VERBOSE_NDR, pargs)

#define RPC_DBG_NDR_ADD(pargs) \
   RPC_DBG_ADD_PRINTF(rpc_es_dbg_verbose_ndr, RPC_C_CN_DBG_VERBOSE_NDR, pargs)

#define RPC_DBG_NDR_DATA(data, len) rpc_dbg_ndr_data(data, len);

char *rpc_dbg_ndr_struct_type(idl_byte struct_type);
char *rpc_dbg_ndr_type(idl_byte type_byte);
void rpc_dbg_ndr_data(void *type_byte, size_t len);


#else

#define RPC_DBG_NDR(pargs) do {;} while(0)
#define RPC_DBG_NDR_ADD(pargs) do {;} while(0)
#define RPC_DBG_NDR_DATA(data, len) do {;} while(0)

#endif
#endif
