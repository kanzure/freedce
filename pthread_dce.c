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


#ifndef lint
static const char rcsid[] = "$Id: pthread_dce.c,v 1.1 2000/08/13 02:02:40 wez Exp $";
#endif

/****************************************************************************
 *
 * pthread 4 emulation over linux threads
 *
 ****************************************************************************/

/*
 * NOT IMPLEMENTED HERE FIXME
 *
 * void pthread_cleanup_pop(int execute)
 *   return: NO this routine must be used as a statement
 *
 * void pthread_cleanup_push(void routine, pthread_addr_t arg)
 *   return: NO this routine must be used as a statement
 */
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>

#include <pthread_dce_common.h>    /* Import common D4/D7 overlays */
#include <pthread_dce.h>           /* Import DCE Threads */


/*
 * Pthreads functions return 0 on success
 */

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILURE
#define FAILURE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

# define PTHREAD_TEMP_FAILURE_RETRY(expression) \
(__extension__ \
({ long int __result; \
do __result = (long int) (expression); \
while (__result == EINTR); \
__result; }))
	
/*
 * Built-in versioning information for the binary
 */

char dce_pthreads_adapter_version[] = 
        "dce_pthreads_adapter_version: 1.6";
char dce_pthreads_adapter_linuxthreads_version[] = 
        "dce_pthreads_adapter_linuxthreads_version: 0.8";
char dce_pthreads_adapter_linuxkernel_version[] = 
        "dce_pthreads_adapter_linuxkernel_version: 2.2.10";

pthread_attr_t               pthread4_attr_default;
pthread_mutexattr_t          pthread4_mutexattr_default;
pthread_condattr_t           pthread4_condattr_default;
const pthread_once_t         pthread4_once_init = PTHREAD_ONCE_INIT;
int linuxdce_threadslib_multithreaded = 1;

pthread_once_t defaults_initialized = pthread_once_init;


/* 
 * Support for pthread_signal_to_cancel_np 
 */

int             pthd4__g_handle_active = FALSE;
pthread_t       pthd4__g_handle_thread, pthd4__g_handle_target;
sigset_t        pthd4__g_handle_sigset;        

/* 
 * DCE Library global lock, if needed 
 */

pthread_mutex_t              pthd4__g_global_lock;

/*
 * Library one-time initialization routine
 */

static void 
pthd4_lib_init(void)
{
  linuxdce_threadslib_multithreaded = 1;
  pthread_attr_init( &pthread4_attr_default );
  pthread_mutexattr_init (&pthread4_mutexattr_default);
  pthread_condattr_init (&pthread4_condattr_default);
  pthread_mutex_init(&pthd4__g_global_lock, &pthread_mutexattr_default);
}

/******************************************************************************
 * pthd4_xxx() routines implement the DCE Threads API ontop Linux Pthreads
 *****************************************************************************/

/*
 * pthread_attr_create:
 *   return:  0 - successfull
 *           -1/ENOMEM Insufficient memory exists to create object
 *           -1/EINVAL Invalid value for attr
 */
int
pthd4_attr_create(pthread_attr_t *attr)
{
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }
    
    /* pthread_attr_init can't fail */
    istat = pthread_attr_init(attr);
    if (istat != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }
                           
    /*
     * Draft 4/DCE threads are created as joinable threads.  POSIX threads
     * are not.
     */
    istat = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }

    return (SUCCESS);
}

/*
 * pthread_attr_delete:
 *   return:  0 - successfull
 *           -1/EINVAL Invalid value for attr
 */
int 
pthd4_attr_delete(pthread_attr_t *attr)
{
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    /* XXX - This is currently a NOP in LinuxThreads 0.71 */
    if (pthread_attr_destroy(attr) != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }

    return (SUCCESS);
}

/****************************************************************************
 * pthd4 pthread_attr_setstacksize()
 * pthd4 pthread_attr_getstacksize()
 *
 * On LinuxThreads 0.7, you cannot set a threads stacksize. Rather
 * kernel thread stacks are grown on demand. Pretend that we can
 * set any stacksize.
 * Always return the initial stack size allocated to a kernel thread,
 * 
 ****************************************************************************/

/*
 * pthread_attr_setstacksize:
 *   return:  0 - successful
 *           -1/EINVAL - invalid attribute
 */
int 
pthd4_attr_setstacksize(pthread_attr_t *attr __attribute__((unused)),
			long stacksize __attribute__((unused)))
{
    return (SUCCESS);
}

