/*
 * samr_server      : demo DCE RPC application
 *
 * Jim Doyle, jrd@bu.edu  09-05-1998
 *
 *
 */

#define HAVE_CONFIG_H
#include <dce/config.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <dce/rpc.h>
#include <dce/pthread_exc.h>
#include "samr.h"
#include "misc.h"
#include <ctype.h>

pthread_t sig_handler_thread;
static void signal_handler(void *arg);
static void wait_for_signals();

extern void rpc__dbg_set_switches(char *, unsigned32 *status);

static void print_asc(unsigned char  const *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		fprintf(stderr, "%c", isprint(buf[i]) ? buf[i] : '.');
	}
}

static void dump_data(const char *buf1, int len)
{
	unsigned char const *buf = (unsigned char const *)buf1;
	int i = 0;

	if (buf == NULL)
	{
		fprintf(stderr, "dump_data: NULL, len=%d\n", len);
		return;
	}
	if (len < 0)
		return;
	if (len == 0)
	{
		fprintf(stderr, "\n");
		return;
	}

	fprintf(stderr, "[%03X] ", i);
	for (i = 0; i < len;)
	{
		fprintf(stderr, "%02X ", (int)buf[i]);
		i++;
		if (i % 8 == 0)
			fprintf(stderr, " ");
		if (i % 16 == 0)
		{
			print_asc(&buf[i - 16], 8);
			fprintf(stderr, " ");
			print_asc(&buf[i - 8], 8);
			fprintf(stderr, "\n");
			if (i < len)
				fprintf(stderr, "[%03X] ", i);
		}
	}

	if (i % 16 != 0)	/* finish off a non-16-char-length row */
	{
		int n;

		n = 16 - (i % 16);
		fprintf(stderr, " ");
		if (n > 8)
			fprintf(stderr, " ");
		while (n--)
			fprintf(stderr, "   ");

		n = i % 16;
		if (n > 8)
			n = 8;
		print_asc(&buf[i - (i % 16)], n);
		fprintf(stderr, " ");
		n = (i % 16) - n;
		if (n > 0)
			print_asc(&buf[i - n], n);
		fprintf(stderr, "\n");
	}
}

/*
 *
 * A template DCE RPC server
 *
 * main() contains the basic calls needed to register an interface,
 * get communications endpoints, and register the endpoints
 * with the endpoint mapper.
 *
 * ReverseIt() implements the interface specified in samr.idl
 *
 */

