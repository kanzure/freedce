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

/************************************************************************
 *
 * Prototypes for Draft 4 Pthreads and DCE Pthreads
 *
 ************************************************************************/

/****************************************************************************
 *
 * pthread_dce_proto.h
 *
 * Prototypes for POSIX Threads Draft 4 
 *
 * Implementation for LINUX Threads
 *
 ****************************************************************************
 */


#ifndef _POSIX_THREADS_DRAFT4_PROTOTYPES
#define _POSIX_THREADS_DRAFT4_PROTOTYPES

#ifdef __cplusplus
    extern "C" {
#endif

/*
 * Attribute types
 */

extern int
pthd4_attr_create( pthread_attr_t * attr );

extern int 
pthd4_attr_delete ( pthread_attr_t * attr);
 
extern long
pthd4_attr_getstacksize( pthread_attr_t attr );

extern int
pthd4_attr_setstacksize( pthread_attr_t * attr, long stacksize );

extern int
pthd4_attr_getprio( pthread_attr_t attr);

extern int
pthd4_attr_setprio( pthread_attr_t * attr, int priority);

extern int
pthd4_attr_getsched( pthread_attr_t attr);

extern int
pthd4_attr_setsched( pthread_attr_t * attr, int schedval);

extern int
pthd4_attr_getinheritedsched( pthread_attr_t attr);

extern int
pthd4_attr_setinheritedsched( pthread_attr_t * attr, int schedval);

extern long
pthd4_attr_getguardsize_np( pthread_attr_t attr );

extern int
pthd4_attr_setguardsize_np( pthread_attr_t * attr, long size);



  /* 
   * mutex types
   */

extern int 
pthd4_mutex_init( pthread_mutex_t * mutex, pthread_mutexattr_t attr );

extern int 
pthd4_mutex_destroy( pthread_mutex_t * mutex );

extern int
pthd4_mutex_lock( pthread_mutex_t * mutex );

extern int
pthd4_mutex_trylock( pthread_mutex_t * mutex );

extern int
pthd4_mutex_unlock( pthread_mutex_t * mutex );

extern int
pthd4_mutexattr_create( pthread_mutexattr_t * mutexattr );

extern int
pthd4_mutexattr_delete( pthread_mutexattr_t * mutexattr );

extern int
pthd4_mutexattr_setkind_np( pthread_mutexattr_t * mutexattr, int kind );

extern int
pthd4_mutexattr_getkind_np( pthread_mutexattr_t  mutexattr );



/*
 * Condition variables
 */

extern int 
pthd4_cond_init( pthread_cond_t * cond, pthread_condattr_t attr );

extern int 
pthd4_cond_destroy( pthread_cond_t * cond );

extern int
pthd4_cond_broadcast( pthread_cond_t * cond );

extern int
pthd4_cond_signal( pthread_cond_t * cond );

extern int
pthd4_cond_signal_int_np( pthread_cond_t * cond );

extern int
pthd4_cond_wait( pthread_cond_t * cond, pthread_mutex_t * mutex );

extern int
pthd4_cond_timedwait( pthread_cond_t * cond,
                      pthread_mutex_t * mutex,
                      struct timespec * abstime );

extern int 
pthd4_condattr_create( pthread_condattr_t * attr );

extern int 
pthd4_condattr_delete( pthread_condattr_t * attr );


/*
 * MISC DCE 
 */

extern void 
pthd4_lock_global_np( void ); 

extern void
pthd4_unlock_global_np( void ); 

/*
 * Draft 4 core and DCE 
 */


extern int 
pthd4_create( pthread_t * th_h,
              pthread_attr_t attr,
              pthread_startroutine_t proc,
              pthread_addr_t arg );

extern int
pthd4_detach( pthread_t * thread );

extern void
pthd4_exit( pthread_addr_t status );

extern int 
pthd4_join( pthread_t thread, pthread_addr_t * status );

extern int
pthd4_equal( pthread_t thd1, pthread_t thd2);

extern pthread_t 
pthd4_self( void );

extern int 
pthd4_setspecific( pthread_key_t key, pthread_addr_t value );

extern int 
pthd4_getspecific( pthread_key_t key, pthread_addr_t *value );

extern int 
pthd4_setprio( pthread_t thd, int p);

extern int 
pthd4_getprio( pthread_t thd);

extern int 
pthd4_setscheduler( pthread_t thread, int scheduler, int priority);

extern int 
pthd4_getscheduler( pthread_t thd);

extern void
pthd4_yield( void );

extern pthread_t
pthd4_self( void );

extern int
pthd4_once( pthread_once_t *once_block, void (*init_routine)(void) );

extern int
pthd4_delay_np( struct timespec * interval );

extern int 
pthd4_keycreate( pthread_key_t * key, pthread_destructor_t destructor );

extern int
pthd4_setcancel( int state );

extern int
pthd4_setasynccancel( int state );

extern int 
pthd4_cancel( pthread_t thread );

extern void 
pthd4_testcancel( void );

extern int
pthd4_get_expiration_np( struct timespec * delta, struct timespec * abstime );

extern int 
pthd4_getunique_np( pthread_t * handle);

extern int
pthd4_signal_to_cancel_np( sigset_t * sig, pthread_t * thd);

extern int
pthd4_is_multithreaded_np( void );

extern void
pthd4_atfork( void * userstate, void (*pre_fork)(void), 
	      void (*parent_fork)(void), void (*child_fork)(void) );


extern void *
pthd4__cancel_thread(void);


#ifdef __cplusplus
}
#endif

#endif

