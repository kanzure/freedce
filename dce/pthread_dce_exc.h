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
 *    
 *  COPYRIGHT NOTICE
 *    
 *  Copyright (C) 1998 James R. Doyle, Andrew Sandoval, et. al.
 *  Copyright (C) 1995, 1996 Michael T. Peterson
 *  This file is part of the PCthreads (tm) multithreading library
 *  package.
 *    
 *  The source files and libraries constituting the PCthreads (tm) package
 *  are free software; you can redistribute them and/or modify them under
 *  the terms of the GNU Library General Public License as published by the 
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *    
 *  The PCthreads (tm) package is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *    
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library (see the file COPYING.LIB); if not,
 *  write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
 *  MA 02139, USA.
 */

/****************************************************************************
 *
 * pthread_dce_exc.h
 *
 * Adaptation layer to map DCE Threads onto POSIX Draft 7 threads
 * Implementation for LINUX Threads
 *
 * DCE Exception Returning Interface
 *
 ****************************************************************************
 */


/************************************************************************
 * Shadowing Macros for Source Compatibility with DCE Pthreads
 *
 ************************************************************************
 *
 * These macro's force the compiler to convert any reference to
 * pthread_xxx() to pthd4_xxx(), allowing us to provide DCE Threads
 * Pthreads draft 4 as an emulation layer over Pthreads Draft 7.
 *
 * _W_A_R_N_I_N_G_  _W_A_R_N_I_N_G_  _W_A_R_N_I_N_G_  _W_A_R_N_I_N_G_ 
 *
 ************************************************************************
 *
 * Defining -D_DCE_PTHREADS_COMPAT_ during compile time will cause
 * the compiler to revert to the function prototypes for the outdated,
 * obsolete Pthreads API used by DCE. The DCE Pthreads function prototypes
 * are markedly different than the latest Pthreads Draft 7 Spec.
 *
 * This compile time option should _ONLY_ be used when compiling 
 * OSF/DCE for a platform _OR_ to compile legacy code that was written
 * using the DCE Pthreads API.
 *
 * If possible, convert your code to use the Draft 7 interfaces
 * (i.e. /usr/include/pthread.h) and NOT the DCE interfaces. 
 *
 ************************************************************************
 */


#if defined(_DCE_PTHREADS_COMPAT_) && !defined(___INSIDE_EXC_HANDLING_H)

#if defined(_DCE_PTHREADS_COMPAT_MACROS_)
#  if !defined(_PTHREADS_DCE_EXC_MACROS_)
#    error ERROR: Attempted conflicting use of pthread with pthread_exc
#  endif
#else

#define _DCE_PTHREADS_COMPAT_MACROS_
#define _PTHREADS_DCE_EXC_MACROS_

/* Let the world know that it is using the exception based pthreads API. */
#define PTHREAD_EXC

/*
 * Define the necessary POSIX features.
 */


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

#define pthread_attr_create            (pthd4exc_attr_create)
#define pthread_attr_delete            (pthd4exc_attr_delete )
#define pthread_attr_setstacksize      (pthd4exc_attr_setstacksize)
#define pthread_attr_getstacksize      (pthd4exc_attr_getstacksize)
#define pthread_attr_getprio           (pthd4exc_attr_getprio)
#define pthread_attr_setprio           (pthd4exc_attr_setprio)
#define pthread_attr_getsched          (pthd4exc_attr_getsched)
#define pthread_attr_setsched          (pthd4exc_attr_setsched)
#define pthread_attr_getinheritedsched (pthd4exc_attr_getinheritedsched)
#define pthread_attr_setinheritedsched (pthd4exc_attr_setinheritedsched)
#define pthread_attr_getguardsize_np   (pthd4exc_attr_getguardsize_np)
#define pthread_attr_setguardsize_np   (pthd4exc_attr_setguardsize_np)

#undef pthread_mutex_init
#undef pthread_mutex_destroy
#undef pthread_mutex_lock   
#undef pthread_mutex_unlock 
#undef pthread_mutex_trylock
#undef pthread_mutexattr_create
#undef pthread_mutexattr_delete
#undef pthread_mutexattr_getkind_np
#undef pthread_mutexattr_setkind_np

#define pthread_mutex_init             (pthd4exc_mutex_init)
#define pthread_mutex_destroy          (pthd4exc_mutex_destroy)
#define pthread_mutex_lock             (pthd4exc_mutex_lock)
#define pthread_mutex_unlock           (pthd4exc_mutex_unlock)
#define pthread_mutex_trylock          (pthd4exc_mutex_trylock)
#define pthread_mutexattr_create       (pthd4exc_mutexattr_create)
#define pthread_mutexattr_delete       (pthd4exc_mutexattr_delete)
#define pthread_mutexattr_getkind_np   (pthd4exc_mutexattr_getkind_np)
#define pthread_mutexattr_setkind_np   (pthd4exc_mutexattr_setkind_np)

