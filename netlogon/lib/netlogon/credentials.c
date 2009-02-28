/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   code to manipulate domain credentials
   Copyright (C) Luke Kenneth Casson Leighton 1996-2001
   Copyright (C) Paul Ashton                       1996
   Copyright (C) Luke Howard                       2001
   
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

#include <stdio.h>

#include <netlogon.h>
#include <byteorder.h>
#include <netlogon.h>
#include <smbhash.h>


/****************************************************************************
represent a credential as a string
****************************************************************************/
static void credstr(const char *report, const unsigned char *cred)
{
	fprintf(stderr, "\t%s: %02X%02X%02X%02X%02X%02X%02X%02X",
			report,
		cred[0], cred[1], cred[2], cred[3], 
		cred[4], cred[5], cred[6], cred[7]);
}


/****************************************************************************
  setup the session key. 
Input: 8 byte challenge block
       8 byte server challenge block
      16 byte md4 encrypted password
Output:
      8 byte session key
****************************************************************************/
void nl_cred_session_key(CYPHER_BLOCK *clnt_chal, CYPHER_BLOCK *srv_chal,
		const char *pass,
	      unsigned char session_key[8])
{
	ULONG sum[2];
	unsigned char sum2[8];

	sum[0] = IVAL(clnt_chal->data, 0) + IVAL(srv_chal->data, 0);
	sum[1] = IVAL(clnt_chal->data, 4) + IVAL(srv_chal->data, 4);

	SIVAL(sum2,0,sum[0]);
	SIVAL(sum2,4,sum[1]);

	nl_cred_hash1(session_key, sum2,(const unsigned char *)pass);

	/* debug output */
	fprintf(stderr, "nl_cred_session_key\n");

	credstr("clnt_chal", clnt_chal->data);
	credstr("srv_chal ", srv_chal->data);
	credstr("clnt+srv ", sum2);
	credstr("sess_key ", session_key);
}


/****************************************************************************
create a credential

Input:
      8 byte sesssion key
      8 byte stored credential
      4 byte timestamp

Output:
      8 byte credential
****************************************************************************/
void nl_cred_create(const unsigned char session_key[8], CYPHER_BLOCK *stor_cred, int timestamp, 
		 CYPHER_BLOCK *cred)
{
	CYPHER_BLOCK time_cred;

	SIVAL(time_cred.data, 0, IVAL(stor_cred->data, 0) + timestamp);
	SIVAL(time_cred.data, 4, IVAL(stor_cred->data, 4));

	nl_cred_hash2(cred->data, time_cred.data, (unsigned char*)session_key);

	/* debug output*/
	fprintf(stderr, "nl_cred_create\n");

	credstr("sess_key ", session_key);
	credstr("stor_cred", stor_cred->data);
	fprintf(stderr, "	timestamp: %x\n"    , timestamp);
	credstr("timecred ", time_cred.data);
	credstr("calc_cred", cred->data);
}


/****************************************************************************
  check a supplied credential

Input:
      8 byte received credential
      8 byte sesssion key
      8 byte stored credential
      4 byte timestamp

Output:
      returns 1 if computed credential matches received credential
      returns 0 otherwise
****************************************************************************/
int nl_cred_assert(const CYPHER_BLOCK *cred, unsigned char session_key[8],
		CYPHER_BLOCK *stored_cred, int timestamp)
{
	CYPHER_BLOCK cred2;

	nl_cred_create(session_key, stored_cred, timestamp, &cred2);

	/* debug output*/
	fprintf(stderr, "nl_cred_assert\n");

	credstr("challenge ", cred->data);
	credstr("calculated", cred2.data);

	if (memcmp(cred->data, cred2.data, 8) == 0)
	{
		fprintf(stderr, "credentials check ok\n");
		return 1;
	}
	else
	{
		fprintf(stderr, "credentials check wrong\n");
		return 0;
	}
}


/****************************************************************************
  checks credentials; generates next step in the credential chain
****************************************************************************/
int nl_clnt_nl_deal_with_creds(unsigned char sess_key[8],
			  NETLOGON_AUTHENTICATOR *sto_clnt_cred, NETLOGON_AUTHENTICATOR *rcv_srv_cred)
{
	ULONG new_clnt_time;
	ULONG new_cred;

	fprintf(stderr, "nl_clnt_nl_deal_with_creds: %d\n", __LINE__);

	/* increment client time by one second */
	new_clnt_time = sto_clnt_cred->timestamp + 1;

	/* check that the received server credentials are valid */
	if (!nl_cred_assert(&(rcv_srv_cred->challenge), sess_key,
			 &(sto_clnt_cred->challenge), new_clnt_time))
	{
		return 0;
	}

	/* first 4 bytes of the new seed is old client 4 bytes + clnt time + 1 */
	new_cred = IVAL(sto_clnt_cred->challenge.data, 0);
	new_cred += new_clnt_time;

	/* store new seed in client credentials */
	SIVAL(sto_clnt_cred->challenge.data, 0, new_cred);

	credstr("new clnt cred", sto_clnt_cred->challenge.data);
	return 1;
}


/****************************************************************************
  checks credentials; generates next step in the credential chain
****************************************************************************/
int nl_deal_with_creds(unsigned char sess_key[8],
		     NETLOGON_AUTHENTICATOR *sto_clnt_cred, 
		     const NETLOGON_AUTHENTICATOR *rcv_clnt_cred, NETLOGON_AUTHENTICATOR *rtn_srv_cred)
{
	ULONG new_clnt_time;
	ULONG new_cred;

	fprintf(stderr, "nl_deal_with_creds: %d\n", __LINE__);

	/* check that the received client credentials are valid */
	if (!nl_cred_assert(&(rcv_clnt_cred->challenge), sess_key,
                    &(sto_clnt_cred->challenge), rcv_clnt_cred->timestamp))
	{
		return 0;
	}

	/* increment client time by one second */
	new_clnt_time = rcv_clnt_cred->timestamp + 1;

	/* first 4 bytes of the new seed is old client 4 bytes + clnt time + 1 */
	new_cred = IVAL(sto_clnt_cred->challenge.data, 0);
	new_cred += new_clnt_time;

	fprintf(stderr, "nl_deal_with_creds: new_cred[0]=%lx\n", new_cred);

	/* doesn't matter that server time is 0 */
	rtn_srv_cred->timestamp = 0;

	fprintf(stderr, "nl_deal_with_creds: new_clnt_time=%lx\n", new_clnt_time);

	/* create return credentials for inclusion in the reply */
	nl_cred_create(sess_key, &(sto_clnt_cred->challenge), new_clnt_time,
	            &(rtn_srv_cred->challenge));
	
	credstr("nl_deal_with_creds: clnt_cred", sto_clnt_cred->challenge.data);

	/* store new seed in client credentials */
	SIVAL(sto_clnt_cred->challenge.data, 0, new_cred);

	return 1;
}