/*
 * pthread_attr_getstacksize:
 *   return:  value
 *           -1/EINVAL - invalid attribute
 */
long
pthd4_attr_getstacksize(pthread_attr_t attr __attribute__((unused)))
{
    return ((long )(4 * 1096));
}

/*
 * pthread_create:
 *   return:  0 - successful
 *           -1/EAGAIN - the system lacks the necessary resources to create object
 *           -1/ENOMEM - insufficient memory exists to create object
 */
int
pthd4_create(pthread_t *th_h,
              pthread_attr_t attr,
              pthread_startroutine_t proc,
              pthread_addr_t arg)
{
    int istat = 0;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_create(th_h,
                           &attr,
                           (pthread_startroutine_t) proc,
                           arg);

#if defined(YIELD_AFTER_PTHREAD_CREATE)
    sched_yield();
#endif

    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EAGAIN:
        errno = EAGAIN;
        return (FAILURE);
        break;
    default:
        /*
         * It is currently unclear what the set of error codes LinuxThreads
         * 0.71 can return here. This is a catch-all for now. DCE documents
         * that it only expects ENOMEM or EAGAIN. Hopefully the code matches
         * the docs.
         */
        errno = ENOMEM;
        return (FAILURE);
    }    
}

/*
 * pthread_detach:
 *   return:  0 - successful
 *           -1/EINVAL - value for thread is invalid
 *           -1/ESRCH  - thread does not match any existing thread
 */
int 
pthd4_detach(pthread_t *thread)
{
    int istat = 0;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init ) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_detach(*thread);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    case EINVAL:
        errno = EINVAL;
        return (FAILURE);
        break;
    default:
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_exit:
 *   return: NO
 */
void
pthd4_exit(pthread_addr_t status)
{
    (void )pthread_once(&defaults_initialized, pthd4_lib_init);
    pthread_exit((void *)status);
}

/*
 * pthread_join:
 *   return:  0 - successful
 *           -1/EINVAL - invalid value for thread
 *           -1/ESRCH  - thread does not match any existing thread
 *           -1/EDEADLK - a deadlock detected
 */
