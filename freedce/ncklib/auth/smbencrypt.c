/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   Copyright (C) Andrew Tridgell 1992-2000
   Copyright (C) Jeremy Allison 1995-2000.
   Copyright (C) Luke Kennethc Casson Leighton 1996-2001.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <commonp.h>
#include "includes.h"
#include <ctype.h>


/* Does the des encryption from the NT or LM MD4 hash. */
void SMBOWFencrypt(const uchar pwrd[16], const uchar * c8, uchar p24[24])
{
	uchar p21[21];

	ZERO_STRUCT(p21);

	memcpy(p21, pwrd, 16);
	E_P24(p21, c8, p24);
}

/* Does the des encryption from the FIRST 8 BYTES of the NT or LM MD4 hash. */
void NTLMSSPOWFencrypt(const uchar pwrd[8], const uchar * ntlmchalresp,
		       uchar p24[24])
{
	uchar p21[21];

	ZERO_STRUCT(p21);
	memcpy(p21, pwrd, 8);
	memset(p21 + 8, 0xbd, 8);

	E_P24(p21, ntlmchalresp, p24);
#ifdef DEBUG_PASSWORD
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("NTLMSSPOWFencrypt: p21, c8, p24\n"));
	dump_data(100, p21, 21);
	dump_data(100, ntlmchalresp, 8);
	dump_data(100, p24, 24);
#endif
}


BOOL make_oem_passwd_hash(uchar data[516],
			  const char *pwrd, int new_pw_len,
			  const uchar old_pw_hash[16])
{
	if (new_pw_len > 512)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1,
		      ("make_oem_passwd_hash: new password is too long.\n"));
		return False;
	}

	/*
	 * Now setup the data area.
	 * We need to generate a random fill
	 * for this area to make it harder to
	 * decrypt. JRA.
	 */
	/*
	generate_random_buffer(data, 516, False);
	*/
	fstrcpy(&data[512 - new_pw_len], pwrd);
	SIVAL(data, 512, new_pw_len);

#ifdef DEBUG_PASSWORD
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("make_oem_passwd_hash\n"));
	dump_data(100, data, 516);
#endif
	if (old_pw_hash != NULL)
	{
		SamOEMhash(data, old_pw_hash, True);
	}

	return True;
}

void SMBOWFencrypt_ntv2(const uchar kr[16],
			const uchar * srv_chal, int srv_chal_len,
			const uchar * cli_chal, int cli_chal_len,
			char resp_buf[16])
{
	HMACMD5Context ctx;

	hmac_md5_init_limK_to_64(kr, 16, &ctx);
	hmac_md5_update(srv_chal, srv_chal_len, &ctx);
	hmac_md5_update(cli_chal, cli_chal_len, &ctx);
	hmac_md5_final(resp_buf, &ctx);

#ifdef DEBUG_PASSWORD
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("SMBOWFencrypt_ntv2: srv_chal, cli_chal, resp_buf\n"));
	dump_data(100, srv_chal, srv_chal_len);
	dump_data(100, cli_chal, cli_chal_len);
	dump_data(100, resp_buf, 16);
#endif
}

void SMBsesskeygen_ntv2(const uchar kr[16],
			const uchar * nt_resp, char sess_key[16])
{
	HMACMD5Context ctx;

	hmac_md5_init_limK_to_64(kr, 16, &ctx);
	hmac_md5_update(nt_resp, 16, &ctx);
	hmac_md5_final(sess_key, &ctx);

#ifdef DEBUG_PASSWORD
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("SMBsesskeygen_ntv2:\n"));
	dump_data(100, sess_key, 16);
#endif
}

void SMBsesskeygen_ntv1(const uchar kr[16],
			const uchar * nt_resp, char sess_key[16])
{
	mdfour(sess_key, kr, 16);

	nt_resp = NULL; /* unused param: stop warning being issued */

#ifdef DEBUG_PASSWORD
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("SMBsesskeygen_ntv1:\n"));
	dump_data(100, sess_key, 16);
#endif
}

#if 0
/***************************************************************************
 tests showed that the nt challenge can be total random-length garbage!
 ***************************************************************************/
