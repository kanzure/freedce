#ifndef NTLMSSP_API_H
#define NTLMSSP_API_H

#include "rpc_ntlmssp.h"

/* ntlmssp state info */

#define NTLMSSP_MALLOC_FN(fn) \
	void* (*fn)(size_t)

#define NTLMSSP_FREE_FN(fn) \
	void (*fn)(void*)

typedef struct _ntlmssp_sec_state
{
	NTLMSSP_MALLOC_FN(malloc); /* need memory? use this */
	NTLMSSP_FREE_FN  (free); /* need to free it? use this */

	RPC_AUTH_NTLMSSP_CHAL ntlmssp_chal;
	uint8 usr_sess_key[16];

	unsigned char ntlmssp_hash[258];
	uint32 ntlmssp_seq_num;

} *ntlmssp_sec_state_p_t, ntlmssp_sec_state_t;

/* server-side */

int ntlmssp_auth_gen(ntlmssp_sec_state_p_t l,
			char *auth_info, size_t *auth_info_len);

int ntlmssp_bind_auth_resp(ntlmssp_sec_state_p_t l,
				char *data, size_t data_len);

/* client _and_ server-side */

int ntlmssp_auth_init(ntlmssp_sec_state_p_t l,
				unsigned char **server_princ_name);

int ntlmssp_sign_seal(ntlmssp_sec_state_p_t l, 
				char *data, size_t data_len,
				char *auth_data, size_t auth_data_len);

int ntlmssp_unsign_unseal(ntlmssp_sec_state_p_t l,
				char *data, size_t data_len,
				char *auth_data, size_t auth_len);

/* client-side */

int ntlmssp_create_bind_req(ntlmssp_sec_state_p_t l,
				unsigned char *server_princ_name,
				struct ntuser_creds *usr,
				char *auth_req_data, size_t *auth_req_len);

int ntlmssp_create_bind_cont(ntlmssp_sec_state_p_t l,
				unsigned char *server_princ_name,
				struct ntuser_creds *usr,
				char *auth_resp_data, size_t *auth_resp_len);

int ntlmssp_decode_bind_resp(ntlmssp_sec_state_p_t l,
				char *auth_data, size_t auth_data_len);

#endif