#undef pthread_cond_init
#undef pthread_cond_destroy 
#undef pthread_cond_signal 
#undef pthread_cond_signal_int_np 
#undef pthread_cond_broadcast
#undef pthread_cond_wait   
#undef pthread_cond_timedwait
#undef pthread_condattr_create
#undef pthread_condattr_delete

#define pthread_cond_init              (pthd4exc_cond_init)
#define pthread_cond_destroy           (pthd4exc_cond_destroy)
#define pthread_cond_signal            (pthd4exc_cond_signal)
#define pthread_cond_signal_int_np     (pthd4exc_cond_signal_int_np)
#define pthread_cond_broadcast         (pthd4exc_cond_broadcast)
#define pthread_cond_wait              (pthd4exc_cond_wait)
#define pthread_cond_timedwait         (pthd4exc_cond_timedwait)
#define pthread_condattr_create        (pthd4exc_condattr_create)
#define pthread_condattr_delete        (pthd4exc_condattr_delete)

#undef pthread_lock_global_np         
#undef pthread_unlock_global_np       

#define pthread_lock_global_np         (pthd4exc_lock_global_np)
#define pthread_unlock_global_np       (pthd4exc_unlock_global_np)

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

#define pthread_equal                  (pthd4exc_equal)
#define pthread_create                 (pthd4exc_create)
#define pthread_detach                 (pthd4exc_detach)
#define pthread_exit                   (pthd4exc_exit)
#define pthread_join                   (pthd4exc_join)
#define pthread_self                   (pthd4exc_self)
#define pthread_setspecific            (pthd4exc_setspecific)
#define pthread_getspecific            (pthd4exc_getspecific)
#define pthread_setprio                (pthd4exc_setprio)
#define pthread_getprio                (pthd4exc_getprio)
#define pthread_setscheduler           (pthd4exc_setscheduler)
#define pthread_getscheduler           (pthd4exc_getscheduler)
#define pthread_yield                  (pthd4exc_yield)
#define pthread_once                   (pthd4exc_once)
#define pthread_delay_np               (pthd4exc_delay_np)
#define pthread_keycreate              (pthd4exc_keycreate)
#define pthread_keydelete              (pthd4exc_keydelete)
#define pthread_setcancel              (pthd4exc_setcancel)
#define pthread_setasynccancel         (pthd4exc_setasynccancel)
#define pthread_cancel                 (pthd4exc_cancel)
#define pthread_testcancel             (pthd4exc_testcancel)
#define pthread_get_expiration_np      (pthd4exc_get_expiration_np)
#define pthread_get_unique_np          (pthd4exc_get_unique_np)
#define pthread_getunique_np          (pthd4exc_get_unique_np)  
#define pthread_signal_to_cancel_np    (pthd4exc_signal_to_cancel_np)
#define pthread_is_multithreaded_np    (pthd4_is_multithreaded_np)

#undef  atfork
#define atfork                         (pthd4exc_atfork)

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
#undef pthread_atfork

#define WARNING_CONFLICTING_API_USAGE \
#error ERROR: Attempted conlicting use of DCE vs. POSIX Draft 7 Pthreads API

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
#define pthread_atfork                           WARNING_CONFLICTING_API_USAGE

#undef WARNING_CONFLICTING_API_USAGE

#endif /* _DCE_PTHREADS_COMPAT_MACROS_ */
#endif /* _DCE_PTHREADS_COMPAT_ && ! ___INSIDE_EXC_HANDLING_H */


#ifndef __PTHREAD_DCE_EXC
#define __PTHREAD_DCE_EXC


#ifndef __DCETHREADS_CONF_H
#  include "dcethreads_conf.h"
#endif


__BEGIN_DECLS

/************************************************************************
 *
 * Prototypes for Draft 4 Pthreads and DCE Pthreads
 *
 ************************************************************************/


extern int
pthd4exc_attr_create __P(( pthread_attr_t * attr ));

extern int 
pthd4exc_attr_delete  __P(( pthread_attr_t * attr));
 
extern long
pthd4exc_attr_getstacksize __P(( pthread_attr_t attr ));

extern int
pthd4exc_attr_setstacksize __P(( pthread_attr_t * attr, long stacksize ));

extern int
pthd4exc_attr_getprio __P(( pthread_attr_t attr));

extern int
pthd4exc_attr_setprio __P(( pthread_attr_t * attr, int priority));

extern int
pthd4exc_attr_getsched __P(( pthread_attr_t attr));

extern int
pthd4exc_attr_setsched __P(( pthread_attr_t * attr, int schedval));

extern int
pthd4exc_attr_getinheritedsched __P(( pthread_attr_t attr));

