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
 * by Miroslaw Dobrzanski-Neumann <mirek-dn@t-online.de> 
 */

/*
 * Fixed memory leaks in macro implementation ENDTRY and RERAISE
 * in case an exception is caught destroy_exc(_eb); was not called
 * Thomas Schuetzkowski <Thomas.Schuetzkowski@web.de>
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

#ifndef EXC_HANDLING_H
#define EXC_HANDLING_H

/*
 * this macro allows us safely include pthd signatures
 * without any mapping macros
 */
#define ___INSIDE_EXC_HANDLING_H 1

/*
**
**  NAME:
**
**      exc_handling.h
**
**  FACILITY:
**
**      Exceptions
**
**  ABSTRACT:
** 
**  Pthread based exception package support header file.
**
**  This header file defines a TRY/CATCH exception mechanism that runs
**  'on top of' LinuxThreads.
**
**  It should be possible to adapt this to other P1003.4a/D4 pthreads
**  implementations.
** 
**  This implementation of DCE Exceptions on LinuxThreads relies on some
**  features of the LinuxThreads library that MAY NOT BE AVAILABLE ON
**  other implementations. For each TRY block, we install a Thread
**  Cancellation Handler which catches a thread cancel and converts it
**  to a RAISEd exception in the current TRY context block. We rely on
**  the ability to setjmp() out of the Thread Cancellation handler dive
**  into the Exception Handling code of the library. In general, setjmp()
**  out of a pthread cancel handler is a bad idea, however, it works
**  with the LinuxThreads implementation.
**
**  Certain other implementations of Pthreads forbid setjmp()ing out
**  of a cancellation handler, since it may interfere with the proper
**  maintainance of the threads context by the Pthreads API library.
** 
*/


#ifndef __DCETHREADS_CONF_H
#  include "dcethreads_conf.h"
#endif

#include </usr/include/pthread.h>          /* Import platform LinuxThreads */
#include "pthread_dce_common.h"
#include "pthread_dce.h"

#include <setjmp.h>



/*
 * Undefine Define the necessary POSIX features.
 * but only if they are not already redefined
 * Do not define this guard in user space
 */


#ifndef _DCE_PTHREADS_COMPAT_MACROS_


#define WARNING_CONFLICTING_API_USAGE \
 ERROR: Attempted conlicting use of DCE vs. POSIX Draft 7 Pthreads API

#undef pthread_attr_default
#undef pthread_mutexattr_default
#undef pthread_condattr_default

#define pthread_attr_default          WARNING_CONFLICTING_API_USAGE
#define pthread_mutexattr_default     WARNING_CONFLICTING_API_USAGE
#define pthread_condattr_default      WARNING_CONFLICTING_API_USAGE

#undef pthread_attr_create 
#undef pthread_attr_delete
#undef pthread_attr_setstacksize
#undef pthread_attr_getstacksize
#undef pthread_attr_getprio
#undef pthread_attr_setprio
#undef pthread_attr_getsched
#undef pthread_attr_setsched
#undef pthread_attr_getinheritedsched
#undef pthread_attr_setinheritedsched
#undef pthread_attr_getguardsize_np
#undef pthread_attr_setguardsize_np

#define pthread_attr_create            WARNING_CONFLICTING_API_USAGE
#define pthread_attr_delete            WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setstacksize      WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getstacksize      WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getprio           WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setprio           WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getsched          WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setsched          WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getinheritedsched WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setinheritedsched WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getguardsize_np   WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setguardsize_np   WARNING_CONFLICTING_API_USAGE

#undef pthread_mutex_init
#undef pthread_mutex_destroy
#undef pthread_mutex_lock   
#undef pthread_mutex_unlock 
#undef pthread_mutex_trylock
#undef pthread_mutexattr_create
#undef pthread_mutexattr_delete
#undef pthread_mutexattr_getkind_np
#undef pthread_mutexattr_setkind_np