void SMBgenclientchals(char *lm_cli_chal,
		       char *nt_cli_chal, int *nt_cli_chal_len,
		       const char *srv, const char *dom)
{
	NTTIME nt_time;
	int srv_len = strlen(srv);
	int dom_len = strlen(dom);
	fstring server;
	fstring domain;
	fstrcpy(server, srv);
	fstrcpy(domain, dom);
	strupper(server);
	strupper(domain);

#ifdef EXPERIMENTATION_THIS_ACTUALLY_WORKS
	generate_random_buffer(nt_cli_chal, 64, False);
	(*nt_cli_chal_len) = 64;
	memcpy(lm_cli_chal, nt_cli_chal + 16, 8);
	generate_random_buffer(lm_cli_chal, 8, False);

	return;
#endif

	generate_random_buffer(lm_cli_chal, 8, False);
	unix_to_nt_time(&nt_time, time(NULL));

	CVAL(nt_cli_chal, 0) = 0x1;
	CVAL(nt_cli_chal, 1) = 0x1;
	SSVAL(nt_cli_chal, 2, 0x0);
	SIVAL(nt_cli_chal, 4, 0x0);
	SIVAL(nt_cli_chal, 8, nt_time.low);
	SIVAL(nt_cli_chal, 12, nt_time.high);
	memcpy(nt_cli_chal + 16, lm_cli_chal, 8);
	/* fill in offset 24, size of structure, later */

	*nt_cli_chal_len = 28;

	SSVAL(nt_cli_chal, *nt_cli_chal_len, 2);
	*nt_cli_chal_len += 2;
	SSVAL(nt_cli_chal, *nt_cli_chal_len, dom_len * 2);
	*nt_cli_chal_len += 2;
	ascii_to_unibuf(nt_cli_chal + (*nt_cli_chal_len), domain,
			dom_len * 2);
	*nt_cli_chal_len += dom_len * 2;
	*nt_cli_chal_len += 4 - ((*nt_cli_chal_len) % 4);

	SSVAL(nt_cli_chal, *nt_cli_chal_len, 2);
	*nt_cli_chal_len += 2;
	SSVAL(nt_cli_chal, 30, srv_len * 2);
	*nt_cli_chal_len += 2;
	ascii_to_unibuf(nt_cli_chal + (*nt_cli_chal_len), server,
			srv_len * 2);
	*nt_cli_chal_len += srv_len * 2;

	SSVAL(nt_cli_chal, 24, (*nt_cli_chal_len) + 16);
	SSVAL(nt_cli_chal, 26, (*nt_cli_chal_len) + 15);

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("SMBgenclientchals: srv %s, dom %s\n", server, domain));
	dump_data(100, nt_cli_chal, *nt_cli_chal_len);
}

/* the user_n and domain_n is Unicode16 but it must be
 * upper-case and in intel-byte-order! user_l and domain_l
 * must be length, in chars (not num unicode16 chars) of
 * string.  NOT including NULL-terminating unicode16 char.
 */
void ntv2_owf_genW(const uchar owf[16],
		  const char *user_n, int user_l,
		  const char *domain_n, int domain_l,
		  uchar kr_buf[16])
{
	HMACMD5Context ctx;

	hmac_md5_init_limK_to_64(owf, 16, &ctx);
	hmac_md5_update(user_u, user_l, &ctx);
	hmac_md5_update(dom_u, domain_l, &ctx);
	hmac_md5_final(kr_buf, &ctx);

#ifdef DEBUG_PASSWORD
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("ntv2_owf_gen: user, domain, owfkey, kr\n"));
	dump_data(100, user_n, user_l);
	dump_data(100, dom_n, domain_l);
	dump_data(100, owf, 16);
	dump_data(100, kr_buf, 16);
#endif
}
#endif

/* Does the LM owf of a user's password */
void lm_owf_genW(const UNISTR2 *pwd, uchar p16[16])
{
	char pwrd[15];

	ZERO_STRUCT(pwrd);
	if (pwd != NULL)
	{
		uint32 i;
		for (i = 0; i < pwd->uni_str_len && i < sizeof(pwrd); i++)
		{
			pwrd[i] = toupper(pwd->buffer[i]);
		}
	}

	/* Mangle the passwords into Lanman format */
	pwrd[14] = '\0';

	/* Calculate the SMB (lanman) hash functions of the password */

	E_P16((uchar *) pwrd, (uchar *) p16);

#ifdef DEBUG_PASSWORD
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("lm_owf_genW: pwd, lm#\n"));
	dump_data(120, pwrd, strlen(pwrd));
	dump_data(100, p16, 16);
#endif
	/* clear out local copy of user's password (just being paranoid). */
	ZERO_STRUCT(pwrd);
}

