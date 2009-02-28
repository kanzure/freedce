/*
 * netlogon_client  : demo DCE RPC application
 *
 * Copyright (C) 1996-2001 - Luke Kenneth Casson Leighton <lkcl@dcerpc.net> 
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dce/rpc.h>
#include <dce/dce_error.h>
#include <dce/sec_authn.h>

#include <nt/lsarpc.h>
#include <netlogon.h>

#include <credentials.h>
#include <misc.h>
#include <getopt.h>
#include <winerror.h>

#define MAX_USER_INPUT 32768
#define MAX_LINE 32768

/*
 * usage()
 */

extern void rpc__dbg_set_switches(char *, unsigned32 *status);

/****************************************************************************
Generate the next creds to use.
****************************************************************************/

static void gen_next_creds(const unsigned char sess_key[8],
		NETLOGON_AUTHENTICATOR *clnt_cred, 
		NETLOGON_AUTHENTICATOR * new_clnt_cred)
{
	/*
	 * Create the new client credentials.
	 */

	clnt_cred->timestamp = time(NULL);

	memcpy(new_clnt_cred, clnt_cred, sizeof(*new_clnt_cred));

	/* Calculate the new credentials. */
	nl_cred_create(sess_key, &(clnt_cred->challenge),
		    new_clnt_cred->timestamp,
		    &(new_clnt_cred->challenge));

}

#define CHECK_CREDS(status, srv_chal, sess_key) \
if (status == 0)                             \
{                                                  \
	if (nl_cred_assert(&srv_chal, sess_key, &srv_chal, 0) == 0)                                 \
	{                                 \
		fprintf(stderr, "server replied with bad credential (bad trust account password ?).\n");                                 \
		status = NT_STATUS_NETWORK_CREDENTIAL_CONFLICT;                                 \
	}                                 \
}

static void usage()
{
  printf("usage:   test_client [-h hostname] [-u] [-t]\n");
  printf("         -u:  use UDP protocol \n");
  printf("         -t:  use TCP protocol (default) \n");
  printf("         -v:  more verbosity\n");
  printf("         -h:  specify host where RPC server lives \n\n");
  exit(0);
}

int
main(argc, argv)
 int argc;
 char *argv[];
{

  /* 
   * command line processing and options stuff
   */

  extern char *optarg;
  extern int optind, opterr, optopt;
  int c;

  int verbose = 1;
  int use_udp = 0;
  int use_tcp = 0;
  int use_lrpc = 0;
  char rpc_host[128] = "localhost";
  char * protocol;
  NETLOGON_AUTHENTICATOR clnt_cred;
  NETLOGON_AUTHENTICATOR new_clnt_cred;
  NETLOGON_AUTHENTICATOR srv_cred;
  WCHAR *logon_srv;
  WCHAR *logon_client;
  WCHAR *trust_acct;
  unsigned char sess_key[8];
  unsigned char trust_pass[16];
  ULONG neg_flags;

  /*
   * stuff needed to make RPC calls
   */

  unsigned32 status;
  

#if 0
        rpc__dbg_set_switches("0-20.30", &status);
            if (status != rpc_s_ok)
            {
                fprintf(stderr, "*** Error setting debug level - %lx\n",
                    status);
            }
#endif

  /*
   * Process the cmd line args
   */
  
    while ((c = getopt(argc, argv, "h:utlv")) != EOF)
    {
      switch (c)
	{
	 case 'l':
	   use_lrpc = 1;
	   break;
	 case 'u':
	   use_udp = 1;
	   break;
	 case 't':
	   use_tcp = 1;
	   break;
	 case 'v':
	   verbose = 0;
	   break;
   	 case 'h':
	   strncpy(rpc_host, optarg, sizeof(rpc_host)-1);
	   break;
	 default:
	   usage();
	}
    }

  if (!use_tcp && !use_udp && !use_lrpc) use_tcp=1;

  if (use_udp) 
    protocol = "udp";
  else if (use_lrpc)
  {
    protocol = "lrpc";
	strcpy(rpc_host, ".");
  }
  else
    protocol = "tcp";

  printf("%s\n", protocol);


  /*
   * Do the RPC call
   */

  printf ("calling lsa server\n");
  logon_srv = (unsigned short *)"\\\0\\\0.\0\0\0";

  {
	  unsigned32 st1;
	  LSA_OBJECT_ATTRIBUTES attr;
	idl_void_p_t pol_sec;
	LSA_HANDLE lsa_pol;

	memset(&attr, 0, sizeof(attr));
	attr.length = sizeof(attr);

	/* lookup domain controller; receive a policy handle */
	st1 = LsarOpenPolicy2(logon_srv, &attr,
                                     0x00000001,
				     &lsa_pol);


	if (st1 == rpc_s_ok)
	{
		unsigned st2;
		UNICODE_STRING secret_name =
		{
			24, 24,
			(unsigned short *)"$\0M\0A\0C\0H\0I\0N\0E\0.\0A\0C\0C\0\0\0"
		};
		st2 = LsarOpenSecret(lsa_pol, &secret_name,
				0x00000001,
				&pol_sec);

		if (st2 == rpc_s_ok)
		{
			LSA_SECRET *secret = NULL;
			LARGE_INTEGER sec_time;

			status = LsarQuerySecret(pol_sec,
					secret, &sec_time,
					NULL, NULL);
			if (secret != NULL)
			{
				printf("secret read (%d bytes)\n",
						secret->length);
				nl_dump_data((char*)secret->string, secret->length);
                /* TODO: free memory? */
			}
		
			st2 = LsarClose(&pol_sec);
		}
		st1 = LsarClose(&lsa_pol);
	}
#if 0
	/* lookup domain controller; receive a policy handle */
	res1 = res ? lsa_open_secret(&lsa_pol,
				     secret_name, SEC_RIGHTS_MAXIMUM_ALLOWED,
				     &pol_sec) : False;

	res2 = res1 ? lsa_query_secret(&pol_sec, secret, last_update) : False;

	res1 = res1 ? lsa_close(&pol_sec) : False;

	res = res ? lsa_close(&lsa_pol) : False;

	return res2;
#endif
}

  printf ("calling netlogon server\n");
  logon_client = (unsigned short *)"H\0I\0G\0H\0F\0I\0E\0L\0D\0\0\0";
  trust_acct = (unsigned short *)"H\0I\0G\0H\0F\0I\0E\0L\0D\0$\0\0\0";

  status = NetrServerReqChallenge(logon_srv, logon_client,
		&clnt_cred.challenge, &srv_cred.challenge);

    if (status != rpc_s_ok)
    {
	dce_error_string_t dceErrorMsg;
	int msgStatus;
	dce_error_inq_text(status, dceErrorMsg, &msgStatus);
	printf ("\t%s\n", (char *)dceErrorMsg);
    }

	nl_cred_session_key(&clnt_cred.challenge, &srv_cred.challenge,
			(char*)trust_pass, sess_key);

	neg_flags = 0x1ff;

	gen_next_creds(sess_key, &clnt_cred, &new_clnt_cred);

  status = NetrServerAuthenticate2(logon_srv, trust_acct, 
		  NETLOGON_SECURE_CHANNEL_WORKSTATION, logon_client,
		&new_clnt_cred.challenge, &srv_cred.challenge,
		&neg_flags);

	CHECK_CREDS(status, srv_cred.challenge, sess_key);
  /*
   * Done. Now gracefully teardown the RPC binding to the server
   */

  exit(0);
  
}

