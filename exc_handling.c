/**************************************************************************
 *
 * DCE Threads Compatibility Library for Linux
 * 
 * A DCE Threads emulation layer ontop of LinuxThreads.
 *
 * This software derives from source from several other implementations
 * and efforts to support DCE Threads including:
 *
 *    OSF/DCE V1.1 Public Domain RPC Release
 *    Michael T. Peterson's PCthreads package and DCE RPC port
 *    Andrew Sandoval's port of DCE RPC to Linux
 *
 * This package is provided under the GNU General Public License. 
 *
 * Contributors to this package include:
 *
 *	Jim Doyle                 <jrd@bu.edu>
 *      John Rousseau             (rousseau@world.std.com>
 * 	Andrew Sandoval		  <sandoval@perigee.net>
 *	Michael T. Peterson	  <mtp@big.aa.net>
 *
 ***************************************************************************/

/*
 * Many changes to support linux threads 0.8 / glibc2.1
 * by Miroslaw Dobrzanski-Neumann <mne@mosaic-ag.com> 
 */

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
 * 
 */

/*
**
**  NAME:
**
**      exception_handling.c
**
**  FACILITY:
**
**      Exceptions
**
**  ABSTRACT:
**
**  Pthread based exception package support routines.
** 
**  These support routines help to implement the TRY/CATCH exception
**  library for C used by DCE.
** 
**  We maintain a per-thread stack of exception context blocks.
**  For each context block (bracketed by TRY/ENDTRY), we catch and
**  dispatch exceptions to handlers by using setjmp()/longjmp().
**
**  In addition, we instrument each exception context block so that
**  thread cancel notification (delivered by the Pthreads library)
**  or synchronous signals, are dispatched to user-code as a RAISEd
**  exception.
** 
**  The exception currently being processed is recorded in per thread
**  data which is set by the excpetion handler package.
** 
**  Exception handlers execute with async cancellability disabled.
** 
**  Arbitrary application pthread_cancel's that are not part of a TRY/CATCH
**  scoped macro will unwind to the most recent TRY/CATCH exception handler.
** 
**  thread cancels may be absorbed by catching pthread_cancel_e. This is
**  different from most implementation of pthreads, where thread cancels
**  cannot be diverted, suspended or forgetten.
**
**  Exceptions that are thrown and not caught by ANY handlers, results
**  in an application exit.
** 
*/

#include <pthread.h>
#include <pthread_dce_common.h>
#include <pthread_dce_exc.h>
#include <pthread_dce_proto.h>

#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#ifdef __BUILD_DCE_PTHREADS_LIBRARY
# ifdef __BUILD_DCE_PTHREADS_LIBRARY_STDALONE
# include <exc_handling.h>
#else
# include <dce/exc_handling.h>
#endif /* STDALONE */
#else
#include <exc_handling.h>
#endif /* DCE Build */

/**
 **
 ** DCE Exceptions API Global Exceptions
 **/

EXCEPTION exc_uninitexc_e;
EXCEPTION exc_exquota_e;
EXCEPTION exc_insfmem_e;
EXCEPTION exc_nopriv_e;
EXCEPTION exc_illaddr_e;
EXCEPTION exc_illinstr_e;
EXCEPTION exc_resaddr_e;
EXCEPTION exc_privinst_e;
EXCEPTION exc_resoper_e;
EXCEPTION exc_aritherr_e;
EXCEPTION exc_intovf_e;
EXCEPTION exc_intdiv_e;
EXCEPTION exc_fltovf_e;
EXCEPTION exc_fltdiv_e;
EXCEPTION exc_fltund_e;
EXCEPTION exc_decovf_e;
EXCEPTION exc_subrng_e;
EXCEPTION exc_excpu_e;
EXCEPTION exc_exfilsiz_e;
EXCEPTION exc_SIGTRAP_e;
EXCEPTION exc_SIGIOT_e;
EXCEPTION exc_SIGUNUSED_e;
EXCEPTION exc_SIGEMT_e;
EXCEPTION exc_SIGSYS_e;

EXCEPTION exc_SIGPIPE_e;
EXCEPTION exc_unksyncsig_e;
EXCEPTION exc_unhandled_exc;

