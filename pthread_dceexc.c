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
static const char rcsid[] = "$Id: pthread_dceexc.c,v 1.1 2000/08/13 02:03:22 wez Exp $";
#endif

/*
 * Exceptions returning interfaces
 */

#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include <pthread_dce_common.h>    /* Import common D4/D7 overlays */
#include <pthread_dce_proto.h>

#ifdef __BUILD_DCE_PTHREADS_LIBRARY
# ifdef __BUILD_DCE_PTHREADS_LIBRARY_STDALONE
# include <exc_handling.h>
#else
# include <dce/exc_handling.h>
#endif /* STDALONE */
#else
#include <exc_handling.h>
#endif /* DCE Build */

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


extern void _exc_thread_init();    /* exception library initialization */

/*
 * declared in pthread_dce.c
 * initialized by pthd4_lib_init().
 */

extern pthread_attr_t               pthread4_attr_default;
extern pthread_mutexattr_t          pthread4_mutexattr_default;
extern pthread_condattr_t           pthread4_condattr_default;
static pthread_once_t               pthd4exc_is_initialized = 
                                                 pthread_once_init;

/*
 * Maps errno values returned by pthd4_xxx() interface to a DCE Exception
 */

#define pthd4_map_errno_to_exc(x)               		\
								\
	switch (x)						\
	 {							\
	   case EBUSY:		RAISE(pthread_in_use_e);	\
				break;				\
	   case EAGAIN:		RAISE(pthread_in_use_e);	\
				break;				\
	   case ENOMEM:		RAISE(exc_insfmem_e);		\
				break;				\
	   case EPERM:		RAISE(exc_nopriv_e);		\
				break;				\
           case ERANGE:		RAISE(pthread_badparam_e);	\
				break;				\
           case ENOSYS:		RAISE(pthread_unimp_e);		\
				break;				\
           case ESRCH:		RAISE(pthread_use_error_e);	\
				break;				\
           case EINVAL:         RAISE(pthread_badparam_e);      \
                                break;                          \
	   default:		printf("map_errno_to_exc: hucking a %d\n", x); RAISE(pthread_badparam_e);      \
	 }



/*
 * at most once initialization of Exc returning DCE threads interface
 */

static void 
pthd4exc_lib_init( void )
{
 _exc_thread_init();
}


/*****************************************************************************
 * Exception returning interfaces to DCE Threads 
 *****************************************************************************/

int
pthd4exc_attr_create( pthread_attr_t *attr )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_create( attr );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_attr_delete( pthread_attr_t *attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_delete( attr );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int 
pthd4exc_attr_setstacksize( pthread_attr_t *attr, long stacksize )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_setstacksize( attr , stacksize);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

long
pthd4exc_attr_getstacksize( pthread_attr_t attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_getstacksize( attr );
  return e;

}

int
pthd4exc_create( pthread_t *th_h,
              pthread_attr_t attr,
              pthread_startroutine_t proc,
              pthread_addr_t arg )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  

  e = pthd4_create(th_h, attr, proc, arg);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

/****/

int 
pthd4exc_detach( pthread_t *thread )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_detach( thread );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

void
pthd4exc_exit( pthread_addr_t status )
{
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  pthd4_exit( status );
}

int 
pthd4exc_join( pthread_t thread, pthread_addr_t *status )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  

  e = pthd4_join( thread, status );
  if (e != SUCCESS)
    {
      switch(e) 
	{
	 
	  /*
	   * jrd added 05-09 PTHREAD_CANCEL
	   */

	 case PTHREAD_CANCELED:
	                  RAISE(pthread_cancel_e);
			  break;
	 case ESRCH:      
	                  RAISE(pthread_use_error_e);
	                  break;
	 case EDEADLK:    
	                  RAISE(pthread_in_use_e);
			  break;
	 default:        
	                  RAISE(pthread_badparam_e);
			  break;
	}            
    }
  /* TODO check the return */
  return e;
}

int
pthd4exc_mutexattr_create( pthread_mutexattr_t *attr )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutexattr_create( attr );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int 
pthd4exc_mutexattr_delete ( pthread_mutexattr_t *attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutexattr_delete( attr );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int 
pthd4exc_mutex_init( pthread_mutex_t * mutex, pthread_mutexattr_t attr )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutex_init( mutex, attr );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_mutex_destroy( pthread_mutex_t *mutex )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutex_destroy( mutex );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int
pthd4exc_mutex_lock( pthread_mutex_t *mutex )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutex_lock( mutex );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int 
pthd4exc_mutex_trylock( pthread_mutex_t *mutex )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutex_trylock( mutex );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int 
pthd4exc_mutex_unlock( pthread_mutex_t *mutex )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutex_unlock( mutex );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;


}

int
pthd4exc_condattr_create( pthread_condattr_t *attr )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_condattr_create( attr );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int 
pthd4exc_condattr_delete ( pthread_condattr_t *attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_condattr_delete( attr );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;


}


int 
pthd4exc_cond_init( pthread_cond_t *cond, pthread_condattr_t attr )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_cond_init( cond, attr );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;


}

