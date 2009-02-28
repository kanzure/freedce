/*
 * netlogon_server      : demo DCE RPC application
 *
 * Jim Doyle, jrd@bu.edu  09-05-1998
 *
 *
 */

#include <dce/config.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <dce/rpc.h>
#include <dce/pthread_exc.h>
#include "netlogon.h"
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
 * ReverseIt() implements the interface specified in netlogon.idl
 *
 */

int main(int ac __attribute__ ((__unused__)),
		 char *av[] __attribute__ ((__unused__)))
{
	unsigned32 status;
	rpc_binding_vector_p_t server_binding;
	char *string_binding;
	unsigned32 i;

        rpc__dbg_set_switches("0-20.30", &status);
            if (status != rpc_s_ok)
            {
                fprintf(stderr, "*** Error setting debug level - %x\n",
                    status);
            }

	/*
	 * Register the Interface with the local endpoint mapper (rpcd)
	 */

	printf("Registering server.... \n");
	rpc_server_register_if(netlogon_v1_0_s_ifspec, NULL, NULL, &status);
	nl_chk_dce_err(status, "rpc_server_register_if()", "", 1);

	printf("registered.\nPreparing binding handle...\n");

	/*
	 * Prepare the server binding handle
	 * use all avail protocols (UDP and TCP). This basically allocates
	 * new sockets for us and associates the interface UUID and
	 * object UUID of with those communications endpoints.
	 */


	/*
	rpc_server_use_protseq_ep("ncalrpc", rpc_c_protseq_max_calls_default,
							  "netlogon", &status);
                              */
	   rpc_server_use_protseq((unsigned char*)"ncacn_ip_tcp",
                              rpc_c_protseq_max_calls_default, &status);
       /*
	   rpc_server_use_all_protseqs(rpc_c_protseq_max_calls_default, &status);
	 */
	/*
	 */
	nl_chk_dce_err(status, "rpc_server_use_protseq()", "", 1);
	rpc_server_inq_bindings(&server_binding, &status);
	nl_chk_dce_err(status, "rpc_server_inq_bindings()", "", 1);

	/*
	 * Register bindings with the endpoint mapper
	 */

	printf("registering bindings with endpoint mapper\n");

	rpc_ep_register(netlogon_v1_0_s_ifspec, server_binding, NULL,
					(unsigned char *)"netlogon server", &status);
	nl_chk_dce_err(status, "rpc_ep_register()", "", 1);

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
	rpc_ep_unregister(netlogon_v1_0_s_ifspec, server_binding, NULL, &status);
	nl_chk_dce_err(status, "rpc_ep_unregister()", "", 0);

	/*
	 * retire the binding information
	 */

	printf("Cleaning up communications endpoints... \n");
	rpc_server_unregister_if(netlogon_v1_0_s_ifspec, NULL, &status);
	nl_chk_dce_err(status, "rpc_server_unregister_if()", "", 0);

	exit(0);

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