int main(int ac __attribute__ ((__unused__)),
		 char *av[] __attribute__ ((__unused__)))
{
	unsigned32 status;
	rpc_binding_vector_p_t server_binding;
	char *string_binding;
	unsigned32 i;

    #if 0
        rpc__dbg_set_switches("0-20.30", &status);
            if (status != rpc_s_ok)
            {
                fprintf(stderr, "*** Error setting debug level - %x\n",
                    status);
            }
    #endif

	/*
	 * Register the Interface with the local endpoint mapper (rpcd)
	 */

	printf("Registering server.... \n");
	rpc_server_register_if(samr_v1_0_s_ifspec, NULL, NULL, &status);
	chk_dce_err(status, "rpc_server_register_if()", "", 1);

	printf("registered.\nPreparing binding handle...\n");

	/*
	 * Prepare the server binding handle
	 * use all avail protocols (UDP and TCP). This basically allocates
	 * new sockets for us and associates the interface UUID and
	 * object UUID of with those communications endpoints.
	 */


	/*rpc_server_use_protseq_ep("ncalrpc", rpc_c_protseq_max_calls_default,
					  "samr", &status); */
	 rpc_server_use_protseq((unsigned char*)"ncacn_ip_tcp",
                            rpc_c_protseq_max_calls_default, &status);
	/*rpc_server_use_all_protseqs(rpc_c_protseq_max_calls_default, &status);
	 */
	chk_dce_err(status, "rpc_server_use_protseq()", "", 1);
	rpc_server_inq_bindings(&server_binding, &status);
	chk_dce_err(status, "rpc_server_inq_bindings()", "", 1);

	/*
	 * Register bindings with the endpoint mapper
	 */

	printf("registering bindings with endpoint mapper\n");

	rpc_ep_register(samr_v1_0_s_ifspec, server_binding, NULL,
			(unsigned char *)"samr server", &status);
	chk_dce_err(status, "rpc_ep_register()", "", 1);

	printf("registered.\n");

	/*
	 * Print out the servers endpoints (TCP and UDP port numbers)
	 */

	printf("Server's communications endpoints are:\n");

	for (i = 0; i < server_binding->count; i++)
	{
		rpc_binding_to_string_binding(server_binding->binding_h[i],
				  (unsigned char **)&string_binding,
				  &status);
		if (string_binding)
			printf("\t%s\n", string_binding);
	}


	/*
	 * Start the signal waiting thread in background. This thread will
	 * Catch SIGINT and gracefully shutdown the server.
	 */

	wait_for_signals();

	/*
	 * Begin listening for calls
	 */

	printf("listening for calls.... \n");

	TRY
	{
		rpc_server_listen(rpc_c_listen_max_calls_default, &status);
	}
	CATCH_ALL
	{
		printf("Server stoppped listening\n");
	}
	ENDTRY
		/*
		 * If we reached this point, then the server was stopped, most likely
		 * by the signal handler thread called rpc_mgmt_stop_server().
		 * gracefully cleanup and unregister the bindings from the 
		 * endpoint mapper. 
		 */
		/*
		 * Kill the signal handling thread
		 */
		printf("Killing the signal handler thread... \n");
	pthread_cancel(sig_handler_thread);

	printf("Unregistering server from the endpoint mapper.... \n");
	rpc_ep_unregister(samr_v1_0_s_ifspec, server_binding, NULL, &status);
	chk_dce_err(status, "rpc_ep_unregister()", "", 0);

	/*
	 * retire the binding information
	 */

	printf("Cleaning up communications endpoints... \n");
	rpc_server_unregister_if(samr_v1_0_s_ifspec, NULL, &status);
	chk_dce_err(status, "rpc_server_unregister_if()", "", 0);

	exit(0);

}

#define CONTEXT_MAGIC 0xfeedf00d

struct connect
{
    int magic;
    int access_required;
};




idl_long_int SamrConnect(
					/* [in] */ handle_t h,
					/* [in] */
					idl_ushort_int server,
					/* [out] */
					connect_hnd_t * hnd,
					/* [in] */ idl_long_int access)
{
	char *binding_info;
	error_status_t e;
	struct connect *result;

	/*
	 * Get some info about the client binding
	 */

	rpc_binding_to_string_binding(h, (unsigned char **)&binding_info, &e);
	if (e != rpc_s_ok)
		return e;
	fprintf(stderr, "SamrConnect called by client: %s\n", binding_info);

	/*
	 *  Print the in_text
	 */

	fprintf(stderr, "\n\nFunction SamrConnect(%u)\n", server);

	result = malloc(sizeof(struct connect));
	result->magic = CONTEXT_MAGIC;
	result->access_required = access;

	*hnd = (connect_hnd_t *) result;

	return error_status_ok;
}

idl_long_int SamrCloseHandle(
			/* [in, out] */ connect_hnd_t * hnd)
{
    struct connect *p = (struct connect*) *hnd;

    if (p->magic != CONTEXT_MAGIC)
    {
        fprintf(stderr, "*** context mismatch; %08x != %08x\n", p->magic, CONTEXT_MAGIC);
        return rpc_s_invalid_handle;
    }

	fprintf(stderr, "closehandle: 0x%x\n", p->access_required);

	free(p);
	*hnd = NULL;

	return error_status_ok;
}

idl_long_int SamrSetSecurityObject(
				  /* [in] */ obj_hnd_t hnd,
				  /* [in] */
				  idl_long_int info_type,
				  /* [in] */
				  BUFFER * sec_info)
{
	return error_status_ok;
}

idl_long_int SamrQuerySecurityObject(
			/* [in] */ obj_hnd_t hnd,
			/* [in] */
			idl_long_int info_type,
			/* [out] */
			BUFFER ** sec_info)
{
	return error_status_ok;
}

idl_long_int SamrShutdownSamServer(
				  /* [in] */ connect_hnd_t hnd)
{
	return error_status_ok;
}