#define pthread_mutex_init             WARNING_CONFLICTING_API_USAGE
#define pthread_mutex_destroy          WARNING_CONFLICTING_API_USAGE
#define pthread_mutex_lock             WARNING_CONFLICTING_API_USAGE
#define pthread_mutex_unlock           WARNING_CONFLICTING_API_USAGE
#define pthread_mutex_trylock          WARNING_CONFLICTING_API_USAGE
#define pthread_mutexattr_create       WARNING_CONFLICTING_API_USAGE
#define pthread_mutexattr_delete       WARNING_CONFLICTING_API_USAGE
#define pthread_mutexattr_getkind_np   WARNING_CONFLICTING_API_USAGE
#define pthread_mutexattr_setkind_np   WARNING_CONFLICTING_API_USAGE

#undef pthread_cond_init
#undef pthread_cond_destroy 
#undef pthread_cond_signal 
#undef pthread_cond_signal_int_np 
#undef pthread_cond_broadcast
#undef pthread_cond_wait   
#undef pthread_cond_timedwait
#undef pthread_condattr_create
#undef pthread_condattr_delete

#define pthread_cond_init              WARNING_CONFLICTING_API_USAGE
#define pthread_cond_destroy           WARNING_CONFLICTING_API_USAGE
#define pthread_cond_signal            WARNING_CONFLICTING_API_USAGE
#define pthread_cond_signal_int_np     WARNING_CONFLICTING_API_USAGE
#define pthread_cond_broadcast         WARNING_CONFLICTING_API_USAGE
#define pthread_cond_wait              WARNING_CONFLICTING_API_USAGE
#define pthread_cond_timedwait         WARNING_CONFLICTING_API_USAGE
#define pthread_condattr_create        WARNING_CONFLICTING_API_USAGE
#define pthread_condattr_delete        WARNING_CONFLICTING_API_USAGE

#undef pthread_lock_global_np         
#undef pthread_unlock_global_np       

#define pthread_lock_global_np         WARNING_CONFLICTING_API_USAGE
#define pthread_unlock_global_np       WARNING_CONFLICTING_API_USAGE

#undef pthread_equal 
#undef pthread_create
#undef pthread_detach 
#undef pthread_exit  
#undef pthread_join  
#undef pthread_self 
#undef pthread_setspecific 
#undef pthread_getspecific 
#undef pthread_setprio    
#undef pthread_getprio   
#undef pthread_setscheduler
#undef pthread_getscheduler
#undef pthread_yield 
#undef pthread_once  
#undef pthread_delay_np 
#undef pthread_keycreate
#undef pthread_setcancel
#undef pthread_setasynccancel
#undef pthread_cancel      
#undef pthread_testcancel 
#undef pthread_get_expiration_np 
#undef pthread_get_unique_np  
#undef pthread_getunique_np
#undef pthread_signal_to_cancel_np
#undef pthread_is_multithreaded_np

#define pthread_equal                  WARNING_CONFLICTING_API_USAGE
#define pthread_create                 WARNING_CONFLICTING_API_USAGE
#define pthread_detach                 WARNING_CONFLICTING_API_USAGE
#define pthread_exit                   WARNING_CONFLICTING_API_USAGE
#define pthread_join                   WARNING_CONFLICTING_API_USAGE
#define pthread_self                   WARNING_CONFLICTING_API_USAGE
#define pthread_setspecific            WARNING_CONFLICTING_API_USAGE
#define pthread_getspecific            WARNING_CONFLICTING_API_USAGE
#define pthread_setprio                WARNING_CONFLICTING_API_USAGE
#define pthread_getprio                WARNING_CONFLICTING_API_USAGE
#define pthread_setscheduler           WARNING_CONFLICTING_API_USAGE
#define pthread_getscheduler           WARNING_CONFLICTING_API_USAGE
#define pthread_yield                  WARNING_CONFLICTING_API_USAGE
#define pthread_once                   WARNING_CONFLICTING_API_USAGE
#define pthread_delay_np               WARNING_CONFLICTING_API_USAGE
#define pthread_keycreate              WARNING_CONFLICTING_API_USAGE
#define pthread_setcancel              WARNING_CONFLICTING_API_USAGE
#define pthread_setasynccancel         WARNING_CONFLICTING_API_USAGE
#define pthread_cancel                 WARNING_CONFLICTING_API_USAGE
#define pthread_testcancel             WARNING_CONFLICTING_API_USAGE
#define pthread_get_expiration_np      WARNING_CONFLICTING_API_USAGE
#define pthread_get_unique_np          WARNING_CONFLICTING_API_USAGE
#define pthread_getunique_np           WARNING_CONFLICTING_API_USAGE
#define pthread_signal_to_cancel_np    WARNING_CONFLICTING_API_USAGE
#define pthread_is_multithreaded_np    WARNING_CONFLICTING_API_USAGE

