/*
 * samr_client  : demo DCE RPC application
 *
 * Jim Doyle, jrd@bu.edu, 09-05-1998
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dce/rpc.h>
#include <dce/pthread_exc.h>
#include <dce/dce_error.h>
#include <dce/sec_authn.h>
#include "samr.h"
#include <misc.h>
#include <getopt.h>

#define MAX_USER_INPUT 32768
#define MAX_LINE 32768

/*
 * Forward declarations
 */

rpc_binding_handle_t     samr_server;
static int
get_client_rpc_binding(rpc_binding_handle_t *, char *, 
		       rpc_if_handle_t, char *);

/*
handle_t open_hnd_t_bind( open_hnd_t h)
{
	void *x = h;
	x = NULL;
	return (handle_t) samr_server;
}
	
void open_hnd_t_unbind( open_hnd_t uh, handle_t h)
{
	void *x = h;
	void *ux = uh;
	x = NULL;
	ux = NULL;
}
*/
	
/*
 * usage()
 */

extern void rpc__dbg_set_switches(char *, unsigned32 *status);

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
  ntuser_creds_t usr;

  /*
   * stuff needed to make RPC calls
   */

  unsigned32 status;
  connect_hnd_t hnd = NULL;
  
#if 0
        rpc__dbg_set_switches("0-20.30", &status);
            if (status != rpc_s_ok)
            {
                fprintf(stderr, "*** Error setting debug level - %x\n",
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
   * Get a binding handle to the server using the following params:
   *
   *  1. the hostname where the server lives
   *  2. the interface description structure of the IDL interface
   *  3. the desired transport protocol (UDP or TCP)
   */

  if (get_client_rpc_binding(&samr_server, 
		      rpc_host, 
		      samr_v1_0_c_ifspec, 
		      protocol) == 0)
    {
      printf ("Couldnt obtain RPC server binding. exiting.\n");
      exit(1);
    }

  /* set user security */

  /* if running on ncalrpc with a samba tng server, this does actually work!
   */

#if 0
  memset(&usr, 0, sizeof(usr));
  usr.ntlmssp_flags = 0x82b1;
  strcpy(usr.user_name, "test");
  strcpy(usr.pwd.password, "test");
  usr.pwd.cleartext = true;
  strcpy(usr.domain, "WORKGROUP");

    rpc_binding_set_auth_info (samr_server,
			       "HIGHFIELD", 
			       rpc_c_authn_level_pkt_privacy,
			       rpc_c_authn_winnt,
			       (rpc_auth_identity_handle_t)
			       &usr,
			       rpc_c_authz_mspac,
			       &status);

    if (status !=  rpc_s_ok)
    {
	dce_error_string_t dceErrorMsg;
	int msgStatus;
	dce_error_inq_text(status, dceErrorMsg, &msgStatus);
	printf ("\t%s\n", (char *)dceErrorMsg);
	exit(1);
    }
#endif


  /*
   * Do the RPC call
   */

  printf ("calling server\n");
  status = SamrConnect2(samr_server, (unsigned short *)"t\0e\0s\0t\0\0",
                        &hnd, 0x1);
  if (status == error_status_ok && hnd != NULL)
    {
      printf ("opened ok\n");
	}

  {
	  IDX_AND_NAME_ARRAY *doms = NULL;
	  int resume = 0;
	  int entries = 0;
	int i;

	  status = SamrEnumerateDomainsInSamServer(hnd, &resume, &doms,
			  			100, &entries);

	  if (status == error_status_ok)
	  {
		  printf("enum ok: %d\n", doms->count);
	  }
	  for (i = 0; i < doms->count; i++)
	  {
		  int j;
		  UNICODE_STRING *str = &doms->entry[i].name;
		  SID *psid = NULL;
		  printf("domain: %d %d ", doms->entry[i].index, str->length);

		  for (j = 0; j < str->length / 2; j++)
		  {
			printf("%c", (char)str->string[j]);
		  }
		printf("\n");
		  status = SamrLookupDomainInSamServer(hnd, str, &psid);
		  printf("SID: S-%d-%d", psid->revision, psid->authority.value[5]);

		  for (j = 0; j < psid->subauth_count; j++)
		  {
			printf("-%u", psid->subauth[j]);
		  }
		printf("\n");
	  }
  }

  {
    UNICODE_STRING server = { 6, 8, (unsigned short*)"a\0b\0c\0\0" };
    UNICODE_STRING user = { 8, 10, (unsigned short*)"t\0e\0s\0t\0\0" };
    CRYPT_PASSWORD nt_newpass;
    CRYPT_HASH nt_oldhash;
    CRYPT_PASSWORD lm_newpass;
    CRYPT_HASH lm_oldhash;

    memset(&nt_newpass, 0, sizeof(CRYPT_PASSWORD));
    memset(&nt_oldhash, 0, sizeof(CRYPT_HASH));
    memset(&lm_newpass, 0, sizeof(CRYPT_PASSWORD));
    memset(&lm_oldhash, 0, sizeof(CRYPT_HASH));

	status = SamrUniChangePasswordUser2(samr_server, &server, &user,
		    &nt_newpass, &nt_oldhash,
		    0x1,
		    &lm_newpass, &lm_oldhash);

    printf ("SamrUniChangePasswordUser2: %x\n", status);
  }

  status = SamrCloseHandle(&hnd);

  /*
   * Done. Now gracefully teardown the RPC binding to the server
   */

  rpc_binding_free(&samr_server, &status);
  exit(0);
  
}

/*==========================================================================
 *
 * get_client_rpc_binding()
 *
 *==========================================================================
 *
 * Gets a binding handle to an RPC interface.
 *
 * parameters:
 *
 *    [in/out]  binding_handle
 *    [in]      hostname       <- Internet hostname where server lives
 *    [in]      interface_uuid <- DCE Interface UUID for service
 *    [in]      protocol       <- "udp", "tcp" or "any"
 *
 *==========================================================================*/

static int
get_client_rpc_binding(binding_handle, hostname, interface_spec, protocol)
     rpc_binding_handle_t * binding_handle;
     char * hostname;
     rpc_if_handle_t interface_spec;
     char * protocol;
{
  char * resolved_binding;
  char * printable_uuid __attribute__((__unused__));
  char * protocol_family;
  char partial_string_binding[128];
  rpc_if_id_t interface __attribute__((__unused__));
  uuid_t ifc_uuid __attribute__((__unused__));
  error_status_t status;

  /*
   * create a string binding given the command line parameters and
   * resolve it into a full binding handle using the endpoint mapper.
   *  The binding handle resolution is handled by the runtime library
   */


  if (strcmp(protocol, "lrpc")==0)
  {
    protocol_family = "ncalrpc";
    sprintf(partial_string_binding, "%s:%s[samr]", 
	  protocol_family,
	  hostname);
  }
  else if (strcmp(protocol, "udp")==0)
  {
    protocol_family = "ncadg_ip_udp";
    sprintf(partial_string_binding, "%s:%s", 
	  protocol_family,
	  hostname);
  }
  else
  {
    protocol_family = "ncacn_ip_tcp";
    sprintf(partial_string_binding, "%s:%s", 
	  protocol_family,
	  hostname);
  }

  printf("partial resolving binding for server is: %s\n", partial_string_binding);
  rpc_binding_from_string_binding((unsigned char *)partial_string_binding,
				  binding_handle,
				  &status);
      chk_dce_err(status, "string2binding()", "get_client_rpc_binding", 1);
  
  /*
   * Resolve the partial binding handle using the endpoint mapper
   */

  rpc_ep_resolve_binding(*binding_handle,
			 interface_spec,
			 &status);
  chk_dce_err(status, "rpc_ep_resolve_binding()", "get_client_rpc_binding", 1);

/*
 * Get a printable rendition of the binding handle and samr to
 * the user.
 */

  rpc_binding_to_string_binding(*binding_handle,
				(unsigned char **)&resolved_binding,
				&status);
        chk_dce_err(status, "binding2string()", "get_client_rpc_binding", 1);

  printf("fully resolving binding for server is: %s\n", resolved_binding);

  return 1;
}
