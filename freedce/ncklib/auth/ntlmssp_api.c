
/* 
 *  NTsecAPI NTLMSSP client/server auth/sign/seal API.
 *  Version 0.1
 *  sign/seal routines
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-2001
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ntlmssp_config.h"

#include <commonp.h>
#include <com.h>
#include <cn.h>
#include <dce/conv.h>
#include <ntlmsspauthcn.h>
#include <ntlmsspauth.h>
#include "includes.h"
#include "ntdomain.h"
#include "ntlmssp_api.h"
#include "rpc_ntlmssp.h"
#include "crc32.h"

int winbind_auth_chal_request(const char *user, const char *domain,
			char *chal, int chal_len,
			char *lm_pw, int lm_pw_len,
			char *nt_pw, int nt_pw_len,
			size_t *len, void **blob);


static void NTLMSSPcalc_p(ntlmssp_sec_state_p_t a, uchar * data, int len)
{
	uchar *hash = a->ntlmssp_hash;
	uchar index_i = hash[256];
	uchar index_j = hash[257];
	int ind;

	for (ind = 0; ind < len; ind++)
	{
		uchar tc;
		uchar t;

		index_i++;
		index_j += hash[index_i];

		tc = hash[index_i];
		hash[index_i] = hash[index_j];
		hash[index_j] = tc;

		t = hash[index_i] + hash[index_j];
		data[ind] = data[ind] ^ hash[t];
	}

	hash[256] = index_i;
	hash[257] = index_j;
}


/*******************************************************************
 * server sign&seal.
********************************************************************/
int ntlmssp_sign_seal(ntlmssp_sec_state_p_t sec_info, 
				char *data, size_t data_len,
				char *auth_data, size_t auth_data_len)
{
	int auth_verify;
	int auth_seal;
	prs_struct rverf;
	RPC_AUTH_NTLMSSP_CHK ntlmssp_chk;
	memset(&ntlmssp_chk, 0, sizeof(RPC_AUTH_NTLMSSP_CHK));

	uint32 crc32 = 0;

	auth_verify = IS_BITS_SET_ALL(sec_info->neg_flags,
					   NTLMSSP_NEGOTIATE_SIGN);
	auth_seal = IS_BITS_SET_ALL(sec_info->neg_flags,
					 NTLMSSP_NEGOTIATE_SEAL);

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 5, ("srv_ntlmssp_sign_seal: sign: %s seal: %s data %d\n",
		  BOOLSTR(auth_verify), BOOLSTR(auth_seal), data_len));

	prs_create(&rverf, auth_data, auth_data_len, 4, MARSHALL);

	if (auth_verify)
	{
		crc32 = crc32_calc_buffer(data_len, data);
		sec_info->ntlmssp_seq_num++;
	}
	if (auth_seal)
	{
		dump_data(20, data, data_len);
		NTLMSSPcalc_p(sec_info, (uchar *) data, data_len);
		dump_data(20, data, data_len);
	}

	make_rpc_auth_ntlmssp_chk(&ntlmssp_chk,
				  NTLMSSP_SIGN_VERSION, crc32,
				  sec_info->ntlmssp_seq_num);
	dump_data(20, auth_data, auth_data_len);
	smb_io_rpc_auth_ntlmssp_chk("auth_sign", &ntlmssp_chk, &rverf, 0);

	if (auth_verify)
	{
		NTLMSSPcalc_p(sec_info, (uchar *) prs_data(&rverf, 4), 12);
		dump_data(20, auth_data, auth_data_len);
	}

	return True;
}