/* Does both the NT and LM owfs of a user's password */
void nt_owf_genW(const UNISTR2 *pwd, uchar nt_p16[16])
{
	char buf[512];
	unsigned int i;

	for (i = 0; i < MIN((size_t)(pwd->uni_str_len), sizeof(buf) / 2); i++)
	{
		SIVAL(buf, i * 2, pwd->buffer[i]);
	}
	/* Calculate the MD4 hash (NT compatible) of the password */
	mdfour(nt_p16, buf, pwd->uni_str_len * 2);

	dump_data_pw("nt_owf_genW:", buf, pwd->uni_str_len * 2);
	dump_data_pw("nt#:", nt_p16, 16);

	/* clear out local copy of user's password (just being paranoid). */
	ZERO_STRUCT(buf);
}

/* Does both the NT and LM owfs of a user's UNICODE password */
void nt_lm_owf_genW(const UNISTR2 *pwd, uchar nt_p16[16], uchar lm_p16[16])
{
	nt_owf_genW(pwd, nt_p16);
	lm_owf_genW(pwd, lm_p16);
}

BOOL nt_encrypt_string2(STRING2 * out, const STRING2 * in, const uchar * key)
{
	const uchar *keyptr = key;
	const uchar *keyend = key + 16;
	int datalen = in->str_str_len;

	uchar *outbuf = (uchar *) out->buffer;
	const uchar *inbuf = (const uchar *)in->buffer;
	const uchar *inbufend;

	out->str_max_len = in->str_max_len;
	out->str_str_len = in->str_str_len;
	out->undoc = 0;

	inbufend = inbuf + datalen;

	dump_data_pw("nt_encrypt_string2\n", inbuf, datalen);

	while (inbuf < inbufend)
	{
		smbhash(outbuf, inbuf, keyptr, 1);

		keyptr += 7;
		if (keyptr + 7 > keyend)
		{
			keyptr = (keyend - keyptr) + key;
		}

		inbuf += 8;
		outbuf += 8;
	}

	dump_data_pw("nt_encrypt_string2\n", out->buffer, datalen);

	return True;
}

BOOL nt_decrypt_string2(STRING2 * out, const STRING2 * in, const uchar * key)
{
	unsigned int datalen = in->str_str_len;

	const uchar *keyptr = key;
	const uchar *keyend = key + 16;

	uchar *outbuf = (uchar *) out->buffer;
	const uchar *inbuf = (const uchar *)in->buffer;
	const uchar *inbufend;

	if (in->str_str_len > MAX_STRINGLEN)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1, ("nt_decrypt_string2: failed\n"));
		return False;
	}

	out->str_max_len = in->str_max_len;
	out->str_str_len = in->str_str_len;
	out->undoc = in->undoc;

	inbufend = inbuf + datalen;

	while (inbuf < inbufend)
	{
		smbhash(outbuf, inbuf, keyptr, 0);
		keyptr += 7;
		if (keyptr + 7 > keyend)
		{
			keyptr = (keyend - keyptr) + key;
		}

		inbuf += 8;
		outbuf += 8;
	}

	datalen = IVAL(out->buffer, 0);

	dump_data_pw("nt_decrypt_string2\n", out->buffer, out->str_str_len);

	if (datalen != in->str_str_len - 8)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 2, ("nt_decrypt_string2: length-match failed\n"));
		return False;
	}

	return True;
}

/***********************************************************
 decode a password buffer
************************************************************/
BOOL decode_pw_buffer(const char buffer[516], char *new_pwrd,
		      int new_pwrd_size, uint32 *new_pw_len)
{
	/* 
	 * The length of the new password is in the last 4 bytes of
	 * the data buffer.
	 */

	(*new_pw_len) = IVAL(buffer, 512);

#ifdef DEBUG_PASSWORD
	dump_data(100, buffer, 516);
#endif

	if ((*new_pw_len) > (uint32)(new_pwrd_size - 1))
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1,
		      ("decode_pw_buffer: incorrect password length (%d).\n",
		       (*new_pw_len)));
		return False;
	}

	memcpy(new_pwrd, &buffer[512 - (*new_pw_len)], (*new_pw_len));
	new_pwrd[(*new_pw_len)] = '\0';

#ifdef DEBUG_PASSWORD
	dump_data(100, new_pwrd, (*new_pw_len));
#endif

	return True;
}

/***********************************************************
 encode a password buffer
************************************************************/
BOOL encode_pw_buffer(char buffer[516], const char *new_pass,
		      int new_pw_len)
{
	/*
	generate_random_buffer(buffer, 516, True);
	*/

	memcpy(&buffer[512 - new_pw_len], new_pass, new_pw_len);

	/* 
	 * The length of the new password is in the last 4 bytes of
	 * the data buffer.
	 */

	SIVAL(buffer, 512, new_pw_len);

#ifdef DEBUG_PASSWORD
	dump_data(100, buffer, 516);
#endif

	return True;
}