EXCEPTION pthread_cancel_e;
EXCEPTION pthread_badparam_e;
EXCEPTION pthread_existence_e;           
EXCEPTION pthread_in_use_e;              
EXCEPTION pthread_use_error_e;           
EXCEPTION pthread_nostackmem_e;          
EXCEPTION pthread_exit_thread_e;         
EXCEPTION pthread_unimp_e;

/**
 ** Internal Error processing tables and values 
 **
 ** This should be replaced by DCE Serviceability Messages at
 ** some point in the future.
 **
 **/

#define EXC_INT_FAIL_KEYCREATE		000000001
#define EXC_INT_FAIL_GETKEY		000000002
#define EXC_INT_FAIL_SETKEY		000000003
#define EXC_INT_FAIL_UNHANDLEDEXC	000000004
#define EXC_INT_FAIL_NULLEXC		000000005
#define EXC_INT_FAIL_NOTRY		000000006

static char  *exc_lib_errmsgs[] = 
{
 "Internal error: Invalid Exception Library error message.",
 "Unable to create initial thread key with pthread_keycreate().",
 "Unable to lookup thread key with pthread_getspecific().",
 "Unable to lookup thread key with pthread_setspecific().",
 "Application raised an unhandled exception.",
 "RAISE called with a NULL exception.",
 "Application raised an exception outside any TRY block",
 NULL
};

#define EXC_SYS_FAIL_MSGS_SIZE sizeof(exc_lib_errmsgs)
      
/* -------------------------------------------------------------------- */

/*
 * Make sure NULL is defined for init_once_block
 */
#ifndef NULL
#define NULL 0
#endif

pthread_key_t _exc_key;    

static pthread_once_t init_once_block = pthread_once_init;

/* -------------------------------------------------------------------- */


/* Prototypes */
static void sync_signal_handler(int signal, int code);

static void setup_sync_signal_handlers(void);

static void init_once(void);

/* static void set_cur_exc(EXCEPTION *exc); */

static void exc_library_fatal_error(int failure_reason,
                                    const char *name,
				    _exc_buf * exc_buffer) __attribute__((__noreturn__));


/* -------------------------------------------------------------------- */


/**
 ** Array of Signal Flags:
 **
 ** The DCE Exception Library uses this array to determine
 ** THE PROCESS-WIDE POLICY for catching and mapping synchronous
 ** signals to raised DCE Exceptions.
 **
 ** Certain Synchronous Signals are delivered to the thread (or Linux
 ** clone() process) that caused the event. Some of these signals 
 ** need to be 'caught' and then delivered to the thread as a RAISE-ed
 ** DCE Exception.
 **
 ** Certain other of these signals should not be caught, i.e. SIGSEGV,
 ** so that we can obtain coredumps with useful information where the
 ** thread abended.
 **
 ** This signal dispatch arrays gives us the future opportunity to bettter
 ** control the catching-to-raising functionality in the library.
 **
 **/

pthread_once_t _dce_exclib_syncsig_setdefault;
int _dce_exclib_syncsig_catch[NSIG] = { 0 };

/*
 * _SET_DCEEXC_SYNCSIGNALS_DEFAULT
 *
 * at-most-once initialization routine to set the default
 * signal catching to exception mapping policy for the package.
 *
 */

void
_set_dceexc_syncsignals_default()
{

  /*
   * The following signals are converted to exceptions
   */

  _dce_exclib_syncsig_catch[SIGIOT] = 1;   /* hardware fault */
#if defined(SIGEMT)
  _dce_exclib_syncsig_catch[SIGEMT] = 1;   /* hardware fault */
#endif
  _dce_exclib_syncsig_catch[SIGFPE] = 1;   /* arithmetic exception */
  _dce_exclib_syncsig_catch[SIGPIPE] = 1;  /* write to pipe w no reader */
  _dce_exclib_syncsig_catch[SIGTRAP] = 1;  /* hardware fault */
#if defined(SIGSYS)       
  _dce_exclib_syncsig_catch[SIGSYS] = 1;   /* invalid syscall() */
#endif

  /*
   * The following signals are not caught, and will result in
   * thread termination with core dump
   */

  _dce_exclib_syncsig_catch[SIGSEGV] = 0;   /* SEG FAULT */
  _dce_exclib_syncsig_catch[SIGBUS] = 0;   /* SEG FAULT */
  _dce_exclib_syncsig_catch[SIGILL] = 0;   /* ILLEGAL INSTRUCTION */

}

