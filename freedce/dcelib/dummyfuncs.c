#include <commonp.h>
#include <stdio.h>
#include <dce/rpc.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

/*
 * Dummy functions for DCE 1.1 RPC on Linux
 *
 */

#include <ctype.h>

static void print_asc(unsigned char  const *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		printf ("%c", isprint(buf[i]) ? buf[i] : '.');
	}
}

void print_data(const char *buf1, int len)
{
	unsigned char const *buf = (unsigned char const *)buf1;
	int i = 0;

	if (buf == NULL)
	{
		printf ("dump_data: NULL, len=%d\n", len);
		return;
	}
	if (len < 0)
		return;
	if (len == 0)
	{
		printf ("\n");
		return;
	}

	printf ("[%03X] ", i);
	for (i = 0; i < len;)
	{
		printf ("%02X ", (int)buf[i]);
		i++;
		if (i % 8 == 0)
			printf (" ");
		if (i % 16 == 0)
		{
			print_asc(&buf[i - 16], 8);
			printf (" ");
			print_asc(&buf[i - 8], 8);
			printf ("\n");
			if (i < len)
				printf ("[%03X] ", i);
		}
	}

	if (i % 16 != 0)	/* finish off a non-16-char-length row */
	{
		int n;

		n = 16 - (i % 16);
		printf (" ");
		if (n > 8)
			printf (" ");
		while (n--)
			printf ("   ");

		n = MIN(8, i % 16);
		print_asc(&buf[i - (i % 16)], n);
		printf (" ");
		n = (i % 16) - n;
		if (n > 0)
			print_asc(&buf[i - n], n);
		printf ("\n");
	}
}

static void dump_asc(int level, unsigned char  const *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level,
				("%c", isprint(buf[i]) ? buf[i] : '.'));
	}
}

void dump_data(int level, const char *buf1, int len)
{
	unsigned char const *buf = (unsigned char const *)buf1;
	int i = 0;

	if (buf == NULL)
	{
		RPC_DBG_PRINTF(rpc_e_dbg_auth, level, ("dump_data: NULL, len=%d\n", len));
		return;
	}
	if (len < 0)
		return;
	if (len == 0)
	{
		RPC_DBG_PRINTF(rpc_e_dbg_auth, level, ("\n"));
		return;
	}

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("[%03X] ", i));
	for (i = 0; i < len;)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("%02X ", (int)buf[i]));
		i++;
		if (i % 8 == 0)
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		if (i % 16 == 0)
		{
			dump_asc(level, &buf[i - 16], 8);
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
			dump_asc(level, &buf[i - 8], 8);
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("\n"));
			if (i < len)
				RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("[%03X] ", i));
		}
	}

	if (i % 16 != 0)	/* finish off a non-16-char-length row */
	{
		int n;

		n = 16 - (i % 16);
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		if (n > 8)
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		while (n--)
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("   "));

		n = MIN(8, i % 16);
		dump_asc(level, &buf[i - (i % 16)], n);
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		n = (i % 16) - n;
		if (n > 0)
			dump_asc(level, &buf[i - n], n);
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("\n"));
	}
}

void rpc_ns_binding_export(
	/* [in] */ unsigned32 entry_name_syntax __attribute__((__unused__)),
	/* [in] */ unsigned_char_p_t entry_name __attribute__((__unused__)),
	/* [in] */ rpc_if_handle_t if_spec __attribute__((__unused__)),
	/* [in] */ rpc_binding_vector_p_t binding_vector
	__attribute__((__unused__)),
	/* [in] */ uuid_vector_p_t object_uuid_vector __attribute__((__unused__)),
	/* [out] */ unsigned32 *status
)
{
	RPC_DBG_PRINTF(rpc_e_dbg_general, 1,
 ("rpc_ns_binding_export\n"));
	*status = rpc_s_name_service_unavailable;
}
 
typedef struct rpc_ns_ctx
{
	int ctx;
	unsigned32 expiration_age;
	rpc_if_handle_t ctx_if_spec;
	char *ctx_name;

} rpc_ns_ctx_t, *rpc_ns_ctx_p_t;

rpc_if_id_t netlogon_iface = 
{
  {0x12345678u, 0x1234, 0xabcd, 0xef, 0x00, {0x1, 0x23, 0x45, 0x67, 0xcf, 0xfb}},
  1, /* if major version */
  0 /* if minor version */
};