int 
pthd4_join(pthread_t thread, pthread_addr_t *status)
{
    int istat = 0;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    istat = pthread_join(thread, (void **)status);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    case EINVAL:
        errno = EINVAL;
        return (FAILURE);
        break;
    case EDEADLK:
        errno = EDEADLK;
        return (FAILURE);
        break;
    default:
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_mutexattr_create:
 *   return:  0 - successful
 *           -1/EINVAL - invalid value for attribute
 *           -1/ENOMEM - insufficient memory to create object
 */
int
pthd4_mutexattr_create(pthread_mutexattr_t *attr)
{
    int istat;
    
    /* pthread_mutexattr_init can't fail */
    istat = pthread_mutexattr_init(attr);
    if (istat != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }    

    return (SUCCESS);
}

/*
 * pthread_mutexattr_delete:
 *   return:  0 - successful
 *           -1/EINVAL - invalid value for attribute
 */
int 
pthd4_mutexattr_delete(pthread_mutexattr_t *attr)
{
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    /* XXX - This is currently a NOP in LinuxThreads 0.71 */
    if (pthread_mutexattr_destroy(attr) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    return (SUCCESS);
}

/*
 * pthread_mutex_init:
 *   return:  0 - successful
 *           -1/EAGAIN - the system lacks the necsssary resources
 *           -1/ENOMEM - insufficient memory to initialize
 */
int 
pthd4_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t attr)
{
    int istat = 0;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    /* pthread_mutex_init can't fail */
    istat = pthread_mutex_init(mutex, &attr);
    if (istat != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }    
    
    return (SUCCESS);
}

/*
 * pthread_mutex_destroy:
 *   return:  0 - successful
 *           -1/EBUSY - mutex already locked
 *           -1/EINVAL - invalid value for mutex
 */
int 
pthd4_mutex_destroy(pthread_mutex_t *mutex)
{
    int istat = 0;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    /*
     * XXX - LinuxThreads 0.71 doesn't seem to care if the mutex is invalid 
     * but draft 4 lists EINVAL as a return code if the mutex is invalid. 
     */
    istat = pthread_mutex_destroy(mutex);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EBUSY:
        errno = EBUSY;
        return (FAILURE);
        break;
    default:
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_mutex_lock:
 *   return:  0 - successfull
 *           -1/EINVAL - invalid value for mutex
 *           -1/EDEADLK - deadlock condition detected
 */
int
pthd4_mutex_lock(pthread_mutex_t *mutex)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        

    istat = pthread_mutex_lock(mutex);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EINVAL:
        errno = EINVAL;
        return (FAILURE);
        break;
    case EDEADLK:
        errno = EDEADLK;
        return (FAILURE);
        break;
    default:
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_mutex_trylock:
 *   return:  1 - successful
 *            0 - already locked, lock not aquired
 *           -1/EINVAL - invalid value for mutex
 */
int 
pthd4_mutex_trylock(pthread_mutex_t *mutex)
{
    int istat = 0;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init ) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    istat = pthread_mutex_trylock(mutex);
    switch (istat) {
    case SUCCESS:
        return (1);             /* The mutex is now locked and owned by */
        break;                  /* the calling thread.                  */
    case EBUSY:
        return (0);             /* The mutex was locked */
        break;
    case EINVAL:
        errno = EINVAL;
        return (FAILURE);
        break;
    default:
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_mutex_unlock:
 *   return:  0 - succesful
 *           -1/EINVAL - invalid value for mutex
 */
int 
pthd4_mutex_unlock(pthread_mutex_t *mutex)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    istat = pthread_mutex_unlock(mutex);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EINVAL:
    case EPERM:
        errno = EINVAL;
        return (FAILURE);
        break;
    default:
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_condattr_create:
 *   return:  0 - successful
 *           -1/EINVAL - invalid attribute
 *           -1/ENOMEM - insufficient memory exists to create object
 */
int
pthd4_condattr_create(pthread_condattr_t *attr)
{
    int istat;
    
    /* XXX - This is currently a NOP in LinuxThreads 0.71 */
    istat = pthread_condattr_init(attr);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return(FAILURE);
    }

    return (SUCCESS);
}

/*
 * pthread_condattr_delete:
 *   return:  0 - successful
 *           -1/EINVAL - invalid attribute
 */
int 
pthd4_condattr_delete(pthread_condattr_t *attr)
{
    int istat;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        

    /* XXX - This is currently a NOP in LinuxThreads 0.71 */
    istat = pthread_condattr_destroy(attr);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return(FAILURE);
    }

    return (SUCCESS);
}


/*
 * pthread_cond_init:
 *   return:  0 - successful
 *           -1/EAGAIN - the system lacks theb necessary resources
 *           -1/ENOMEM - insufficient memory to initialize
 */
int 
pthd4_cond_init(pthread_cond_t *cond, pthread_condattr_t attr)
{
    int istat = 0;
    
    if (pthread_once( &defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        

    /* pthread_cond_init can't fail */
    istat = pthread_cond_init(cond, &attr);
    if (istat != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }    

    return (SUCCESS);
}

/*
 * pthread_cond_destroy:
 *   return:  0 - successful
 *           -1/EINVAL - invalid condition
 *           -1/EBUSY  - a thread is currently waiting on condition
 */
int 
pthd4_cond_destroy(pthread_cond_t *cond)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        

    istat = pthread_cond_destroy(cond);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EBUSY:
        errno = EBUSY;
        return (FAILURE);
        break;
    default:
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_cond_broadcast:
 *   return:  0 - successful
 *           -1/EINVAL - invalid condition
 */
int 
pthd4_cond_broadcast(pthread_cond_t *cond)
{
    int istat = 0;
    
    if (pthread_once( &defaults_initialized, pthd4_lib_init ) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    /* pthread_cond_broadcast can't fail */
    istat = pthread_cond_broadcast(cond);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (SUCCESS);
}

/*
 * pthread_cond_signal:
 *   return:  0 - successful
 *           -1/EINVAL - invalid condition
 */
int 
pthd4_cond_signal(pthread_cond_t *cond)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        
    
    /* pthread_cond_signal can't fail */
    istat = pthread_cond_signal(cond);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (SUCCESS);
}

/*
 * pthread_cond_wait:
 *   return:  0 - successful
 *           -1/EINVAL - value for parameters are invalid
 *           -1/EDEADLK - dedlock condition detected
 */
int 
pthd4_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }        

    /* pthread_cond_wait can't fail, although it may call pthread_exit() */
    istat = pthread_cond_wait(cond, mutex);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (SUCCESS);
}

/*
 * pthread_cond_timedwait:
 *   return:  0 - successful
 *           -1/EINVAL - value for parameters are invalid
 *           -1/EAGAIN - time expired
 *           -1/EDEADLK - dedlock condition detected
 */
int 
pthd4_cond_timedwait(pthread_cond_t *cond,
                     pthread_mutex_t *mutex,
                     struct timespec *abstime)
{
    int istat = 0;
    
    if (pthread_once( &defaults_initialized, pthd4_lib_init ) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = PTHREAD_TEMP_FAILURE_RETRY(pthread_cond_timedwait(cond, mutex, abstime));
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case ETIMEDOUT:
        errno = EAGAIN;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_once:
 *   return  0 - successful
 *          -1/? - invalid parameter
 *
 *   The prototype in documentation is:
 *  int pthread_once(pthread_once_t*, pthread_initroutine_t); FIXME
 */
int 
pthd4_once(pthread_once_t *once_block, void (*init_routine)(void))
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }
    
    /* pthread_once can't fail */
    istat = pthread_once(once_block, init_routine);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (SUCCESS);
}

/*
 * pthread_keycreate:
 *   return:  0 - successful
 *           -1/EINVAL - invalic value for key
 *           -1/ENOMEM - key namespace exhausted
 *           -1/EAGAIN - insufficient memory exist for create a key
 */
int 
pthd4_keycreate(pthread_key_t *key, pthread_destructor_t destructor)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_key_create(key, destructor);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EAGAIN:
        errno = EAGAIN;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_setspecific:
 *   return:  0 - successfull
 *           -1/EINVAL - invalid value for key
 */
int 
pthd4_setspecific(pthread_key_t key, pthread_addr_t value)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_setspecific(key, value);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case ENOMEM:
    case EINVAL:
        errno = EINVAL;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_getspecific:
 *   return:  0 - successfull
 *           -1/EINVAL - invalid value for key
 */
int 
pthd4_getspecific(pthread_key_t key, pthread_addr_t *value)
{

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    *value = pthread_getspecific(key);
    if (value == NULL) {
        errno = EINVAL;
        return (FAILURE);
    }

    return (SUCCESS);
}

/*
 * Post a cancel to a thread.  This does not directly cancel the thread.
 * Once posted, the target thread will cancel itself when both of the
 * following conditions are true:
 *
 *  1)  The target thread reaches a cancellation point or calls
 *      pthread_testcancel()
 *
 *  and
 *
 *  2) The value of the target thread's cancel state is CANCEL_ON.
 */

/*
 * pthread__cancel:
 *   return:  0 - successful
 *           -1/EINVAL - specified thread is invalid
 *           -1/ERSCH  - specified thread does not refer to a currently existing thread
 */
int 
pthd4_cancel(pthread_t thread)
{
    int istat = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_cancel(thread);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * Enables (CANCEL_ON) or disables (CANCEL_OFF) asynchronous cancellation.
 * For a thread to be asynchronously canceled, asynchronous cancellation must
 * be enabled *AND* the thread must have been requested to cancel, i.e., its
 * cancel pending flag must be TRUE.
 */

/*
 * pthread_setasynccancel:
 *   return  CANCEL_ON  - asynchronous cancelability was on
 *           CANCEL_OFF - asynchronous cancelability was off
 *          -1/EINVAL - invalid value
 */
int 
pthd4_setasynccancel(int state)
{
    int istat = 0;
    int old_type = 0;
    int new_type = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    if (state == CANCEL_OFF) {
        new_type = PTHREAD_CANCEL_DEFERRED;
    }
    else if (state == CANCEL_ON) {
        new_type = PTHREAD_CANCEL_ASYNCHRONOUS;
    }
    else {
        errno = EINVAL;
        return (FAILURE);
    }

    istat = pthread_setcanceltype(new_type, &old_type);
    switch (istat) {
    case SUCCESS:
        return (old_type);
        break;
    case EINVAL:
        errno = EINVAL;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_setcancel:
 *   return  CANCEL_ON  - Cancelability was on
 *           CANCEL_OFF - Cancelability was off
 *          -1/EINVAL - invalid value
 */
int 
pthd4_setcancel(int state)
{
    int istat = 0;
    int prev_state = 0;
    int new_state = 0;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    if (state == CANCEL_OFF) {
        new_state = PTHREAD_CANCEL_DISABLE;
    }
    else {
        new_state = PTHREAD_CANCEL_ENABLE;
    }

    istat = pthread_setcancelstate(new_state, &prev_state);
    switch (istat) {
    case SUCCESS:
        return (prev_state);
        break;
    case EINVAL:
        errno = EINVAL;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/***************************************************************************
 *
 * additional draft 4/DCE functions added by jrd@bu.edu
 * These need to be implemented for linux threads
 *
 ***************************************************************************/

/****************************************************************************
 * pthd4 pthread_attr_setprio():
 * pthd4 pthread_attr_getprio():
 * pthd4 pthread_attr_setsched():
 * pthd4 pthread_attr_getsched():
 *
 * These DCE Threads functions can be emulated on Draft 7 Pthreads using
 * the pthread_attr_setschedparam()/pthread_attr_getschedparam(),
 * pthread_attr_setschedpolicy()/pthread_attr_getschedpolicy(),
 * interface.
 *
 ****************************************************************************/

/*
 * pthread_attr_setprio:
 *   return  0 - successful
 *          -1/EINVAL - invalid attribute
 *          -1/ERANGE - one or more parameter have an invalid value
 */
int
pthd4_attr_setprio(pthread_attr_t *attr, int priority)
{
    struct sched_param my_schedparam;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    my_schedparam.sched_priority = priority;

    istat = pthread_attr_setschedparam(attr, &my_schedparam);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EINVAL:
        errno = ERANGE;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_attr_getprio:
 *   return:  value
 *           -1/EINVAL - invalid attribute
 */
int
pthd4_attr_getprio(pthread_attr_t attr)
{
    struct sched_param my_schedparam;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    /* pthread_sttr_getschedparam can't fail */
    istat = pthread_attr_getschedparam(&attr, &my_schedparam);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (my_schedparam.sched_priority);
}

/*
 * pthread_attr_setsched:
 *   return:  0 - successful
 *           -1/EINVAL - invalid attribute
 *           -1/EPERM  - caller has no permission
 *
 *   The prototype in documentation is:
 *   int pthread_attr_setsched(pthread_attr_t*, int*); FIXME
 */
int
pthd4_attr_setsched(pthread_attr_t *attr, int sched)
{
    int istat;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_attr_setschedpolicy(attr, sched);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EINVAL:
        errno = ERANGE;
        return (FAILURE);
        break;
    case ENOTSUP:
        errno = EPERM;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

/*
 * pthread_attr_getsched
 *   return:  value
 *           -1/EINVAL - invalid attribute
 */
int
pthd4_attr_getsched(pthread_attr_t attr)
{
    int sched;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    /* pthread_attr_getschedpolicy can't fail */
    istat = pthread_attr_getschedpolicy(&attr, &sched);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (SUCCESS);
}

/*
 * pthread_attr_setinheritedsched:
 *   return:  0 - successful
 *           -1/EINVAL - invalid attribute
 */
int
pthd4_attr_setinheritedsched(pthread_attr_t * attr, int sched)
{
    int old_sched;
    int istat;

    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    /* pthread_attr_getinheritsched can't fail */
    istat = pthread_attr_getinheritsched(attr, &old_sched);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    
    
    istat = pthread_attr_setinheritsched(attr, sched);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    
    
    return (old_sched);
}

/*
 * pthread_attr_getinheritedsched:
 *   return  value
 *          -1/EINVAL - invalid attribute
 */
int
pthd4_attr_getinheritedsched(pthread_attr_t attr)
{
    int sched;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    /* pthread_attr_getinheritsched can't fail */
    istat = pthread_attr_getinheritsched(&attr, &sched);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (sched);
}

/****************************************************************************
 *
 * pthd4  pthread_attr_setguardsize_np():
 * pthd4  pthread_attr_getguardsize_np():
 *
 * Platform Threads Library dependent.
 * LinuxThreads are based on kernel-threads. User-space schedulers tend
 * to have options to allow setting the inter-thread stack guard size.
 * For Linux threads, we just behave like nothing happens.
 * 
 ****************************************************************************/

int
pthd4_attr_setguardsize_np(pthread_attr_t *attr __attribute__((unused)),
			   long size __attribute__((unused)))
{
    return (SUCCESS);
}

long
pthd4_attr_getguardsize_np(pthread_attr_t attr __attribute__((unused)))
{
    return (2 * 4096);
}

/****************************************************************************
 *
 * pthd4  pthread_attr_setkind_np():
 * pthd4  pthread_attr_getkind_np():
 *
 * Platform Threads Library dependent.
 * LinuxThreads 0.7 seems to already support this non-portable, non-standard
 * interface. Other threads implemenations are likely not to. In the case
 * of LinuxThreads, we can simply use the provided interface. For other
 * Draft7 Pthreads, I am not quite sure what you'd do here.
 * 
 ****************************************************************************/

/*
 * pthread_mutexattr_setkind_np:
 *   return:  0 - successful
 *           -1/EINVAL - invalid value for attribute
 *           -1/EPERM  - caller has no permission
 *           -1/ERANGE - one or more parameters out of range
 */
int pthread_mutexattr_setkind_np (pthread_mutexattr_t*, int);
int
pthd4_mutexattr_setkind_np(pthread_mutexattr_t *mutex_attr, int kind)
{
    return (pthread_mutexattr_setkind_np(mutex_attr, kind));
}

/*
 * pthread_mutexattr_getkind_np:
 *   return:  value - successful
 *           -1/EINVAL - invalid value for attribue
 *
 *   The prototype in documentation is:
 *  int pthread_mutexattr_getkind_np(pthread_mutexattr_t*); FIXME
 */
 
int pthread_mutexattr_getkind_np (const pthread_mutexattr_t*, int*);
int
pthd4_mutexattr_getkind_np(pthread_mutexattr_t mutex_attr)
{
    int kind;
    int istat;

    /* pthread_mutexattr_getkind_np can't fail */
    istat = pthread_mutexattr_getkind_np(&mutex_attr, &kind);
    if (istat != SUCCESS) {
        errno = EINVAL;
        return (FAILURE);
    }    

    return (kind);
}


/****************************************************************************
 *
 * pthd4  pthread_cond_signal_int_np():
 * 
 ****************************************************************************/

int
pthd4_cond_signal_int_np(pthread_cond_t * cond __attribute__((__unused__)))
{
    return (SUCCESS);
}

/****************************************************************************
 * pthd4 pthread_getprio():
 * pthd4 pthread_setprio():
 * pthd4 pthread_setsched():
 * pthd4 pthread_getsched():
 *
 * The DCE Threads functions can be emulated on Draft 7 Pthreads using
 * the pthread_setschedparam()/pthread_getschedparam() interface.
 *
 ****************************************************************************/

/*
 * pthread_setprio:
 *   return:  prevvalue - successful
 *           -1/EINVAL  - invalid value for thread
 *           -1/ENOTSUP - not supported policy
 *           -1/ESRCH   - thread does not exist
 *           -1/ENOPERM - caller has no permission
 */
int
pthd4_setprio(pthread_t thread, int priority)
{
    /*
     * We are only asking to change the running priority. First we
     * get the current scheduling parameters of the running thread.
     * Then we tweak the priority, and reset the sched params
     */
    
    struct sched_param my_schedparam;
    int old_priority;
    int my_policy;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    /* pthread_getschedparam can't fail */
    istat = pthread_getschedparam(thread, &my_policy, &my_schedparam);
    switch (istat) {
    case SUCCESS:
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    default: 
        /*
         * XXX - LinuxThreads can return any errno from pthread_getschedparam().
         * The following isn't consistent with Draft 4, so we may need
         * squash this to an EINVAL.
         */
        errno = istat;
        return (FAILURE);
    }
    
    old_priority = my_schedparam.sched_priority;
    my_schedparam.sched_priority = priority;

    istat = pthread_setschedparam(thread, my_policy, &my_schedparam);
    switch (istat) {
    case SUCCESS:
        return (old_priority);
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    default: 
        /*
         * XXX - LinuxThreads can return any errno from pthread_setschedparam().
         * The following isn't consistent with Draft 4, so we may need
         * squash this to an EINVAL.
         */
        errno = istat;
        return (FAILURE);
    }
}

/*
 * pthread_getprio:
 *   return:  value
 *           -1/EINVAL - value for thread invalid
 *           -1/ESRCH  - thread does not match any existing thread
 */
int
pthd4_getprio(pthread_t thread)
{
    struct sched_param my_schedparam;
    int my_policy;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_getschedparam(thread, &my_policy, &my_schedparam);
    switch (istat) {
    case SUCCESS:
        return (my_schedparam.sched_priority);
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    default: 
        /*
         * XXX - LinuxThreads can return any errno from pthread_getschedparam().
         * The following isn't consistent with Draft 4, so we may need
         * squash this to an EINVAL.
         */
        errno = istat;
        return (FAILURE);
    }
}

/*
 * pthread_setscheduler:
 *   return:  0 - successful
 *           -1/EINVAL  - invlaid value fpor thread
 *           -1/ENOTSUP - unsupported policy
 *           -1/ESRCH   - thread does not exist
 *           -1/ENOPERM - caller has no permission
 */
int
pthd4_setscheduler(pthread_t thread, int scheduler, int priority)
{
    struct sched_param my_schedparam;
    int previous_priority;
    int my_policy;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_getschedparam(thread, &my_policy, &my_schedparam);
    switch (istat) {
    case SUCCESS:
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    default: 
        /*
         * XXX - LinuxThreads can return any errno from pthread_getschedparam().
         * The following isn't consistent with Draft 4, so we may need
         * squash this to an EINVAL.
         */
        errno = istat;
        return (FAILURE);
    }
    
    previous_priority = my_schedparam.sched_priority;
    my_schedparam.sched_priority = priority;
    my_policy = scheduler;
    
    istat = pthread_setschedparam(thread, my_policy, &my_schedparam);
    switch (istat) {
    case SUCCESS:
        return (previous_priority);
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    default: 
        /*
         * XXX - LinuxThreads can return any errno from pthread_setschedparam().
         * The following isn't consistent with Draft 4, so we may need
         * squash this to an EINVAL.
         */
        errno = istat;
        return (FAILURE);
    }
}

/*
 * pthread_getscheduler:
 *   return:  value
 *           -1/EINVAL - invalid value for thread
 *           -1/ESRCH  - thread does not match any existing thread
 */
int
pthd4_getscheduler(pthread_t thread)
{
    struct sched_param my_schedparam;
    int my_policy;
    int istat;
    
    if (pthread_once(&defaults_initialized, pthd4_lib_init) != SUCCESS) {
        errno = ENOMEM;
        return (FAILURE);
    }

    istat = pthread_getschedparam(thread, &my_policy, &my_schedparam);
    switch (istat) {
    case SUCCESS:
        return (my_policy);
        break;
    case ESRCH:
        errno = ESRCH;
        return (FAILURE);
        break;
    default: 
        /*
         * XXX - LinuxThreads can return any errno from pthread_getschedparam().
         * The following isn't consistent with Draft 4, so we may need
         * squash this to an EINVAL.
         */
        errno = istat;
        return (FAILURE);
    }
}


/*
 * pthread_testcancel:
 *   return: NONE
 */
void
pthd4_testcancel(void)
{
    pthread_testcancel();
}

int
pthd4_equal(pthread_t thd1, pthread_t thd2)
{
    return (pthread_equal(thd1, thd2));
}

/*
 * pthread_yield:
 *   return: NONE
 */
void 
pthd4_yield(void)
{
    /*
     * Linux Threads 0.7 supports "sched_yield", use it.
     */
    sched_yield();
}

/*
 * pthread_self:
 *   return: NONE
 */
pthread_t
pthd4_self(void)
{
    return (pthread_self());
}

/****************************************************************************
 * pthd4 pthread_delay_np()
 *
 * Another lovely DCEism. Linux has nanosleep(), so we can use it. 
 * 
 ****************************************************************************/

/*
 * pthread_delay_np:
 *   return:  0 - successful
 *           -1/EINVAL - invalid delay
 */
int
pthd4_delay_np(struct timespec *delay)
{
    int res;
    struct timespec remainder, request;
    
    request.tv_sec = delay->tv_sec;
    request.tv_nsec = delay->tv_nsec;
    
    res = nanosleep(&request, &remainder);
    
    /*
     * if we get interrupted, for some reason, then resume
     * until we deplete the remainder to zero.
     */
    
    while ((res == FAILURE) && (errno == EINTR)) {
        request.tv_sec = remainder.tv_sec;
        request.tv_nsec = remainder.tv_nsec;
        res = nanosleep(&request, &remainder);
    }
    
    if (res == SUCCESS) {
        return (SUCCESS);
    }
    else {
      errno = EINVAL;
      return (FAILURE);
    }
}

/****************************************************************************
 * pthd4 pthread_getexpiration_np()
 *
 * Another lovely DCEism. Do some dumb arithmetic while managing
 * carry and overflows on certain fields.
 * 
 * delta [in]:   number of seconds/nsecs add to current system time.
 * abstime [out]: value representing absolute value of target time.
 * 
 ****************************************************************************/

#define NANOSECS_PER_SEC 1000000000

/*
 * pthd4_get_expiration_np
 *   return:  0 - successful
 *           -1/EINVAL - invalid delta
 */
int 
pthd4_get_expiration_np(struct timespec *delta, struct timespec *abstime)
{
    struct timeval _now;
    struct timespec now;
    
    gettimeofday(&_now, NULL);
    
    now.tv_sec = _now.tv_sec;
    now.tv_nsec = _now.tv_usec * 1000;      /* microseconds -> nanoseconds */
    
    abstime->tv_sec = delta->tv_sec + now.tv_sec;
    abstime->tv_nsec = delta->tv_nsec + now.tv_nsec;
    
    /* adjust overflow nsecs to secs */
    
    abstime->tv_sec += abstime->tv_nsec / NANOSECS_PER_SEC;
    abstime->tv_nsec = abstime->tv_nsec % NANOSECS_PER_SEC;
    
    return (SUCCESS);
}

/****************************************************************************
 * pthd4 pthread_getunique_np()
 *
 * DCE Threads have unique thread ID's.
 * For now, (LinuxThread 0.7, Kernel 2.0.32), we can rely on an
 * 'implementation deficit' of threads:  Each thread has a separate
 * UNIX PID. Use the UNIX PID as a thread ID.
 *
 * WARNING: This will change in Linux 2.1 or 2.2  (!)(!)(!)
 * It is a portability hazard.
 * 
 ****************************************************************************/

int
pthd4_getunique_np(pthread_t *thread __attribute__((__unused__)))
{
	return pthread_self();
    /* return getpid(); */
}

/****************************************************************************
 * pthd4 pthread_lock_global_np()
 *       pthread_unlock_global_np()
 *
 * DCE Threads were user-space threads that included thread-safe
 * wrappers for blocking system calls and non-reentrant library functions.
 * The 'global lock' was used to acquire a runtime-library wide lock
 * for interface to non-threadsafe code.
 *
 * Since Linux now uses GLIBC, we need to proxy calls from these older
 * DCE interfaces to the equivalent monolithic lock used in the threaded
 * GLIBC implementation.
 * 
 * 
 ****************************************************************************/

/****************************************************************************
 * pthd4 pthread_atfork()
 *
 * Fork handler
 *
 ****************************************************************************/


/*
 * atfork:
 *  return: nothing
 *  exception is raised if there is not enough space
 */
void
pthd4_atfork(void *userstate __attribute__((__unused__)),
             void (* pre_fork)(void), 
             void (* parent_fork)(void), 
             void (* child_fork)(void))
{
    pthread_atfork(pre_fork, parent_fork, child_fork);
}

/*
 * pthread_lock_global_np:
 *   return: NONE
 */
void
pthd4_lock_global_np(void)
{
  pthread_mutex_lock(&pthd4__g_global_lock);
}

/*
 * pthread_unlock_global_np:
 *   return: NONE
 */
void 
pthd4_unlock_global_np(void)
{
  pthread_mutex_unlock(&pthd4__g_global_lock);
}

int
pthd4_is_multithreaded_np(void)
{
  return linuxdce_threadslib_multithreaded;
}





/****************************************************************************
 * pthd4 pthread_signal_to_cancel_np()
 *
 *
 * Semantics:  
 *             
 *             
 *
 * Assumptions: 
 *              
 *              
 *
 *
 *
 ****************************************************************************/

/*
 * pthread_signal_to_cancel_np:
 *   return:  0 - successful
 *           -1/EINVAL - invalid value for thread
 */
int
pthd4_signal_to_cancel_np(sigset_t *sigset, pthread_t *thread)
{
    int istat;

    /* XXX - First cut, no locking. */
    
    /*
     * If there already is a running handler thread, cancel it.
     * We don't bother to wait for it to die.
     */
    if (pthd4__g_handle_active) {
        pthread_cancel(pthd4__g_handle_thread);
        pthread_detach(pthd4__g_handle_thread);
    }

    pthd4__g_handle_target = *thread;
    pthd4__g_handle_sigset = *sigset;
    pthd4__g_handle_active = TRUE;

    /* Create the new handler thread */
    istat = pthread_create(&pthd4__g_handle_thread,
                           NULL,
                           (void *)&pthd4__cancel_thread,
                           NULL);
    switch (istat) {
    case SUCCESS:
        return (SUCCESS);
        break;
    case EINVAL:
    case EAGAIN:
    case EPERM:
        errno = EINVAL;
        return (FAILURE);
        break;
    default: 
        errno = EINVAL;
        return (FAILURE);
    }
}

extern void *
pthd4__cancel_thread(void)
{
    int retsig;
    int istat;

    for (;;) {
        if ((istat = sigwait(&pthd4__g_handle_sigset, &retsig)) != SUCCESS) {
            /* Will anyone ever see this? */
            errno = istat;
	    /* TODO check return */
            return NULL;

        }

        pthread_cancel(pthd4__g_handle_target);
    }
}

            
int
linuxdce_set_pthread_is_multithreaded(int new)
{
  int old;
  old = linuxdce_threadslib_multithreaded;
  linuxdce_threadslib_multithreaded = new;
  return old;
}