static void ntlmssp_sec_state_init(ntlmssp_sec_state_p_t sec_info,
				const char *lm_hash,
				const uchar *lm_owf,
				int mode_40bit)
{
	uchar p24[24];
	uchar j = 0;
	int ind;
	uchar password[16];
	uchar k2[16];
	int len;

	ZERO_STRUCT(password);
	memcpy(password, lm_hash, 8);

	NTLMSSPOWFencrypt(password, lm_owf, p24);

	if (mode_40bit)
	{
		len = 8;
		memcpy(k2, p24, 5);
		k2[5] = 0xe5;
		k2[6] = 0x38;
		k2[7] = 0xb0;
	}
	else
	{
		len = 16;
		memcpy(k2, p24, 16);
	}

	for (ind = 0; ind < 256; ind++)
	{
		sec_info->ntlmssp_hash[ind] = (uchar) ind;
	}

	for (ind = 0; ind < 256; ind++)
	{
		uchar tc;

		j += (sec_info->ntlmssp_hash[ind] + k2[ind % len]);
		tc = sec_info->ntlmssp_hash[ind];
		sec_info->ntlmssp_hash[ind] = sec_info->ntlmssp_hash[j];
		sec_info->ntlmssp_hash[j] = tc;
	}

	sec_info->ntlmssp_hash[256] = 0;
	sec_info->ntlmssp_hash[257] = 0;
	sec_info->ntlmssp_seq_num = 0;
}

static int srv_ntlmssp_verify(ntlmssp_sec_state_p_t sec_info, prs_struct *data_i)
{
	uchar lm_owf[24];
	uchar nt_owf[128];
	NET_USER_INFO_3 info3;
	size_t lm_owf_len;
	size_t nt_owf_len;
	size_t usr_len;
	size_t dom_len;
	size_t wks_len;
	int auth_validated = True;
	int unicode = False;

	fstring user_name;
	fstring domain;
	fstring wks;
	size_t blob_len = 0;
	void *blob = NULL;

	RPC_AUTH_NTLMSSP_RESP ntlmssp_resp;

	smb_io_rpc_auth_ntlmssp_resp("", &ntlmssp_resp, data_i, 0);

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 5, ("srv_ntlmssp_verify: checking user details\n"));

	lm_owf_len = ntlmssp_resp.hdr_lm_resp.str_str_len;
	nt_owf_len = ntlmssp_resp.hdr_nt_resp.str_str_len;
	usr_len = ntlmssp_resp.hdr_usr.str_str_len;
	dom_len = ntlmssp_resp.hdr_domain.str_str_len;
	wks_len = ntlmssp_resp.hdr_wks.str_str_len;

	if (lm_owf_len > sizeof(lm_owf))
		return False;
	if (nt_owf_len > sizeof(nt_owf))
		return False;

	memcpy(lm_owf, ntlmssp_resp.lm_resp, sizeof(lm_owf));
	memcpy(nt_owf, ntlmssp_resp.nt_resp, sizeof(nt_owf));

	dump_data_pw("lm_owf:", lm_owf, sizeof(lm_owf));
	dump_data_pw("nt_owf:", nt_owf, sizeof(nt_owf));
	dump_data_pw("chal:", sec_info->ntlmssp_chal.challenge, 8);

	unicode = IS_BITS_SET_ALL
	    (sec_info->neg_flags, NTLMSSP_NEGOTIATE_UNICODE);

	if (unicode)
	{
		unibuf_to_ascii(user_name, ntlmssp_resp.user,
				MIN((size_t)(ntlmssp_resp.hdr_usr.str_str_len / 2),
				    (size_t)(sizeof(user_name) - 1)));
		unibuf_to_ascii(domain, ntlmssp_resp.domain,
				MIN((size_t)(ntlmssp_resp.hdr_domain.str_str_len / 2),
				    (size_t)(sizeof(domain) - 1)));
		unibuf_to_ascii(wks, ntlmssp_resp.wks,
				MIN((size_t)(ntlmssp_resp.hdr_wks.str_str_len / 2),
				    (size_t)(sizeof(wks) - 1)));
	}
	else
	{
		fstrcpy(user_name, ntlmssp_resp.user);
		fstrcpy(domain, ntlmssp_resp.domain);
		fstrcpy(wks, ntlmssp_resp.wks);
	}

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 3, ("NTLMSSP %s %s\n",
				  user_name, domain));
	dump_data(3, sec_info->ntlmssp_chal.challenge, 8);
	dump_data(3, lm_owf, lm_owf_len);
	dump_data(3, nt_owf, nt_owf_len);
	auth_validated = winbind_auth_chal_request(user_name, domain,
						  (char *)
						  sec_info->ntlmssp_chal.challenge, 8,
						  lm_owf, lm_owf_len,
						  nt_owf, nt_owf_len,
						  &blob_len, &blob) == 0x0;
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 3, ("status: %d\n", auth_validated));
	dump_data(3, blob, blob_len);

	if (blob == NULL)
		auth_validated = False;

	if (auth_validated)
	{
		/************************************************************/
		/****************** lkclXXXX - NTLMv1 ONLY! *****************/
		/************************************************************/

		prs_struct data_i;
		prs_create(&data_i, blob, blob_len, 4, UNMARSHALL);

		auth_validated = net_io_user_info3("usr", &info3, &data_i, 0);
		free(blob);
	}

	if (auth_validated)
	{
		ntlmssp_sec_state_init(sec_info, info3.padding, lm_owf, True);
	}

	return auth_validated;
}