rpc_if_id_t lsarpc_iface = 
{
  {0x12345778u, 0x1234, 0xabcd, 0xef, 0x00, {0x1, 0x23, 0x45, 0x67, 0x89, 0xab}},
  0, /* if major version */
  0 /* if minor version */
};

void
rpc_ns_binding_import_begin(
    /* [in] */ unsigned32 entry_name_syntax __attribute__((__unused__)),
    /* [in] */ unsigned_char_p_t entry_name __attribute__((__unused__)),
    /* [in] */ rpc_if_handle_t if_spec ,
    /* [in] */ uuid_p_t object_uuid __attribute__((__unused__)),
    /* [out] */ rpc_ns_handle_t *import_context,
    /* [out] */ unsigned32 *status
)
{
	rpc_if_id_t if_id;
	rpc_ns_ctx_p_t ctx;

	RPC_DBG_PRINTF(rpc_e_dbg_general, 1,
			("rpc_ns_binding_import_begin\n"));
	dump_data(10, (char *)entry_name, 16);
	dump_data(10, (char *)if_spec, 32);
	dump_data(10, (char *)object_uuid, 16);

	RPC_MEM_ALLOC(ctx, rpc_ns_ctx_p_t,
			sizeof(rpc_ns_ctx_t),
			RPC_C_MEM_NS_INFO,
			RPC_C_MEM_WAITOK);

	*import_context = (rpc_ns_handle_t) ctx;

	ctx->ctx = 2;
	ctx->ctx_if_spec = if_spec;

	rpc_if_inq_id(if_spec, &if_id, status);

	if (uuid_equal(&if_id.uuid, &lsarpc_iface.uuid, status))
		ctx->ctx_name = "lsarpc";
	else if (uuid_equal(&if_id.uuid, &netlogon_iface.uuid, status))
		ctx->ctx_name = "netlogon";
	else
		*status = rpc_s_obj_uuid_not_found;
}

void
rpc_ns_binding_import_done(
    /* [in, out] */ rpc_ns_handle_t *import_context
	 __attribute__((__unused__)),
    /* [out] */ unsigned32 *status    
)
{
	RPC_DBG_PRINTF(rpc_e_dbg_general, 1,
		("rpc_ns_binding_import_done\n"));
	*status = rpc_s_ok;
	RPC_MEM_FREE(*import_context, RPC_C_MEM_NS_INFO);
}

void
rpc_ns_mgmt_handle_set_exp_age(
    /* [in] */ rpc_ns_handle_t ns_handle,
    /* [in] */ unsigned32 expiration_age,
    /* [out] */ unsigned32 *status

)
{
	rpc_ns_ctx_p_t ctx = (rpc_ns_ctx_p_t) ns_handle;
	RPC_DBG_PRINTF(rpc_e_dbg_general, 1,
		("rpc_ns_mgmt_handle_set_exp_age\n"));
	*status = rpc_s_ok;
	ctx->ctx = 2;
	ctx->expiration_age = expiration_age;
}

void
rpc_ns_binding_import_next(
    /* [in] */ rpc_ns_handle_t import_context,
    /* [out] */ rpc_binding_handle_t *binding,
    /* [out] */ unsigned32 *status
	       )
{
	rpc_ns_ctx_p_t ctx = (rpc_ns_ctx_p_t) import_context;
	RPC_DBG_PRINTF(rpc_e_dbg_general, 1,
		 ("rpc_ns_binding_import_next\n"));
	if (import_context == NULL)
	{
		*status = rpc_s_invalid_inquiry_context;
	}
	else if (ctx->ctx != 0)
	{
		char partial_binding[128];
		sprintf(partial_binding, "ncalrpc:.[%s]", ctx->ctx_name);
		rpc_binding_from_string_binding(
				(unsigned char *)partial_binding,
				  binding,
				  status);
		if ((*status) != rpc_s_ok)
		{
			RPC_DBG_PRINTF(rpc_e_dbg_general, 1,
				 ("rpc_ns_binding_import_next: string binding failed\n"));
			return;
		}
		rpc_ep_resolve_binding(*binding, ctx->ctx_if_spec, status);
		ctx->ctx--;
	}
	else
	{
		*status = rpc_s_no_more_bindings ;
	}
}