#undef  atfork
#undef  pthread_atfork
#define atfork                         WARNING_CONFLICTING_API_USAGE
#define pthread_atfork                 WARNING_CONFLICTING_API_USAGE

/************************************************************************
 *
 * Boiler-plate compiler redirection for illegal, attempted use of
 * Draft 7 API functions.
 *
 * These definitions insure that any use of Draft 7 API functions not
 * defined in Draft 4 _CANNOT_ be used by application programmers.
 * This insures that people dont attempt to use this adapter layer with
 * code that uses Both Draft 7 and Draft 4 functions.
 *
 *
 ************************************************************************/


#undef pthread_attr_init
#undef pthread_attr_destroy
#undef pthread_attr_setdetachstate
#undef pthread_attr_getdetachstate
#undef pthread_attr_setschedparam
#undef pthread_attr_getschedparam
#undef pthread_attr_setschedpolicy
#undef pthread_attr_getschedpolicy
#undef pthread_attr_setinheritsched
#undef pthread_attr_getinheritsched
#undef pthread_attr_setscope
#undef pthread_attr_getscope
#undef pthread_attr_setschedparam
#undef pthread_attr_getschedparam
#undef pthread_mutexattr_init
#undef pthread_mutexattr_destroy
#undef pthread_condattr_init
#undef pthread_condattr_destroy
#undef pthread_cleanup_push_defer
#undef pthread_cleanup_pop_restore
#undef pthread_kill
#undef pthread_key_create
#undef pthread_key_delete

#define pthread_attr_init                        WARNING_CONFLICTING_API_USAGE
#define pthread_attr_destroy                     WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setdetachstate              WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getdetachstate              WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setschedparam               WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getschedparam               WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setschedpolicy              WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getschedpolicy              WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setinheritsched             WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getinheritsched             WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setscope                    WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getscope                    WARNING_CONFLICTING_API_USAGE
#define pthread_attr_setschedparam               WARNING_CONFLICTING_API_USAGE
#define pthread_attr_getschedparam               WARNING_CONFLICTING_API_USAGE
#define pthread_mutexattr_init                   WARNING_CONFLICTING_API_USAGE
#define pthread_mutexattr_destroy                WARNING_CONFLICTING_API_USAGE
#define pthread_condattr_init                    WARNING_CONFLICTING_API_USAGE
#define pthread_condattr_destroy                 WARNING_CONFLICTING_API_USAGE
#define pthread_cleanup_push_defer               WARNING_CONFLICTING_API_USAGE
#define pthread_kill                             WARNING_CONFLICTING_API_USAGE
#define pthread_key_create                       WARNING_CONFLICTING_API_USAGE
#define pthread_key_delete                       WARNING_CONFLICTING_API_USAGE

#undef WARNING_CONFLICTING_API_USAGE

#endif /* _DCE_PTHREADS_COMPAT_MACROS_ */

/*
 * Define the Implementation Type of DCE Exceptions
 */

#define DCE_EXCEPTIONS_IMPL_CMALIB 0     /* DCE CMA Library Exceptions */
#define DCE_EXCEPTIONS_IMPL_SETJMP 1     /* Thread-Safe setjmp() based Exc */
#define DCE_EXCEPTIONS_IMPL_GNUEXC 2     /* GNU Binutils/EGCS Exceptions */