int 
pthd4exc_cond_destroy( pthread_cond_t *cond )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_cond_destroy( cond );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_cond_broadcast( pthread_cond_t *cond )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_cond_broadcast( cond );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_cond_signal( pthread_cond_t *cond )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_cond_signal( cond );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_cond_wait( pthread_cond_t *cond, pthread_mutex_t *mutex )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_cond_wait( cond, mutex );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int 
pthd4exc_cond_timedwait(pthread_cond_t *cond,
                        pthread_mutex_t *mutex,
                        struct timespec *abstime)
{
    int e;

    if (pthread_once(&pthd4exc_is_initialized, pthd4exc_lib_init) != SUCCESS)
        RAISE(pthread_use_error_e);
    
    e = pthd4_cond_timedwait(cond, mutex, abstime);
    if ((e != SUCCESS) && (errno == EAGAIN)) {
        return (EAGAIN);
    } 

    if (e != SUCCESS) {
        pthd4_map_errno_to_exc(errno);
    }

    return (SUCCESS);
}

int 
pthd4exc_once( pthread_once_t *once_block, void (*init_routine)(void) )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_once( once_block, init_routine);
  if ((e != SUCCESS) && (errno == EINVAL)) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_keycreate( pthread_key_t *key, pthread_destructor_t destructor )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_keycreate( key, destructor);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_setspecific( pthread_key_t key, pthread_addr_t value )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_setspecific(key, value);
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_getspecific( pthread_key_t key, pthread_addr_t *value )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_getspecific(key, value);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_cancel( pthread_t thread )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_cancel(thread);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_setasynccancel( int state )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_setasynccancel( state );
  /* 
   * pthd4_setASYNCcancel() returns -1 for error. Othewise the previous
   * value of the cancel state is returned. 
   */
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int 
pthd4exc_setcancel( int state )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_setcancel( state );
  /* 
   * pthd4_setcancel() returns -1 for error. Othewise the previous
   * value of the cancel state is returned. 
   */

  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int
pthd4exc_attr_setprio( pthread_attr_t * attr, int priority)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_setprio( attr, priority );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int
pthd4exc_attr_getprio( pthread_attr_t attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_getprio( attr );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}


int
pthd4exc_attr_setsched( pthread_attr_t * attr, int sched)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_setsched( attr, sched );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int
pthd4exc_attr_getsched( pthread_attr_t attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_getsched( attr );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int
pthd4exc_attr_setinheritedsched( pthread_attr_t * attr, int sched)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_setinheritedsched( attr, sched );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int
pthd4exc_attr_getinheritedsched( pthread_attr_t attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_getinheritedsched( attr );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int
pthd4exc_attr_setguardsize_np( pthread_attr_t * attr, long size)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_setguardsize_np( attr, size );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

long
pthd4exc_attr_getguardsize_np( pthread_attr_t attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_attr_getguardsize_np(attr);
  return e;

}

int
pthd4exc_mutexattr_setkind_np( pthread_mutexattr_t * mutex_attr, int kind)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutexattr_setkind_np( mutex_attr, kind );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int
pthd4exc_mutexattr_getkind_np( pthread_mutexattr_t mutex_attr)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_mutexattr_getkind_np( mutex_attr);
  return e;

}

int
pthd4exc_cond_signal_int_np( pthread_cond_t * cond)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_cond_signal_int_np( cond );
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;

}

int
pthd4exc_setprio( pthread_t thread, int priority)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_setprio( thread, priority );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int
pthd4exc_getprio( pthread_t thread)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_getprio(thread);
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int
pthd4exc_setscheduler( pthread_t thread, int scheduler, int priority)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_setscheduler(thread, scheduler, priority);
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int
pthd4exc_getscheduler( pthread_t thread )
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_getscheduler( thread );
  if (e == -1) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}


void
pthd4exc_testcancel(void)
{
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  pthd4_testcancel();
}

int
pthd4exc_equal(pthread_t thd1, pthread_t thd2)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_equal(thd1, thd2);
  return e;

}


void 
pthd4exc_yield( void )
{
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  pthd4_yield();
}

pthread_t
pthd4exc_self( void )
{
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  return pthd4_self();
}

int
pthd4exc_delay_np(struct timespec * delay)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_delay_np(delay);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}


int 
pthd4exc_get_expiration_np(struct timespec * delta, struct timespec * abstime)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_get_expiration_np(delta, abstime);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

int
pthd4exc_getunique_np(pthread_t * thread)
{
 return pthd4_getunique_np( thread );
}

int
pthd4exc_signal_to_cancel_np(sigset_t * sigset, pthread_t * thread)
{
  int e;
  if ( pthread_once( &pthd4exc_is_initialized, pthd4exc_lib_init ) != SUCCESS )
                RAISE(pthread_use_error_e);
  
  e = pthd4_signal_to_cancel_np(sigset, thread);
  if (e != SUCCESS) 
    {
      pthd4_map_errno_to_exc(errno);
    }
  return e;
}

void
pthd4exc_atfork(void * userstate, 
                void (*pre_fork)(void), 
                void (*parent_fork)(void), 
                void (*child_fork)(void) 
	     )
{
  pthd4_atfork(userstate, pre_fork, parent_fork, child_fork);
}

void
pthd4exc_lock_global_np(void)
{
  pthd4_lock_global_np();
}

void 
pthd4exc_unlock_global_np(void)
{
  pthd4_unlock_global_np();
}