/*
 * S Y N C _ S I G N A L _ H A N D L E R
 *
 * This signal handler is installed for each thread invoked through
 * the DCE Pthreads Exceptions-mapping API. This signal handler
 * catches per-thread synchronous signals and maps them to DCE 
 * exceptions. It does this by RAISING an exception that results
 * in a siglongjmp() out of the signal handler and into the
 * exception dispatch code path.
 * 
 * Opon receipt of a synchronous signal, we simply raise an exception
 * to the current exception context block.
 * 
 * NOTE:
 *
 * It is assumed that it is okay to do a RAISE from a SYNCHRONOUS signal
 * handler without any ill effects.  While RAISE is doing a couple of
 * pthread operations, the fact that these would only be performed in the
 * thread that had the fault due to a synchronous fault in the thread
 * (i.e. we were in user code, not the pthread library when the fault
 * occurred) should mean that there are no pthread re-entrency problems.
 *
 * 
 */

static void 
sync_signal_handler(int signal, int code __attribute__((__unused__)))
{
    EXCEPTION *exc;
    struct sigaction action;

    switch (signal) {
        case SIGILL:    exc = &exc_illinstr_e;      break;
        case SIGTRAP:   exc = &exc_SIGTRAP_e;       break;
        case SIGIOT:    exc = &exc_SIGIOT_e;        break;
#if defined(SIGEMT)       
        case SIGEMT:    exc = &exc_SIGEMT_e;        break;
#endif        
        case SIGFPE:    exc = &exc_aritherr_e;      break;
        case SIGBUS:    exc = &exc_illaddr_e;       break;
        case SIGSEGV:   exc = &exc_illaddr_e;       break;
#if defined(SIGSYS)       
        case SIGSYS:    exc = &exc_SIGSYS_e;        break;
#endif        
        case SIGPIPE:   exc = &exc_SIGPIPE_e;       break;
        default:        exc = &exc_unksyncsig_e;    break;
    }

    /*
     * Re-install the handler for whatever signal was triggered.
     * Use the array of signal policy flags to determine if we
     * reinstall the signal.
     */

    if (_dce_exclib_syncsig_catch[signal] != 0)
      {
	sigaction(signal, (struct sigaction *)0, &action);
	if (action.sa_handler == SIG_DFL)
	  action.sa_handler = (__sighandler_t) sync_signal_handler;
	sigaction(signal, &action, (struct sigaction *)0);
      }

    _exc_raise(exc, NULL, 0);
}


/* 
 * S E T U P _ S Y N C _ S I G N A L _ H A N D L E R S
 *
 * Setup a signal handler to catch all synchronous signals and convert
 * them to exceptions.  The occurance of a synchronous signal results
 * in an immediate exception unwind on the stack of the thrad that caused
 * the signal.
 *
 * In many cases, we dont want the Exception Library to catch
 * particularly nasty signals (i.e. SIGSEGV, SIGILL, SIGBUS).
 * We want the default Unix action to occur (core dump) so that we
 * can use a debugger to figure out WHAT went wrong. 
 *
 *
 */

static void
setup_sync_signal_handlers()
{
    /*
     * Setup synchronous handlers.  Note that we get the current state of
     * each signal and then just change the handler field.  Reputed to
     * be better for some implementations.
     */

#define SIGACTION(_sig) \
{ \
    struct sigaction action; \
    (void)sigaction((_sig), (struct sigaction *)0, &action); \
    if (action.sa_handler == SIG_DFL) \
        action.sa_handler = (__sighandler_t) sync_signal_handler; \
    (void)sigaction((_sig), &action, (struct sigaction *)0); \
};

/*
 *  initialize the PER-PROCESS sync signal policy table,
 *  if it hasnt already been done. 
 */

    pthread_once(&_dce_exclib_syncsig_setdefault, 
		     _set_dceexc_syncsignals_default);
	
    if (_dce_exclib_syncsig_catch[SIGIOT]) SIGACTION(SIGIOT);
#if defined(SIGEMT)    
    if (_dce_exclib_syncsig_catch[SIGEMT]) SIGACTION(SIGEMT);
#endif    


    if (_dce_exclib_syncsig_catch[SIGILL]) SIGACTION(SIGILL);
    if (_dce_exclib_syncsig_catch[SIGTRAP]) SIGACTION(SIGTRAP);
    if (_dce_exclib_syncsig_catch[SIGFPE]) SIGACTION(SIGFPE);
    if (_dce_exclib_syncsig_catch[SIGBUS]) SIGACTION(SIGBUS);
    if (_dce_exclib_syncsig_catch[SIGSEGV]) SIGACTION(SIGSEGV);

#if defined(SIGSYS)
    if (_dce_exclib_syncsig_catch[SIGSYS]) SIGACTION(SIGSYS);
#endif    
    if (_dce_exclib_syncsig_catch[SIGPIPE]) SIGACTION(SIGPIPE);

#undef SIGACTION
}