#define DCE_EXCEPTIONS_IMPLEMENTATION    DCE_EXCEPTIONS_IMPL_SETJMP


/* --------------------------------------------------------------------------- */

__BEGIN_DECLS

typedef enum _exc_kind_t 
{
    _exc_c_kind_address = 0x02130455,  
    _exc_c_kind_status  = 0x02130456
} _exc_kind_t;

typedef struct _exc_exception
{
    _exc_kind_t kind;
    union match_value {
        long int                value;
        struct _exc_exception   *address;       
    } match;
  char printable_name[64+1];
  char thrown_at_srcfile[80+1];            /* which file RAISE was done in */
  unsigned int thrown_at_lineno;           /* where RAISE was done in */
} EXCEPTION;


static inline void
exc_set_status(EXCEPTION * e, long int s)
{
    e->match.value = s;
    e->kind = _exc_c_kind_status;
}

static inline int
exc_get_status(EXCEPTION * e, long int * s)
{
    return (e->kind == _exc_c_kind_status ? (*s = e->match.value, 0) : -1);
}

extern pthread_key_t _exc_key;
extern pthread_once_t _dce_exclib_syncsig_setdefault;
extern int _dce_exclib_syncsig_catch[];

/*
 * Initialize a new Exception object
 */

#define EXCEPTION_INIT(e) { \
    (e).kind = _exc_c_kind_address; \
    (e).match.address = &(e); \
    strncpy((e).printable_name, #e, 64); \
    (e).thrown_at_srcfile[0] = 0; \
    (e).thrown_at_lineno = 0; }



/* --------------------------------------------------------------------------- */

/*
 * An exception handler context buffer (state block).
 *
 * Every TRY block in a C program has associated a buffer of this type.
 * A stack of context blocks for each nested TRY block is managed by
 * our exception package. The topmost context block is associated with
 * thread-specific data, making it always possible to find the most
 * recent TRY block anywhere be fetching TSD.
 */

typedef struct _exc_buf
{
    jmp_buf             jb;             /* Jump buffer */
    struct _pthread_cleanup_buffer*
                        cancel_buf;     /* thread cancel handler buffer */
    EXCEPTION           *current_exc;   /* Current exception */
    struct _exc_buf     *next;          /* Next state block */
} _exc_buf;

/* --------------------------------------------------------------------------- */

/* Prototypes of functions in exc_handing.c */

int exc_matches(EXCEPTION *cur_exc, EXCEPTION *exc);

void exc_raise_status(int s);

void _exc_raise(EXCEPTION *exc, char * file, int line) __attribute__((__noreturn__));

void _exc_reraise(EXCEPTION *exc) __attribute__((__noreturn__));

void _exc_thread_init(void);

void destroy_exc(void *_exc_cur);

void _exc_alloc_buf(_exc_buf **buf);

void exc_report(EXCEPTION *exc);

static inline void _exc_push_buf(_exc_buf * buf);

static inline void _exc_pop_buf(_exc_buf * buf);

void _exc_cancel_catcher(void * arg);


/* --------------------------------------------------------------------------- */

/*
 * The following exception is used to report an attempt to raise an
 * uninitialized exception object. It should never be raised by any other
 * code.
 */

extern EXCEPTION exc_uninitexc_e;       /* Uninitialized exception */
extern EXCEPTION exc_unhandled_exc;     /* Unhandled exception exception */
/*
 * The following exceptions are common error conditions which may be raised
 * by the thread library or any other facility following this exception
 * specification.
 */

extern EXCEPTION exc_exquota_e;         /* Exceeded quota */
extern EXCEPTION exc_insfmem_e;         /* Insufficient memory */
extern EXCEPTION exc_nopriv_e;          /* No privilege */