static int srv_ntlmssp(ntlmssp_sec_state_p_t sec_info, prs_struct *data_i,
		uint32 msg_type)
{
	/* receive a negotiate; send a challenge; receive a response */
	switch (msg_type)
	{
		case NTLMSSP_NEGOTIATE:
		{
			RPC_AUTH_NTLMSSP_NEG ntlmssp_neg;
			if (!smb_io_rpc_auth_ntlmssp_neg("", &ntlmssp_neg, data_i, 0))
			{
				return False;
			}
			/*
			if (strlen(ntlmssp_neg.myname) == 0 ||
			    strlen(ntlmssp_neg.domain) == 0)
			{
				return False;
			}
			*/
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 10, ("ntlmssp neg: myname %s domain %s neg_flags %lx\n",
				   ntlmssp_neg.myname, ntlmssp_neg.domain,
				   ntlmssp_neg.neg_flgs));
			sec_info->neg_flags = ntlmssp_neg.neg_flgs;
			break;
		}
		case NTLMSSP_AUTH:
		{
			if (!srv_ntlmssp_verify(sec_info, data_i))
			{
				data_i->offset = 0;
			}
			break;
		}
		default:
		{
			/* NTLMSSP expected: unexpected message type */
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 3, ("unexpected message type in NTLMSSP %d\n",
				  msg_type));
			return False;
		}
	}

	return data_i->offset != 0;
}

int ntlmssp_bind_auth_resp(ntlmssp_sec_state_p_t sec_info,
				char *data, size_t data_len)
{
	RPC_AUTH_VERIFIER auth_verifier;
	prs_struct data_i;

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 5, ("ntlmssp_bind_auth_resp: decode request.\n"));

	prs_create(&data_i, data, data_len, 4, UNMARSHALL);

	/* decode the authentication verifier response */
	smb_io_rpc_auth_verifier("", &auth_verifier, &data_i, 0);
	if (&data_i.offset == 0)
		return False;

	if (!rpc_auth_verifier_chk(&auth_verifier, "NTLMSSP", NTLMSSP_AUTH))
	{
		return False;
	}

	return srv_ntlmssp(sec_info, &data_i, auth_verifier.msg_type);
}

int srv_ntlmssp_auth_verify(ntlmssp_sec_state_p_t sec_info,
		char *data, size_t data_len)
{
	RPC_AUTH_VERIFIER auth_verifier;
	prs_struct data_i;

	prs_create(&data_i, data, data_len, 4, UNMARSHALL);

	if (!smb_io_rpc_auth_verifier("", &auth_verifier,
					  &data_i, 0))
	{
		return False;
	}
	if (data_i.offset == 0)
		return False;

	if (strcmp(auth_verifier.signature, "NTLMSSP") == 0)
	{
		return srv_ntlmssp(sec_info, &data_i, auth_verifier.msg_type);
	}
	return False;
}