/*
 * D E S T R O Y _ E X C
 * 
 * Destroy the thread specific exception state storage.
 */
void 
destroy_exc(void *_exc_cur)
{
    free(_exc_cur);
}


/*
 * _ E X C _ A L L O C _ B U F
 * 
 * Allocate and initialize a new _exc_buf.
 */
void
_exc_alloc_buf(_exc_buf **buf)
{
    *buf = (_exc_buf *)malloc(sizeof(**buf));
    memset((void *)*buf, 0, sizeof(**buf));
}


/*
 * I N I T _ O N C E 
 * 
 * Initialize the exception package. This function is run through pthread_once().
 * Create the key for the thread specific exception state.
 */
static void
init_once()
{
    EXCEPTION_INIT(exc_uninitexc_e);
    EXCEPTION_INIT(exc_exquota_e);
    EXCEPTION_INIT(exc_insfmem_e);
    EXCEPTION_INIT(exc_nopriv_e);
    EXCEPTION_INIT(exc_illaddr_e);
    EXCEPTION_INIT(exc_illinstr_e);
    EXCEPTION_INIT(exc_resaddr_e);
    EXCEPTION_INIT(exc_privinst_e);
    EXCEPTION_INIT(exc_resoper_e);
    EXCEPTION_INIT(exc_aritherr_e);
    EXCEPTION_INIT(exc_intovf_e);
    EXCEPTION_INIT(exc_intdiv_e);
    EXCEPTION_INIT(exc_fltovf_e);
    EXCEPTION_INIT(exc_fltdiv_e);
    EXCEPTION_INIT(exc_fltund_e);
    EXCEPTION_INIT(exc_decovf_e);
    EXCEPTION_INIT(exc_subrng_e);
    EXCEPTION_INIT(exc_excpu_e);
    EXCEPTION_INIT(exc_exfilsiz_e);
    EXCEPTION_INIT(exc_SIGTRAP_e);
    EXCEPTION_INIT(exc_SIGIOT_e);
#if defined(SIGEMT) 
    EXCEPTION_INIT(exc_SIGEMT_e);
#endif
#if defined(SIGSYS) 
    EXCEPTION_INIT(exc_SIGSYS_e);
#endif    
    EXCEPTION_INIT(exc_SIGPIPE_e);
    EXCEPTION_INIT(exc_unksyncsig_e);

    EXCEPTION_INIT(pthread_cancel_e);
    EXCEPTION_INIT(pthread_badparam_e);
    EXCEPTION_INIT(pthread_existence_e);           
    EXCEPTION_INIT(pthread_in_use_e);              
    EXCEPTION_INIT(pthread_use_error_e);           
    EXCEPTION_INIT(pthread_nostackmem_e);          
    EXCEPTION_INIT(pthread_exit_thread_e);         
    EXCEPTION_INIT(pthread_unimp_e);

    EXCEPTION_INIT(exc_unhandled_exc);

    if (pthd4_keycreate(&_exc_key, destroy_exc) != 0) {
	exc_library_fatal_error(EXC_INT_FAIL_KEYCREATE, "init_once", 0);
    }
}


/*
 * _ E X C _ T H R E A D _ I N I T
 * 
 * Initialize the exception package for per-thread stuff.
 */