/*
 * The following exceptions describe hardware or operating system error
 * conditions that are appropriate for most hardware and operating system
 * platforms. These are raised by the exception facility to report operating
 * system and hardware error conditions.
 */

extern EXCEPTION exc_illaddr_e;         /* Illegal address */
extern EXCEPTION exc_illinstr_e;        /* Illegal instruction */
extern EXCEPTION exc_resaddr_e;         /* Reserved addressing mode */
extern EXCEPTION exc_privinst_e;        /* Privileged instruction */
extern EXCEPTION exc_resoper_e;         /* Reserved operand */
extern EXCEPTION exc_aritherr_e;        /* Arithmetic error */
extern EXCEPTION exc_intovf_e;          /* Integer overflow */
extern EXCEPTION exc_intdiv_e;          /* Integer divide by zero */
extern EXCEPTION exc_fltovf_e;          /* Floating overflow */
extern EXCEPTION exc_fltdiv_e;          /* Floating divide by zero */
extern EXCEPTION exc_fltund_e;          /* Floating underflow */
extern EXCEPTION exc_decovf_e;          /* Decimal overflow */
extern EXCEPTION exc_subrng_e;          /* Subrange */
extern EXCEPTION exc_excpu_e;           /* Exceeded CPU quota */
extern EXCEPTION exc_exfilsiz_e;        /* Exceeded file size */

/*
 * The following exceptions correspond directly to UNIX synchronous
 * terminating signals.  This is distinct from the prior list in that those
 * are generic and likely to have equivalents on most any operating system,
 * whereas these are highly specific to UNIX platforms.
 */

extern EXCEPTION exc_SIGTRAP_e;         /* SIGTRAP received */
extern EXCEPTION exc_SIGIOT_e;          /* SIGIOT received */
extern EXCEPTION exc_SIGUNUSED_e;       /* SIGUNUSED (Linux) received */
extern EXCEPTION exc_SIGEMT_e;          /* SIGEMT received */
extern EXCEPTION exc_SIGSYS_e;          /* SIGSYS received */
extern EXCEPTION exc_SIGPIPE_e;         /* SIGPIPE received */
extern EXCEPTION exc_unksyncsig_e;      /* Unknown synchronous signal */

/*
 * The following exception is raised in the target of a cancel
 */

extern EXCEPTION pthread_cancel_e;  
extern EXCEPTION pthread_badparam_e;            /* Bad parameter */
extern EXCEPTION pthread_existence_e;           /* Object does not exist */
extern EXCEPTION pthread_in_use_e;              /* Object is in use */
extern EXCEPTION pthread_use_error_e;           /* Object inappropriate */
extern EXCEPTION pthread_nostackmem_e;          /* No memory to alloc stack */
extern EXCEPTION pthread_exit_thread_e;         /* Used to terminate a thd */
extern EXCEPTION pthread_unimp_e;               /* Unimplemented feature */
    
/*
 * The following aliases exist for backward compatibility with CMA.
 */

#define cma_e_alerted           pthread_cancel_e
                                                    
/* --------------------------------------------------------------------------- */

/*
 * The following aliases exist for backward compatibility with CMA.
 */

#define exc_e_uninitexc         exc_uninitexc_e
#define exc_e_illaddr           exc_illaddr_e
#define exc_e_exquota           exc_exquota_e
#define exc_e_insfmem           exc_insfmem_e
#define exc_e_nopriv            exc_nopriv_e
#define exc_e_illinstr          exc_illinstr_e
#define exc_e_resaddr           exc_resaddr_e
#define exc_e_privinst          exc_privinst_e
#define exc_e_resoper           exc_resoper_e
#define exc_e_SIGTRAP           exc_SIGTRAP_e
#define exc_e_SIGIOT            exc_SIGIOT_e
#define exc_e_SIGEMT            exc_SIGEMT_e
#define exc_e_aritherr          exc_aritherr_e
#define exc_e_SIGSYS            exc_SIGSYS_e
#define exc_e_SIGPIPE           exc_SIGPIPE_e
#define exc_e_excpu             exc_excpu_e
#define exc_e_exfilsiz          exc_exfilsiz_e
#define exc_e_intovf            exc_intovf_e
#define exc_e_intdiv            exc_intdiv_e
#define exc_e_fltovf            exc_fltovf_e
#define exc_e_fltdiv            exc_fltdiv_e
#define exc_e_fltund            exc_fltund_e
#define exc_e_decovf            exc_decovf_e
#define exc_e_subrng            exc_subrng_e