int ntlmssp_auth_gen(ntlmssp_sec_state_p_t sec_info, 
			char *auth_info, size_t *auth_info_len)
{
	prs_struct rauth;
	uint8 challenge[8];
	RPC_AUTH_VERIFIER auth_verifier;

	prs_create(&rauth, auth_info, *auth_info_len, 4, MARSHALL);

	memset(challenge, 0xfe, 8);
	/*
	generate_random_buffer(challenge, 8, False);
	*/

	/*** NTLMSSP verifier ***/

	make_rpc_auth_verifier(&auth_verifier, "NTLMSSP", NTLMSSP_CHALLENGE);
	smb_io_rpc_auth_verifier("", &auth_verifier, &rauth, 0);

	/* NTLMSSP challenge ** */

	sec_info->neg_flags &= 0x000082b1;

	make_rpc_auth_ntlmssp_chal(&sec_info->ntlmssp_chal, sec_info->neg_flags, challenge);
	smb_io_rpc_auth_ntlmssp_chal("", &sec_info->ntlmssp_chal, &rauth, 0);

	*auth_info_len = rauth.offset;

	return True;
}

/****************************************************************************
 decrypt data on an rpc pipe
 ****************************************************************************/
int ntlmssp_unsign_unseal(ntlmssp_sec_state_p_t sec_info,
				char *data, size_t data_len,
				char *auth_data, size_t auth_len)
{
	int auth_verify;
	int auth_seal  ;

	auth_verify = IS_BITS_SET_ALL(sec_info->neg_flags,
	                              NTLMSSP_NEGOTIATE_SIGN);
	auth_seal   = IS_BITS_SET_ALL(sec_info->neg_flags,
	                              NTLMSSP_NEGOTIATE_SEAL);

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 5,("decode_ntlmssp_pdu: len: %d auth_len: %d verify %s seal %s\n",
	          data_len, auth_len, BOOLSTR(auth_verify), BOOLSTR(auth_seal)));

	if ((auth_verify || auth_seal) && auth_len != 16)
	{
		return False;
	}
	
	if (auth_seal)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 10,("decode_ntlmssp_pdu: seal\n"));
		dump_data(20, data, data_len);
		NTLMSSPcalc_p(sec_info, (uchar*)data, data_len);
		dump_data(20, data, data_len);
	}

	if (auth_verify)
	{
		RPC_AUTH_NTLMSSP_CHK chk;
		uint32 crc32;

		prs_struct auth_verf;

		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 10,("decode_ntlmssp_pdu: verify\n"));
		dump_data(20, auth_data, auth_len);
		NTLMSSPcalc_p(sec_info, (uchar*)(auth_data+4), auth_len - 4);
		prs_create(&auth_verf, auth_data, 16, 4, UNMARSHALL);
		smb_io_rpc_auth_ntlmssp_chk("auth_sign", &chk, &auth_verf, 0);
		dump_data(20, auth_data, auth_len);

		crc32 = crc32_calc_buffer(data_len, data);
		if (!rpc_auth_ntlmssp_chk(&chk, crc32 , sec_info->ntlmssp_seq_num))
		{
			return False;
		}
		sec_info->ntlmssp_seq_num++;
	}
	return True;
}

/*******************************************************************
 creates a DCE/RPC bind request

 - initialises the parse structure.
 - dynamically allocates the header data structure
 - caller is expected to free the header data structure once used.

 ********************************************************************/
int ntlmssp_create_bind_req(ntlmssp_sec_state_p_t sec_info,
				unsigned char *server_princ_name,
				struct ntuser_creds *usr,
				char *auth_req_data, size_t *auth_req_len)
{
	prs_struct auth_req;

	RPC_AUTH_VERIFIER auth_verifier;
	RPC_AUTH_NTLMSSP_NEG ntlmssp_neg;

	if (usr == NULL || sec_info == NULL)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 10,("create_ntlmssp_bind_req: NULL user creds\n"));
		return False;
	}

	prs_create(&auth_req, auth_req_data, *auth_req_len, 4, MARSHALL);

	make_rpc_auth_verifier(&auth_verifier, "NTLMSSP", NTLMSSP_NEGOTIATE);

	smb_io_rpc_auth_verifier("auth_verifier", &auth_verifier, &auth_req, 0);

	make_rpc_auth_ntlmssp_neg(&ntlmssp_neg,
			       usr->ntlmssp_flags, server_princ_name,
				   usr->domain);

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 10,("create_ntlmssp_bind_req: neg_flags: 0x%x\n", ntlmssp_neg.neg_flgs));
	smb_io_rpc_auth_ntlmssp_neg("ntlmssp_neg", &ntlmssp_neg, &auth_req, 0);

	*auth_req_len = auth_req.offset;

	return True;
}