void
_exc_thread_init(void)
{
    _exc_buf *eb;
    
    /*
     * One time initialization for all threads.
     */
    pthd4_once(&init_once_block, init_once);

    /*
     * If we already have the thread-specific storage to hold this thread's
     * current exception buffer (actually the pointer to the head (most recent) 
     * of the exc_buf list), we're done.
     */
    if (pthd4_getspecific(_exc_key, (void *)&eb) == 0) {
	if (eb != NULL) {
	    return;
        }
    }

    /*
     * Allocate the storage to hold this thread's "current exception".
     * Initial the current exception to "cancel" so that the thread sees
     * a cancel exception when its cancelled.  (Non-cancel
     * exceptions--i.e., those raised via RAISE--will set the value to
     * something else.  ENDTRY resets it back to cancel.)
     */
    _exc_alloc_buf(&eb);

    /* XXX     eb->current_exc = &exc_unhandled_exc;        
     * jrd 06-10-1998
     */


    eb->current_exc = &pthread_cancel_e;
    if (pthd4_setspecific(_exc_key, (void *)eb) == -1) {
        exc_library_fatal_error(EXC_INT_FAIL_SETKEY, "_exc_thread_init",eb);
    }

    /*
     * Set up signal handlers for this thread.
     */
    setup_sync_signal_handlers();
}


/* 
 * _ E X C _ S E T _ C U R R E N T
 *
 * Set the thread's current exception to the specified exception.
 */
static void _exc_set_current(EXCEPTION *exc) __attribute__((__noreturn__));
static void 
_exc_set_current(EXCEPTION *exc)
{
    _exc_buf *eb;

    if (pthd4_getspecific(_exc_key, (void *)&eb) == -1) {
        exc_library_fatal_error(EXC_INT_FAIL_GETKEY, "_exc_set_current", eb);
    }

    /*
     * It is possible to throw an dce exception in not dce code
     * since the dce threads are implemented on the top of the LinuxThreads
     * one can call throw any exception without any handler installed
     */
    if (!eb) {
	    _exc_buf excb;
	    bzero(&excb, sizeof(excb));
	    excb.current_exc = exc;
	    eb = &excb;
	    exc_library_fatal_error(EXC_INT_FAIL_NOTRY, "_exc_set_current", eb);
    }
    /* 
     * check to see if we've encounted the tail end of a cancel unwind.
     * at this point, we simply request cancellation of the current thd
     */

    if (eb->next == NULL && exc_matches(exc, &pthread_cancel_e))
	{
	  pthd4_setcancel(CANCEL_ON);
	  pthread_cancel(pthread_self());
	  pthread_testcancel();
	}
    /*
     * Set the current exception in the most recent context block
     */

    eb->current_exc = exc;

    /* 
     * Dispatch the current exception
     * If eb->next == NULL, then we have unwound as far as we can
     * and we have an unhandled exception. Request termination of the
     * program.
     */

    if (eb->next == NULL ) 
      {
        exc_library_fatal_error(EXC_INT_FAIL_UNHANDLEDEXC, 
				"_exc_set_current", 
				eb);
      }
    else
      {
	siglongjmp(eb->jb, 1);
      }
}


/* 
 * _ E X C _ R A I S E 
 * 
 * RAISE operation.
 */

void 
_exc_raise(EXCEPTION *exc, char * file, int line)
{
    if (exc == NULL) {
        exc_library_fatal_error(EXC_INT_FAIL_NULLEXC, "_exc_raise", 0);
    }

    /* If a FILE, LINE NUMBER has been specified to tag this exception
     * raise with, we save it... This allows us to track where in the
     * program source the exception was raised so we can report it
     * on program termination.
     */


    if (file != NULL)
      {
	exc->thrown_at_lineno = line;
	strncpy(exc->thrown_at_srcfile, file, 
		sizeof(exc->thrown_at_srcfile)-1);
      }
    
    _exc_set_current(exc); /* do longjmp */
}

/* 
 * _ E X C _ R E R A I S E 
 * 
 * RERAISE operation.
 */

void 
_exc_reraise(EXCEPTION * exc)
{
    if (exc == NULL) {
        exc_library_fatal_error(EXC_INT_FAIL_NULLEXC, "_exc_raise", 0);
    }
    
    _exc_set_current(exc); /* do longjmp */
}


