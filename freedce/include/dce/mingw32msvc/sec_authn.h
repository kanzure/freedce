/* (c) Copyright 2001 Luke Kenneth Casson Leighton
 * NTLMSSP Authentication Implementation (see ISBN 1578701503)
 */

#ifndef NTLMSSP_AUTH_CREDS
#define NTLMSSP_AUTH_CREDS
struct pwd_info
{
	int null_pwd;
	int cleartext;
	int crypted;

	char password[129];

	unsigned char smb_lm_pwd[16];
	unsigned char smb_nt_pwd[16];

	unsigned char smb_lm_owf[24];
	unsigned char smb_nt_owf[128];
	size_t nt_owf_len;

	unsigned char lm_cli_chal[8];
	unsigned char nt_cli_chal[128];
	size_t nt_cli_chal_len;
};

typedef struct ntuser_creds
{
	char user_name[129];
	char domain[129];
	struct pwd_info pwd;

	unsigned32 ntlmssp_flags;

} ntuser_creds_t, *ntuser_creds_p_t;

/* this blob contains the information for an nt security context
 */
typedef struct rpc_np_sec_context_t {
    char pipe_name[128];
    char host_name[128];
    unsigned32 Length;
    void *blob;
} rpc_np_sec_context_t, *rpc_np_sec_context_p_t;

#endif

/*
 * 
 * (c) Copyright 1991 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1991 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1991 DIGITAL EQUIPMENT CORPORATION
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
 */
#ifndef sec_authn_v0_0_included
#define sec_authn_v0_0_included
typedef struct {
    int length;
    char *data;
} sec_krb_message;


#endif