extern int
pthd4exc_attr_setinheritedsched __P(( pthread_attr_t * attr, int schedval));

extern long
pthd4exc_attr_getguardsize_np __P(( pthread_attr_t attr ));

extern int
pthd4exc_attr_setguardsize_np __P(( pthread_attr_t * attr, long size));



  /* 
   * mutex types
   */

extern int 
pthd4exc_mutex_init __P(( pthread_mutex_t * mutex, pthread_mutexattr_t attr ));

extern int 
pthd4exc_mutex_destroy __P(( pthread_mutex_t * mutex ));

extern int
pthd4exc_mutex_lock __P(( pthread_mutex_t * mutex ));

extern int
pthd4exc_mutex_trylock __P(( pthread_mutex_t * mutex ));

extern int
pthd4exc_mutex_unlock __P(( pthread_mutex_t * mutex ));

extern int
pthd4exc_mutexattr_create __P(( pthread_mutexattr_t * mutexattr ));

extern int
pthd4exc_mutexattr_delete __P(( pthread_mutexattr_t * mutexattr ));

extern int
pthd4exc_mutexattr_setkind_np __P(( pthread_mutexattr_t * mutexattr, int kind ));

extern int
pthd4exc_mutexattr_getkind_np __P(( pthread_mutexattr_t  mutexattr ));



/*
 * Condition variables
 */

extern int 
pthd4exc_cond_init __P(( pthread_cond_t * cond, pthread_condattr_t attr ));

extern int 
pthd4exc_cond_destroy __P(( pthread_cond_t * cond ));

extern int
pthd4exc_cond_broadcast __P(( pthread_cond_t * cond ));

extern int
pthd4exc_cond_signal __P(( pthread_cond_t * cond ));

extern int
pthd4exc_cond_signal_int_np __P(( pthread_cond_t * cond ));

extern int
pthd4exc_cond_wait __P(( pthread_cond_t * cond, pthread_mutex_t * mutex ));

extern int
pthd4exc_cond_timedwait __P(( pthread_cond_t * cond,
                      pthread_mutex_t * mutex,
                      struct timespec * abstime ));

extern int 
pthd4exc_condattr_create __P(( pthread_condattr_t * attr ));

extern int 
pthd4exc_condattr_delete __P(( pthread_condattr_t * attr ));


/*
 * MISC DCE 
 */

extern void
pthd4exc_lock_global_np __P(( void )); 

extern void
pthd4exc_unlock_global_np __P(( void )); 

/*
 * Draft 4 core and DCE 
 */


extern int 
pthd4exc_create __P(( pthread_t * th_h,
              pthread_attr_t attr,
              pthread_startroutine_t proc,
              pthread_addr_t arg ));

extern int
pthd4exc_detach __P(( pthread_t * thread ));

extern void
pthd4exc_exit __P(( pthread_addr_t status ));

extern int 
pthd4exc_join __P(( pthread_t thread, pthread_addr_t * status ));

extern int
pthd4exc_equal __P(( pthread_t thd1, pthread_t thd2));

extern pthread_t 
pthd4exc_self __P(( void ));

extern int 
pthd4exc_setspecific __P(( pthread_key_t key, pthread_addr_t value ));

extern int 
pthd4exc_getspecific __P(( pthread_key_t key, pthread_addr_t *value ));

extern int 
pthd4exc_setprio __P(( pthread_t thd, int p));

extern int 
pthd4exc_getprio __P(( pthread_t thd));

extern int 
pthd4exc_setscheduler __P(( pthread_t thread, int scheduler, int priority));

extern int 
pthd4exc_getscheduler __P(( pthread_t thd));

extern void
pthd4exc_yield __P(( void ));

extern pthread_t
pthd4exc_self __P(( void ));

extern int
pthd4exc_once __P(( pthread_once_t *once_block, void (*init_routine)(void) ));

extern int
pthd4exc_delay_np __P(( struct timespec * interval ));

extern int 
pthd4exc_keycreate __P(( pthread_key_t * key, pthread_destructor_t destructor ));

extern int
pthd4exc_setcancel __P(( int state ));

extern int
pthd4exc_setasynccancel __P(( int state ));

extern int 
pthd4exc_cancel __P(( pthread_t thread ));

extern void 
pthd4exc_testcancel __P(( void ));

extern int
pthd4exc_get_expiration_np __P(( struct timespec * delta, struct timespec * abstime ));

extern int 
pthd4exc_getunique_np __P(( pthread_t * handle));

extern int
pthd4exc_signal_to_cancel_np __P(( sigset_t * sig, pthread_t * thd));

extern int
pthd4_is_multithreaded_np __P(( void ));

extern void
pthd4exc_atfork __P(( void * userstate, void (*pre_fork)(void*), 
		 void  (*parent_fork)(void*), void (*child_fork)(void*) ));


__END_DECLS


#endif
