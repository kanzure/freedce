/*
 * echo_server      : demo DCE RPC application
 *
 * Jim Doyle, jrd@bu.edu  09-05-1998
 *
 *
 */

#ifndef EXTERNAL
#  define EXTERNAL      extern
#endif

#ifndef GLOBAL
#  define GLOBAL
#endif

#ifndef PUBLIC
#  define PUBLIC
#endif

#ifndef PRIVATE
#  define PRIVATE
#endif

#ifndef INTERNAL
#  define INTERNAL        static
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <dce/rpc.h>
#include <dce/pthread_exc.h>
#include <dce/pthreads_rename.h>
#include <dce/dce_error.h>
#include <rpcdbg.h>
#include "echo.h"
#include "misc.h"

pthread_t sig_handler_thread;
static void signal_handler(void * arg);
static void wait_for_signals();

#define STATUS_OK(s) ((s)==NULL || *(s) == rpc_s_ok)
#define SET_STATUS(s,val) *(s) = val
#define SET_STATUS_OK(s) SET_STATUS(s, error_status_ok)
#define STATUS(s) *(s)

/*
 *
 * A template DCE RPC server
 *
 * main() contains the basic calls needed to register an interface,
 * get communications endpoints, and register the endpoints
 * with the endpoint mapper.
 *
 * ReverseIt() implements the interface specified in echo.idl
 *
 */

PRIVATE void show_st(str, st)
	char            *str;
	error_status_t  *st;
{
	    dce_error_string_t estr;
	        int             tmp_st;
		    
		    dce_error_inq_text(*st, estr, &tmp_st);
		        fprintf(stderr, "(rpcd) %s: (0x%lx) %s\n", str, *st, estr);
}

PRIVATE boolean32 check_st_bad(str, st)
	char            *str;
	error_status_t  *st;
{
	    if (STATUS_OK(st)) 
		            return false;

	        show_st(str, st);
		    return true;
}


/*
 *  Process args
 */