/*
 * E X C _ M A T C H E S
 * 
 * Return true iff two exceptions match.
 */
int 
exc_matches(EXCEPTION *cur_exc,
            EXCEPTION *exc)
{

    if (cur_exc->kind == exc->kind && 
	cur_exc->match.address == exc->match.address) {
         return 1;
    }

    return 0;
}

/*
 * E X C _ R A I S E _ S T A T U S 
 *
 * Creates a new exception on heap, sets an integer status value to
 * it, and raises. NOTE:  These may memory leak if used, since there
 * are no destructors for this object.
 */

void
exc_raise_status(int s)
{
  EXCEPTION * new_exception;

  new_exception = (EXCEPTION *)malloc(sizeof(EXCEPTION));
  /* XXX What if new_exception == NULL ??   Add error processing */
  EXCEPTION_INIT(*new_exception);
  exc_set_status(new_exception, s);
  _exc_reraise(new_exception);
  
}

/*
 * E X C _ L I B R A R Y _ F A T A L _ E R R O R
 * 
 * Print out a diagnostic message and terminate the program. 
 *
 */
static void 
exc_library_fatal_error(int failure_reason, 
                         const char *name, _exc_buf * exc_buffer)
{
    if ((failure_reason < 0) || ((unsigned) failure_reason >= EXC_SYS_FAIL_MSGS_SIZE)) {
        failure_reason = 0;
    }
    
    fprintf(stderr, "****************  "
	             "LinuxDCE Exception Library FATAL ERROR  "
	            "****************  \n");

    fprintf(stderr,
            "\tat exception library routine %s()\n\treason: %s\n", 
	    name, exc_lib_errmsgs[failure_reason]);

    if (exc_buffer != NULL )
      {
	fprintf(stderr, "\tpointers: next = <%p>, "
		"current_exc=<%p>\n",
		exc_buffer->next,
		exc_buffer->current_exc);

	if (exc_buffer->current_exc != NULL)
	  {
	    exc_report(exc_buffer->current_exc);
	  }	
      }	

    fprintf(stderr, 
	    "*************************************************************"
	    "*************\n");
    
     fflush(stderr);

     /*
      * Death. No saving throw.
      *
      * Terminate all threads in the process, then exit with 255 
      */

     pthread_kill_other_threads_np();
     /* raise(SIGABRT); */
     exit(255);
}

/*
 * E X C _ R E P O R T
 * 
 * Print some information on a particular exception object. 
 */
void exc_report(exc)
EXCEPTION *exc;
{
	fflush(stdout);

	if (!exc) {
		fprintf(stderr, "\n\tLinux DCE Exception: \n"
				"\t ====== NO EXCEPTION ======\n");
		return;
	}
	fprintf(stderr, "\n\tLinux DCE Exception: \n"
			"\texception <%s>",
			exc->printable_name);
	switch (exc->kind) {
	case _exc_c_kind_address:
		fprintf(stderr, ", address <%p>\n",
				exc->match.address);
		break;
	case _exc_c_kind_status:
		fprintf(stderr, ", status value <%d>\n",
				exc->match.value);
		break;
	default:
		fprintf(stderr, ", unknown kind <%d> = <0x%x>\n",
				exc->kind, exc->kind);
	}
	fprintf(stderr, "\tthrown in src file '%s' , line %d\n",
			exc->thrown_at_srcfile, exc->thrown_at_lineno);
/*
	if (exc && exc->kind == _exc_c_kind_address) 
	{

		fprintf(stderr, "\n\tLinux DCE Exception: \n"
				"\texception <%s>, address <%p>\n"
				"\tthrown in src file '%s' , line %d\n",
				exc->printable_name,
				exc->match.address,
				exc->thrown_at_srcfile, exc->thrown_at_lineno);
	}
	*/
}


/*
 * E X C _ C A N C E L _ C A T C H E R
 *
 *  This is a routine invoked by the LinuxThreads runtime as a
 *  thread cancellation handler. It takes a cancel notification, 
 *  and converts it into an exception.
 */
void
_exc_cancel_catcher(void * arg __attribute__((__unused__)))
{
  _exc_set_current(&pthread_cancel_e); /* do longjmp */
}