#define exc_e_accvio            exc_e_illaddr
#define exc_e_SIGILL            exc_e_illinstr
#define exc_e_SIGFPE            exc_e_aritherr
#define exc_e_SIGBUS            exc_e_illaddr
#define exc_e_SIGSEGV           exc_e_illaddr
#define exc_e_SIGXCPU           exc_e_excpu
#define exc_e_SIGXFSZ           exc_e_exfilsiz

/* --------------------------------------------------------------------------- */


/*
 * _ E X C _ P U S H / P O P _ B U F ( B U F )
 *
 * Add the new _exc_buf, buf to the head of the _exc_buf list, _head .
 * This list is kept on a per-therad basis, and a pointer to the head 
 * of the list is stored under _exc_key in the thread specific data.
 */

static inline void
_exc_push_buf(_exc_buf * buf)
{

    _exc_buf *__head;
    if ((pthd4_getspecific(_exc_key, (void **)&__head)) == -1) {
      // XXX Something went seriously wrong
	pthread_cancel(pthread_self()); pthread_testcancel();
    }
    (buf)->next = __head;
    __head = (buf);
    pthd4_setspecific(_exc_key, (void *)__head);
    _pthread_cleanup_push_defer (buf->cancel_buf, 
				 _exc_cancel_catcher, NULL);
}


static inline void
_exc_pop_buf(_exc_buf * buf)
{
    _exc_buf *__head; 
    if ((pthd4_getspecific(_exc_key, (void **)&__head)) == -1) { 
      // XXX Something went seriously wrong
	pthread_cancel(pthread_self()); pthread_testcancel();
    }
    (buf) = __head;
    __head = __head->next;
    pthd4_setspecific(_exc_key, (void *)__head);
    _pthread_cleanup_pop_restore (buf->cancel_buf, 0); 
}
       
/* --------------------------------------------------------------------------- */

/*
 * mdn 24 Oct 1999
 *
 * D O _ N O T _ C L O B B E R
 *
 * gcc tends to clobber automatic variables
 * use this macro if you dont want this
 * YOU MUST USE IT IN THE PRESENCE OF setjmp/longjmp/vfork TRY / CATCH ...
 *
 * if yout rewrite this macro keep the following in mind:
 *   1. you must take the address of the variable that should not be clobbered
 *   2. the (g)cc must still be happy about the code
 *       no warnings such:
 * 		a) statement with no effect
 *		b) value computed but not used
 *		and so on
 */

#define DO_NOT_CLOBBER(var) var = *&var


/*
 * T R Y
 *
 */
    
#define _exc_longjmp(jmpbuf, val) siglongjmp((jmpbuf)->jb, (val))
#define _exc_setjmp(jmpbuf) sigsetjmp((jmpbuf), 1)
#define disable_async_events()
#define enable_async_events()