idl_long_int SamrLookupDomainInSamServer(
				/* [in] */ connect_hnd_t
				hnd,
				/* [in] */
				UNICODE_STRING * domain,
				/* [out] */
				SID ** sid)
{
	SID *res = NULL;

	res = (SID *) rpc_ss_allocate(sizeof(*res));
	memset(res, 0, sizeof(*res));
	res->revision = 1;
	res->subauth_count = 1;
	res->authority.value[5] = 5;
	res->subauth[0] = 32;
	*sid = res;

	return error_status_ok;
}

idl_long_int SamrEnumerateDomainsInSamServer(
		/* [in] */ connect_hnd_t hnd,
		/* [in, out] */ idl_long_int * resume_hnd,
		/* [out] */ IDX_AND_NAME_ARRAY ** domains,
		/* [in] */ idl_long_int pref_maxlen,
		/* [out] */ idl_long_int * entries)
{
	size_t sz;
	idl_long_int i;
	IDX_AND_NAME_ARRAY *res = NULL;
	IDX_AND_NAME *rn = NULL;
	idl_long_int count = 2;


	res = (IDX_AND_NAME_ARRAY *) rpc_ss_allocate(sizeof(*res));
	rn = (IDX_AND_NAME *) rpc_ss_allocate(sizeof(*rn) * count);
	res->count = count;
	res->entry = rn;

	for (i = 0; i < count; i++)
	{
		IDX_AND_NAME *idn = &(res->entry[i]);
		UNICODE_STRING *str = &idn->name;
		idn->index = 0;
		str->size = 8;
		str->length = 8;
		str->string = (unsigned short *)rpc_ss_allocate(10);
		memcpy(str->string, "t\0e\0s\0t\0\0\0", 10);
	}

	*domains = res;
	*entries = count;
	*resume_hnd = count;

	return error_status_ok;
}

idl_long_int SamrOpenDomain(
					   /* [in] */ connect_hnd_t sam_hnd,
					   /* [in] */
					   idl_long_int access,
					   /* [in] */ SID * domain,
					   /* [out] */
					   domain_hnd_t * hnd)
{
	return error_status_ok;
}