int ntlmssp_decode_bind_resp(ntlmssp_sec_state_p_t sec_info,
				char *auth_data, size_t auth_data_len)
{
	int valid_ack = True;
	prs_struct rdata;

	prs_create(&rdata, auth_data, auth_data_len, 4, UNMARSHALL);

	if (valid_ack)
	{
		RPC_AUTH_VERIFIER rhdr_verf;
		smb_io_rpc_auth_verifier("", &rhdr_verf, &rdata, 0);
		if (rdata.offset == 0 ||
		    !rpc_auth_verifier_chk(&rhdr_verf,
		                                   "NTLMSSP",
		                                    NTLMSSP_CHALLENGE))
		{
			valid_ack = False;
		}
	}
	if (valid_ack)
	{
		smb_io_rpc_auth_ntlmssp_chal("", &sec_info->ntlmssp_chal, &rdata, 0);
		sec_info->neg_flags = sec_info->ntlmssp_chal.neg_flags;
		if (rdata.offset == 0)
			valid_ack = False;
	}
	return valid_ack;
}

/*******************************************************************
 creates a DCE/RPC bind authentication response

 - initialises the parse structure.
 - dynamically allocates the header data structure
 - caller is expected to free the header data structure once used.

 ********************************************************************/
static int create_ntlmssp_rpc_bind_resp(struct pwd_info *pwd,
				char *domain, char *user_name, char *my_name,
				uint32 ntlmssp_cli_flgs,
                prs_struct *auth_resp)
{
	RPC_AUTH_VERIFIER auth_verifier;
	uchar lm_owf[24];
	uchar nt_owf[128];
	size_t nt_owf_len;

	make_rpc_auth_verifier(&auth_verifier,
			       "NTLMSSP", NTLMSSP_AUTH);

	smb_io_rpc_auth_verifier("auth_verifier", &auth_verifier, auth_resp, 0);

	pwd_get_lm_nt_owf(pwd, lm_owf, nt_owf, &nt_owf_len);

	create_ntlmssp_resp(lm_owf, nt_owf, nt_owf_len,
			domain, user_name, my_name, ntlmssp_cli_flgs,
                                auth_resp);

	return True;
}

/*******************************************************************
 creates a DCE/RPC bind continue request

 - initialises the parse structure.
 - dynamically allocates the header data structure
 - caller is expected to free the header data structure once used.

 ********************************************************************/
int ntlmssp_create_bind_cont(ntlmssp_sec_state_p_t sec_info,
				unsigned char *server_princ_name,
				struct ntuser_creds *usr,
				char *auth_resp_data, size_t *auth_resp_len)
{
	unsigned char lm_owf[24];
	unsigned char lm_hash[16];
	prs_struct auth_resp;

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 5,("Bind RPC Cont\n"));

	if (sec_info == NULL || usr == NULL)
	{
		return False;
	}

	prs_create(&auth_resp, auth_resp_data, *auth_resp_len, 4, MARSHALL);

	/* hmmm... */
	if (usr->pwd.cleartext)
	{
		pwd_make_lm_nt_16(&usr->pwd, usr->pwd.password);
	}
	pwd_make_lm_nt_owf(&usr->pwd, sec_info->ntlmssp_chal.challenge, sec_info->usr_sess_key);

	create_ntlmssp_rpc_bind_resp(&usr->pwd, usr->domain,
			     usr->user_name, server_princ_name,
			     sec_info->neg_flags,
			     &auth_resp);
			    
	pwd_get_lm_nt_owf(&usr->pwd, lm_owf, NULL, NULL);
	pwd_get_lm_nt_16(&usr->pwd, lm_hash, NULL);
	ntlmssp_sec_state_init(sec_info, lm_hash, lm_owf, True);
	ZERO_STRUCT(lm_hash);

	*auth_resp_len = auth_resp.offset;

	return True;
}