#define TRY \
do \
{ \
    struct _pthread_cleanup_buffer _cb; \
    _exc_buf *_eb; \
    EXCEPTION *_exc_cur; \
    volatile char _exc_cur_handled = 0; \
    volatile char _exc_in_finally = 0; \
    volatile int _setjmp_res; \
    _exc_cur = *&_exc_cur; \
    _exc_thread_init(); \
    _exc_alloc_buf(&_eb); \
    _eb->cancel_buf = &_cb; \
    _exc_push_buf(_eb); \
    _setjmp_res = _exc_setjmp(_eb->jb); \
    if (_setjmp_res != 0) \
    { \
        /* we land here from a RAISE */ \
        _exc_pop_buf(_eb); \
        _exc_cur = _eb->current_exc; \
        if (_exc_in_finally) \
            _exc_reraise(_exc_cur); \
    } \
    else { \
	     /* we land here from as TRY continues through */ \
        pthd4_setspecific(_exc_key, (void *)_eb); \
    } \
    if (_setjmp_res == 0) \
    { \
        /* normal code here  */

/* --------------------------------------------------------------------------- */

/*
 * C A T C H ( e x c )
 *
 * Define the beginning of an exception handler scope for the exception
 * "exc".  The exception handler code must either "fall through" to
 * ENDTRY (indicating that the exception has been handled and to resume
 * execution after the ENDTRY), RERAISE the current exception or RAISE
 * a different exception (in both cases, propagating an unwind to the next
 * higher exception handler).
 */
#define CATCH(exc) \
    } \
    else if (exc_matches(_exc_cur, &(exc))) \
    { \
        _exc_cur_handled = 1; \
        /* exception code here */

/*
 * C A T C H _ A L L
 *
 * Define the beginning of an exception handler scope for any exception
 * not explicitly caught by a CATCH(exc).  Everything else is just like
 * CATCH(exc).
 */
#define CATCH_ALL \
    } \
    else \
    { \
        EXCEPTION *THIS_CATCH __attribute__((__unused__)) = _exc_cur; \
        _exc_cur_handled = 1; \
        /* exception code here */

/* --------------------------------------------------------------------------- */

/*
 * F I N A L L Y
 *
 * Define the beginning of a code block that is to be executed regardless
 * of whether or not an exception occurs.  FINALLY should NOT be used
 * if a CATCH or CATCH_ALL is used.
 */
#define FINALLY \
    } \
    { \
        _exc_in_finally = 1; \
        /* user finally code here */

/* --------------------------------------------------------------------------- */

/*
 * E N D T R Y
 *
 * Terminate an exception handler scope.
 *
 * We can reach the ENDTRY block under several conditions.  
 *
 * Note that we WILL NOT REACH HERE if a RERAISE or a RAISE occurred in 
 * a CATCH or CATCH_ALL block of this exception scope.
 *
 * We WILL get here if:
 *        - there was no exception raised in this TRY block.
 *        - A exception was raised, but was handled (absorbed) by a 
 *          preceeding CATCH or CATCH_ALL block.
 *        - An exception was raised, but was NOT handled (absorbed) by
 *          any eligible handlers.
 *
 * pthread_cancel_e's are NEVER absorbed. We take advantage of a special
 * property of LinuxThreads that allows us to abandon a cancel and
 * resume normal thread operation. This may not be possible on other
 * pthreads implementations.
 *
 */

#define ENDTRY \
    } \
    if (_setjmp_res != 0) \
    { \
        /* we get here from being longjmp'ed into the exception dispatch */ \
        destroy_exc(_eb); \
        if (! _exc_cur_handled ) \
            _exc_reraise(_exc_cur); \
        break; \
    } \
    else { \
        /* We get here at the end of a normal TRY block runthrough */ \
        _exc_pop_buf(_eb); \
        destroy_exc(_eb); \
    } \
} while (0); \
 /* End of exception handler scope; continue execution */


/*
 * R A I S E ( e x c )
 *
 * Raise an exception - i.e. initiate an unwind to the next
 * exception handler.
 */
#define RAISE(exc)  _exc_raise(&exc, __FILE__, __LINE__ )

/* --------------------------------------------------------------------------- */

/*
 * R E R A I S E
 *
 * Raise the current exception - i.e. initiate an unwind to next exception
 * handler.  Note: RERAISE is legal only within a CATCH or a CATCH_ALL.
 * Note _exc_cur may be NULL due to an "outside" pthread_cancel().
 */
#define RERAISE    destroy_exc(_eb); _exc_reraise(_exc_cur)

/* --------------------------------------------------------------------------- */

__END_DECLS

#undef ___INSIDE_EXC_HANDLING_H

#endif