void process_args(argc, argv)
int             argc;
char            *argv[];
{
    int             c;
    unsigned32      status;
    extern int      optind;
    extern char     *optarg;
    

    /*
     * Process args.
     */

    while ((c = getopt(argc, argv, "d:")) != EOF)
    {
        switch (c)
        {
        case 'd':
            rpc__dbg_set_switches(optarg, &status);
            if (check_st_bad("Error setting debug switches", &status))
                return;
            break;

        default:
            exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
  unsigned32 status;
  rpc_binding_vector_p_t     server_binding;
  char * string_binding;
  unsigned32 i;

  /*
#ifdef ENABLE_PTHREADS
      dcethreads_exc_lib_init();
#endif
*/

  /*
   * Register the Interface with the local endpoint mapper (rpcd)
   */

    process_args(argc, argv);

  printf ("Registering server.... \n");
  rpc_server_register_if(echo_v1_0_s_ifspec, 
			 NULL,
			 NULL,
			 &status);
      chk_dce_err(status, "rpc_server_register_if()", "", 1);

	printf("registered.\nPreparing binding handle...\n");
  
      /*
       * Prepare the server binding handle
       * use all avail protocols (UDP and TCP). This basically allocates
       * new sockets for us and associates the interface UUID and
       * object UUID of with those communications endpoints.
       */

  rpc_server_use_all_protseqs_if(0, echo_v1_0_s_ifspec, &status);
      chk_dce_err(status, "rpc_server_use_all_protseqs()", "", 1);
  rpc_server_inq_bindings(&server_binding, &status);
      chk_dce_err(status, "rpc_server_inq_bindings()", "", 1);

      /*
       * Register bindings with the endpoint mapper
       */

	printf("registering bindings with endpoint mapper\n");
		
  rpc_ep_register(echo_v1_0_s_ifspec,
		  server_binding,
		  NULL,
		  (unsigned char *)"QDA application server",
		  &status);
      chk_dce_err(status, "rpc_ep_register()", "", 1);

	printf("registered.\n");

      /*
       * Print out the servers endpoints (TCP and UDP port numbers)
       */

  printf ("Server's communications endpoints are:\n");
 
  for (i=0; i<server_binding->count; i++)
    {
      rpc_binding_to_string_binding(server_binding->binding_h[i], 
				    (unsigned char **)&string_binding,
				    &status
				    );
      if (string_binding)
		printf("\t%s\n",string_binding);
    }


  /*
   * Start the signal waiting thread in background. This thread will
   * Catch SIGINT and gracefully shutdown the server.
   */

  wait_for_signals();

  /*
   * Begin listening for calls
   */

  printf ("listening for calls.... \n");

  TRY
    {
      rpc_server_listen(rpc_c_listen_max_calls_default, &status);
    }
  CATCH_ALL
    {
      printf ("Server stoppped listening\n");
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
  sys_pthread_cancel(sig_handler_thread);

  printf ("Unregistering server from the endpoint mapper.... \n");
  rpc_ep_unregister(echo_v1_0_s_ifspec,
		    server_binding,
		    NULL,
		    &status);
  chk_dce_err(status, "rpc_ep_unregister()", "", 0);

  /*
   * retire the binding information
   */

  printf("Cleaning up communications endpoints... \n");
  rpc_server_unregister_if(echo_v1_0_s_ifspec,
			   NULL,
			   &status);
  chk_dce_err(status, "rpc_server_unregister_if()", "", 0);

  exit(0);

}


/*=========================================================================
 *
 * Server implementation of ReverseIt()
 *
 *=========================================================================*/

idl_boolean 
ReverseIt(h, in_text, out_text, status)
     rpc_binding_handle_t h;
     args * in_text;
     args ** out_text;
     error_status_t * status;
{

  char * binding_info;
  error_status_t e;
  unsigned result_size;
  args * result;
  unsigned32 i,j,l;

  /*
   * Get some info about the client binding
   */

  rpc_binding_to_string_binding(h, (unsigned char **)&binding_info, &e);
  if (e == rpc_s_ok)
    {
      printf ("ReverseIt() called by client: %s\n", binding_info);
    }

  if (in_text == NULL) return 0;

  /*
   *  Print the in_text
   */

  printf("\n\nFunction ReverseIt() -- input argments\n");
  
  for (i=0; i<in_text->argc; i++)
	printf("\t[arg %ld]: %s\n", i, in_text->argv[i]);

  printf ("\n=========================================\n");
  
  /*
   * Allocate the output args as dynamic storage bound
   * to this RPC. The output args are the same size as the
   * input args since we are simply reversing strings.
   */

  result_size = sizeof(args) + in_text->argc * sizeof(string_t *);
  result = (args * )rpc_ss_allocate(result_size);
  result->argc = in_text->argc;
  
  for (i=0; i < in_text->argc; i++)
    {
      result->argv[i] = 
	(string_t)rpc_ss_allocate(strlen(in_text->argv[i]) + 1);
    }

  /* 
   * do the string reversal
   */

  for (i=0; i < in_text->argc; i++)
    {
      l = strlen(in_text->argv[i]);
      for (j=0; j<l; j++)
	{
	  result->argv[i][j] = in_text->argv[i][l-j-1];
	}
      result->argv[i][l]=0;           /* make sure its null terminated! */
    }

  *out_text = result;
  *status = error_status_ok;

  return 1;

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

void 
wait_for_signals()
{

#if defined(__linux__)
sigset_t default_signal_mask;
sigset_t old_signal_mask;

  sigemptyset(&default_signal_mask);
  pthread_sigmask(SIG_BLOCK,  &default_signal_mask, &old_signal_mask);
#endif

#ifndef _AIX
  sys_pthread_create(&sig_handler_thread, 
		 &sys_pthread_attr_default,
		 (void*)signal_handler,
		 NULL);
#endif

#ifdef _AIX
  signal(SIGINT, (void (*)(int))signal_handler);
#endif

}



static void
signal_handler(void * arg __attribute__((__unused__)))
{

  unsigned32 status;

#if defined(__linux__)
  int which_signal;
  sigset_t catch_signal_mask;
  sigset_t old_signal_mask;

  sigemptyset(&catch_signal_mask);
  sigaddset(&catch_signal_mask, SIGINT);

  pthread_sigmask(SIG_BLOCK,  &catch_signal_mask, &old_signal_mask);
#endif

#ifdef PTHREAD_CANCEL_DEFAULT_ON
  sys_pthread_setcancel(CANCEL_ON);
#endif

  while (1) 
    {
      
#ifndef HAVE_OS_WIN32
#ifndef _AIX
      /* Wait for a signal to arrive */
      sigwait(&catch_signal_mask, &which_signal);

      if ((which_signal == SIGINT) || (which_signal == SIGQUIT))
	rpc_mgmt_stop_server_listening(NULL, &status);
#endif
#endif

#if defined(_AIX) || defined(HAVE_OS_WIN32)
	rpc_mgmt_stop_server_listening(NULL, &status);
#endif

    }
  
}