idl_long_int SamrQueryInformationDomain(
			   /* [in] */ domain_hnd_t hnd,
			   /* [in, out] */
			   idl_ushort_int * level,
			   /* [out] */
			   DOMAIN_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrSetInformationDomain(
			 /* [in] */ domain_hnd_t hnd,
			 /* [in] */
			 idl_ushort_int level,
			 /* [in] */
			 DOMAIN_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrCreateGroupInDomain(
			/* [in] */ domain_hnd_t
			hnd_dom,
			/* [in] */
			UNICODE_STRING * name,
			/* [in] */
			idl_long_int access,
			/* [out] */
			group_hnd_t * hnd,
			/* [out] */
			idl_long_int * rid)
{
	return error_status_ok;
}

idl_long_int SamrEnumerateGroupsInDomain(
				/* [in] */ domain_hnd_t
				hnd,
				/* [in, out] */
				idl_long_int * resume_hnd,
				/* [in] */
				idl_long_int mask,
				/* [out] */
				IDX_AND_NAME_ARRAY ** groups,
				/* [in] */
				idl_long_int pref_maxlen,
				/* [out] */
				idl_long_int * entries)
{
	return error_status_ok;
}

idl_long_int SamrCreateUserInDomain(
				   /* [in] */ domain_hnd_t hnd_dom,
				   /* [in] */
				   UNICODE_STRING * name,
				   /* [in] */
				   idl_long_int access,
				   /* [out] */
				   user_hnd_t * hnd,
				   /* [out] */
				   idl_long_int * rid)
{
	return error_status_ok;
}

idl_long_int SamrEnumerateUsersInDomain(
			   /* [in] */ domain_hnd_t hnd,
			   /* [in, out] */
			   idl_long_int * resume_hnd,
			   /* [in] */
			   idl_long_int mask,
			   /* [out] */
			   IDX_AND_NAME_ARRAY ** groups,
			   /* [in] */
			   idl_long_int pref_maxlen,
			   /* [out] */
			   idl_long_int * entries)
{
	return error_status_ok;
}

idl_long_int SamrCreateAliasInDomain(
			/* [in] */ domain_hnd_t
			hnd_dom,
			/* [in] */
			UNICODE_STRING * name,
			/* [in] */
			idl_long_int access,
			/* [out] */
			alias_hnd_t * hnd,
			/* [out] */
			idl_long_int * rid)
{
	return error_status_ok;
}

idl_long_int SamrEnumerateAliasesInDomain(
				 /* [in] */ domain_hnd_t
				 hnd,
				 /* [in, out] */
				 idl_long_int * resume_hnd,
				 /* [in] */
				 idl_long_int mask,
				 /* [out] */
				 IDX_AND_NAME_ARRAY ** groups,
				 /* [in] */
				 idl_long_int pref_maxlen,
				 /* [out] */
				 idl_long_int * entries)
{
	return error_status_ok;
}

idl_long_int SamrGetAliasMembersip(
				  /* [in] */ alias_hnd_t hnd,
				  /* [in] */
				  PSID_ARRAY * sids,
				  /* [out] */
				  INDEX_ARRAY * aliases)
{
	return error_status_ok;
}

idl_long_int SamrLookupNamesInDomain(
			/* [in] */ domain_hnd_t hnd,
			/* [in] */
			idl_long_int count,
			/* [in] */
			UNICODE_STRING * names,
			/* [out] */
			INDEX_ARRAY * rids,
			/* [out] */
			INDEX_ARRAY * types)
{
	return error_status_ok;
}

idl_long_int SamrLookupIdsInDomain(
				  /* [in] */ domain_hnd_t hnd,
				  /* [in] */
				  idl_long_int count,
				  /* [in] */
				  idl_long_int * rids,
				  /* [out] */
				  UNICODE_STRING_ARRAY * names,
				  /* [out] */
				  INDEX_ARRAY * types)
{
	return error_status_ok;
}

idl_long_int SamrOpenGroup(
					  /* [in] */ domain_hnd_t hnd_dom,
					  /* [in] */
					  idl_long_int access,
					  /* [in] */ idl_long_int rid,
					  /* [out] */ group_hnd_t * hnd)
{
	return error_status_ok;
}

idl_long_int SamrQueryInformationGroup(
			  /* [in] */ group_hnd_t hnd,
			  /* [in, out] */
			  idl_ushort_int * level,
			  /* [out] */
			  GROUP_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrSetInformationGroup(
			/* [in] */ group_hnd_t hnd,
			/* [in] */
			idl_ushort_int level,
			/* [in] */
			GROUP_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrAddMemberToGroup(
				 /* [in] */ group_hnd_t hnd,
				 /* [in] */
				 idl_long_int group,
				 /* [in] */
				 idl_long_int rid)
{
	return error_status_ok;
}

idl_long_int SamrDeleteGroup(
			/* [in] */ group_hnd_t hnd)
{
	return error_status_ok;
}

idl_long_int SamrRemoveMemberFromGroup(
			  /* [in] */ group_hnd_t hnd,
			  /* [in] */ idl_long_int group,
			  /* [in] */ idl_long_int rid)
{
	return error_status_ok;
}

idl_long_int SamrGetMembersInGroup(
				  /* [in] */ group_hnd_t hnd,
				  /* [out] */ MEMBER_ARRAY ** members)
{
	return error_status_ok;
}

idl_long_int SamrSetMemberAttributesOfGroup(
				   /* [in] */ group_hnd_t hnd,
				   /* [in] */
				   idl_long_int attrib)
{
	return error_status_ok;
}

idl_long_int SamrOpenAlias(
					  /* [in] */ domain_hnd_t hnd_dom,
					  /* [in] */ idl_long_int access,
					  /* [in] */ idl_long_int rid,
					  /* [out] */ alias_hnd_t * hnd)
{
	return error_status_ok;
}

idl_long_int SamrQueryInformationAlias(
			  /* [in] */ alias_hnd_t hnd,
			  /* [in, out] */
			  idl_ushort_int * level,
			  /* [out] */ ALIAS_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrSetInformationAlias(
			/* [in] */ alias_hnd_t hnd,
			/* [in, out] */
			idl_ushort_int * level,
			/* [in] */ ALIAS_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrDeleteAlias(
			/* [in] */ alias_hnd_t hnd)
{
	return error_status_ok;
}

idl_long_int SamrAddMemberToAlias(
				 /* [in] */ alias_hnd_t hnd,
				 /* [in] */ SID * member)
{
	return error_status_ok;
}

idl_long_int SamrRemoveMemberFromAlias(
			  /* [in] */ alias_hnd_t hnd,
			  /* [in] */ SID * member)
{
	return error_status_ok;
}

idl_long_int SamrGetMembersInAlias(
				  /* [in] */ alias_hnd_t hnd,
				  /* [out] */ PSID_ARRAY * members)
{
	return error_status_ok;
}

idl_long_int SamrOpenUser(
					 /* [in] */ domain_hnd_t hnd_dom,
					 /* [in] */ idl_long_int access,
					 /* [in] */ idl_long_int rid,
					 /* [out] */ user_hnd_t * hnd)
{
	return error_status_ok;
}

idl_long_int SamrDeleteUser(
					   /* [in] */ user_hnd_t hnd)
{
	return error_status_ok;
}

idl_long_int SamrQueryInformationUser(
			 /* [in] */ user_hnd_t hnd,
			 /* [in, out] */
			 idl_ushort_int * level,
			 /* [out] */ USER_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrSetInformationUser(
				   /* [in] */ user_hnd_t hnd,
				   /* [in] */ idl_ushort_int level,
				   /* [in] */ USER_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrChangePasswordUser(
		/* [in] */ user_hnd_t hnd,
		/* [in] */ idl_char unknown1,
		/* [in] */ CRYPT_HASH * hash1,
		/* [in] */ CRYPT_HASH * hash2,
		/* [in] */ idl_char unknown2,
		/* [in] */ CRYPT_HASH * hash3,
		/* [in] */ CRYPT_HASH * hash4,
		/* [in] */ idl_char unknown3,
		/* [in] */ CRYPT_HASH * hash5,
		/* [in] */ idl_char unknown4,
		/* [in] */ CRYPT_HASH * hash6)
{
	return error_status_ok;
}

idl_long_int SamrGetGroupsForUser(
				 /* [in] */ user_hnd_t hnd,
				 /* [out] */
				 USER_GROUP_ARRAY ** groups)
{
	return error_status_ok;
}

idl_long_int SamrQueryDisplayInformation(
				/* [in] */ domain_hnd_t hnd,
				/* [in, out] */
				idl_ushort_int * level,
				/* [in] */
				idl_long_int start_idx,
				/* [in] */
				idl_long_int max_entries,
				/* [in] */
				idl_long_int pref_maxsize,
				/* [out] */
				idl_long_int * total_size,
				/* [out] */
				idl_long_int * ret_size,
				/* [out] */
				DISPLAY_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrGetDisplayEnumerationIndex(
				   /* [in] */ domain_hnd_t
				   hnd,
				   /* [in] */
				   idl_ushort_int level,
				   /* [in] */
				   UNICODE_STRING * name,
				   /* [out] */
				   idl_long_int * index)
{
	return error_status_ok;
}

idl_long_int SamrTestPrivateFunctionsDomain(
				   /* [in] */ connect_hnd_t
				   hnd)
{
	return error_status_ok;
}

idl_long_int SamrTestPrivateFunctionsUser(
				 /* [in] */ user_hnd_t hnd)
{
	return error_status_ok;
}

idl_long_int SamrGetUserDomPasswordInfo(
			   /* [in] */ user_hnd_t * hnd,
			   /* [out] */
			   PASSWORD_INFO * info)
{
	fprintf(stderr, "SamrGetUserDomPasswordInfo: %d %d\n",
			info->unknown1, info->unknown2);
	return error_status_ok;
}

idl_long_int SamrRemoveMemFromForeignDom(
				/* [in] */ unknown_hnd_t hnd,
				/* [in] */ SID * member)
{
	return error_status_ok;
}

idl_long_int SamrQueryInformationDomain2(
				/* [in] */ domain_hnd_t hnd,
				/* [in, out] */
				idl_ushort_int * level,
				/* [out] */
				DOMAIN_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrQueryInformationUser2(
			  /* [in] */ user_hnd_t hnd,
			  /* [in, out] */
			  idl_ushort_int * level,
			  /* [out] */ USER_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrQueryDisplayInformation2(
				 /* [in] */ domain_hnd_t hnd,
				 /* [in, out] */
				 idl_ushort_int * level,
				 /* [in] */
				 idl_long_int start_idx,
				 /* [in] */
				 idl_long_int max_entries,
				 /* [in] */
				 idl_long_int pref_maxsize,
				 /* [out] */
				 idl_long_int * total_size,
				 /* [out] */
				 idl_long_int * ret_size,
				 /* [out] */
				 DISPLAY_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrGetDisplayEnumerationIndex2(
					/* [in] */ domain_hnd_t
					hnd,
					/* [in] */
					idl_ushort_int level,
					/* [in] */
					UNICODE_STRING * name,
					/* [out] */
					idl_long_int * index)
{
	return error_status_ok;
}

idl_long_int SamrCreateUser2InDomain(
			/* [in] */ domain_hnd_t hnd_dom,
			/* [in] */ UNICODE_STRING * name,
			/* [in] */ idl_long_int acct_ctrl,
			/* [in] */ idl_long_int access,
			/* [out] */ user_hnd_t * hnd,
			/* [out] */
			idl_long_int * unknown,
			/* [out] */ idl_long_int * rid)
{
	return error_status_ok;
}

idl_long_int SamrQueryDisplayInformation3(
				 /* [in] */ domain_hnd_t hnd,
				 /* [in, out] */
				 idl_ushort_int * level,
				 /* [in] */
				 idl_long_int start_idx,
				 /* [in] */
				 idl_long_int max_entries,
				 /* [in] */
				 idl_long_int pref_maxsize,
				 /* [out] */
				 idl_long_int * total_size,
				 /* [out] */
				 idl_long_int * ret_size,
				 /* [out] */
				 DISPLAY_INFO * info)
{
	return error_status_ok;
}

idl_long_int SamrAddMultipleMembersToAlias(
				  /* [in] */ alias_hnd_t
				  hnd,
				  /* [in] */
				  PSID_ARRAY * sids)
{
	return error_status_ok;
}

idl_long_int SamrRemoveMultMemsFromAlias(
				/* [in] */ alias_hnd_t hnd,
				/* [in] */ PSID_ARRAY * sids)
{
	return error_status_ok;
}

idl_long_int SamrOemChangePasswordUser2(
		/* [in] */ handle_t hnd,
		/* [in] */ STRING * server,
		/* [in] */ STRING * user,
		/* [in] */ CRYPT_PASSWORD * lm_newpass,
		/* [in] */ CRYPT_HASH * lm_oldhash)
{
	return error_status_ok;
}

idl_long_int SamrUniChangePasswordUser2(
		/* [in] */ handle_t hnd,
		/* [in] */ UNICODE_STRING * server,
		/* [in] */ UNICODE_STRING * user,
		/* [in] */ CRYPT_PASSWORD * nt_newpass,
		/* [in] */ CRYPT_HASH * nt_oldhash,
		/* [in] */ idl_char lm_change,
		/* [in] */ CRYPT_PASSWORD * lm_newpass,
		/* [in] */ CRYPT_HASH * lm_oldhash)
{
	fprintf(stderr, "SamrUniChangePasswordUser2\n");
	dump_data((char*)server->string, server->length);
	dump_data((char*)user->string, user->length);

	return error_status_ok;
}

idl_long_int SamrGetDomainPasswordInfo(
		/* [in] */ handle_t hnd,
		/* [in] */ UNICODE_STRING * domain,
		/* [out] */ PASSWORD_INFO * info)
{
	fprintf(stderr, "SamrGetDomainPasswordInfo\n");
	dump_data((char*)domain->string, domain->length * 2);

	/* errr... */
	info->unknown1 = 0x15;
	info->unknown2 = 0x0;

	return error_status_ok;
}

idl_long_int SamrConnect2(
		/* [in] */ handle_t h,
		/* [in] */ idl_ushort_int * server,
		/* [out] */ connect_hnd_t * hnd,
		/* [in] */ idl_long_int access)
{
	char *binding_info;
	error_status_t e;
	struct connect *result;

	/*
	 * Get some info about the client binding
	 */

	rpc_binding_to_string_binding(h, (unsigned char **)&binding_info, &e);
	if (e != rpc_s_ok)
		return e;
	fprintf(stderr, "SamrConnect called by client: %s\n", binding_info);

	/*
	 *  Print the in_text
	 */

	fprintf(stderr, "\n\nFunction SamrConnect2(%u)\n", server[0]);

	result = malloc(sizeof(struct connect));

	result->magic = CONTEXT_MAGIC;
	result->access_required = access;

	*hnd = (connect_hnd_t *) result;
	return error_status_ok;
}

idl_long_int SamrSetInformationUser2(
			/* [in] */ user_hnd_t hnd,
			/* [in] */ idl_ushort_int level,
			/* [in] */ USER_INFO * info)
{
	return error_status_ok;
}

idl_long_int Function_3b(
					/* [in] */ unknown_hnd_t element_570,
					/* [in] */ idl_ushort_int element_571,
					/* [in] */ UNICODE_STRING * element_572,
					/* [in] */ UNICODE_STRING * element_573)
{
	return error_status_ok;
}

idl_long_int Function_3c(
					/* [in] */ unknown_hnd_t element_574,
					/* [out] */ idl_ushort_int * element_575)
{
	return error_status_ok;
}

void connect_hnd_t_rundown(rpc_ss_context_t context_handle)
{
	error_status_t st;
    struct connect *p = (struct connect*) context_handle;

	fprintf(stderr, "Running down connect context.\n");

    if (p->magic != CONTEXT_MAGIC)
    {
        fprintf(stderr, "*** context mismatch; %08x != %08x\n", p->magic, CONTEXT_MAGIC);
        return;
    }

	st = SamrCloseHandle(&context_handle);
	if (st != error_status_ok)
		fprintf(stderr,
				"Running down connect context - SamrCloseHandle failed.\n");
}

void domain_hnd_t_rundown(rpc_ss_context_t context_handle)
{
}

void user_hnd_t_rundown(rpc_ss_context_t context_handle)
{
}

void group_hnd_t_rundown(rpc_ss_context_t context_handle)
{
}

void alias_hnd_t_rundown(rpc_ss_context_t context_handle)
{
}

void obj_hnd_t_rundown(rpc_ss_context_t context_handle)
{
}

void unknown_hnd_t_rundown(rpc_ss_context_t context_handle)
{
}


/*=========================================================================
 *
 * wait_for_signals()
 *
 *
 * Set up the process environment to properly deal with signals.
 * By default, we isolate all threads from receiving asynchronous
 * signals. We create a thread that handles all async signals. 
 * The signal handling actions are handled in the handler thread.
 *
 * For AIX, we cant use a thread that sigwaits() on a specific signal,
 * we use a plain old, lame old Unix signal handler.
 *
 *=========================================================================*/

void wait_for_signals()
{

	sigset_t default_signal_mask;
	sigset_t old_signal_mask;

#if defined(__linux__)
	sigemptyset(&default_signal_mask);
	pthread_sigmask(SIG_BLOCK, &default_signal_mask, &old_signal_mask);
#endif

#ifndef _AIX
	pthread_create(&sig_handler_thread, &pthread_attr_default,
				   (void *)signal_handler, NULL);
#endif

#ifdef _AIX
	signal(SIGINT, (void (*)(int))signal_handler);
#endif

}



static void signal_handler(void *arg __attribute__ ((__unused__)))
{

	sigset_t catch_signal_mask;
	sigset_t old_signal_mask;
	int which_signal;
	unsigned32 status;

#if defined(__linux__)
	sigemptyset(&catch_signal_mask);
	sigaddset(&catch_signal_mask, SIGINT);

	pthread_sigmask(SIG_BLOCK, &catch_signal_mask, &old_signal_mask);
#endif

	while (1)
	{

#ifndef _AIX
		/* Wait for a signal to arrive */
		sigwait(&catch_signal_mask, &which_signal);

		if ((which_signal == SIGINT) || (which_signal == SIGQUIT))
			rpc_mgmt_stop_server_listening(NULL, &status);
#endif

#ifdef _AIX
		rpc_mgmt_stop_server_listening(NULL, &status);
#endif

	}

}
