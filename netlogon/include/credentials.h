/*The following definitions come from  libsmb/credentials.c  */

#ifndef _NETLOGON_CREDENTIALS_H
#define _NETLOGON_CREDENTIALS_H

void nl_cred_session_key(
	CYPHER_BLOCK *clnt_chal,
	CYPHER_BLOCK *srv_chal,
	const char *pass,
	UCHAR session_key[8]);

void nl_cred_create(const UCHAR session_key[8],
	CYPHER_BLOCK *stor_cred,
	ULONG timestamp,
	CYPHER_BLOCK *cred);

int nl_cred_assert(const CYPHER_BLOCK *cred,
	UCHAR session_key[8],
	CYPHER_BLOCK *stored_cred,
	ULONG timestamp);

int nl_clnt_deal_with_creds(UCHAR sess_key[8],
	NETLOGON_AUTHENTICATOR *sto_clnt_cred,
	NETLOGON_AUTHENTICATOR *rcv_srv_cred);

int nl_deal_with_creds(UCHAR sess_key[8],
	NETLOGON_AUTHENTICATOR *sto_clnt_cred,
	const NETLOGON_AUTHENTICATOR *rcv_clnt_cred,
	NETLOGON_AUTHENTICATOR *rtn_srv_cred);

#endif /* _NETLOGON_CREDENTIALS_H */

