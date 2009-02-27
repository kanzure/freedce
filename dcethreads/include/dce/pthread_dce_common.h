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

/*
 * Many changes to support linux threads 0.8 / glibc2.1
 * by Miroslaw Dobrzanski-Neumann <mirek-dn@t-online.de> 
 */

/***************************************************************************
 *
 * Common Definitions for DCE Threads Layering ontop of LinuxThreads
 *
 ***************************************************************************/


#ifndef _PTHREAD_DCE_COMMONDEFS_
#define _PTHREAD_DCE_COMMONDEFS_

/*
 * DCE Threads uses PTHREAD_DEFAULT_SCHED 
 */

#define  PTHREAD_DEFAULT_SCHED PTHREAD_INHERIT_SCHED

/* 
 * DCE Thread Priorities
 *
 * used by: pthread_attr_setprio(), pthread_attr_getprio(), 
 * pthread_setprio(), pthread_getprio().
 */

#define PRI_OTHER_MIN           0
#define PRI_OTHER_MAX           127
#define PRI_FIFO_MIN            PRI_OTHER_MIN           
#define PRI_FIFO_MAX            PRI_OTHER_MAX           
#define PRI_RR_MIN              PRI_OTHER_MIN           
#define PRI_RR_MAX              PRI_OTHER_MAX           
#define PRI_FG_MIN_NP           PRI_OTHER_MIN           
#define PRI_FG_MAX_NP           PRI_OTHER_MAX           
#define PRI_BG_MIN_NP           PRI_OTHER_MIN           
#define PRI_BG_MAX_NP           PRI_OTHER_MAX           

/*
 * DCE Threads Scheduling 
 *
 * pthread_attr_setsched(), pthread_attr_getsched(), 
 * pthread_setsched(), pthread_getsched().
 */


/* #define SCHED_FIFO           Already defined by LinuxThreads */
/* #define SCHED_RR             Already defined by LinuxThreads */
/* #define SCHED_OTHER          Already defined by LinuxThreads */
#define SCHED_FG_NP             SCHED_OTHER
#define SCHED_BG_NP             SCHED_OTHER

/*
 * DCE Threads Mutexes
 *
 * Alias to reflect values used by LinuxThreads 0.7
 */

#define MUTEX_FAST_NP                   PTHREAD_MUTEX_FAST_NP
#if defined(PTHREAD_MUTEX_ERRORCHECK_NP)
#define MUTEX_NONRECURSIVE_NP           PTHREAD_MUTEX_ERRORCHECK_NP
#else
#define MUTEX_NONRECURSIVE_NP           PTHREAD_MUTEX_FAST_NP
#endif

#define MUTEX_RECURSIVE_NP              PTHREAD_MUTEX_RECURSIVE_NP

/************************************************************************
 * TYPES AND DEFINITIONS:
 ************************************************************************
 *
 *
 ************************************************************************
 */

typedef void * pthread_addr_t;
typedef void * (*pthread_startroutine_t)(void *);
typedef void  (*pthread_cleanup_t)(void *);
typedef void  (*pthread_destructor_t)(void *);
typedef void  (*pthread_initroutine_t)(void);

#undef pthread4_attr_default
#undef pthread4_mutexattr_default
#undef pthread4_condattr_default
#undef pthread4_once_init

#define pthread_attr_default		(pthread4_attr_default)
#define pthread_mutexattr_default	(pthread4_mutexattr_default)
#define pthread_condattr_default	(pthread4_condattr_default)
#define pthread_once_init		PTHREAD_ONCE_INIT

extern pthread_attr_t		pthread4_attr_default;
extern pthread_mutexattr_t	pthread4_mutexattr_default;
extern pthread_condattr_t	pthread4_condattr_default;
extern const pthread_once_t	pthread4_once_init;

#ifndef CANCEL_ON
#define CANCEL_ON (1)
#endif

#ifndef CANCEL_OFF
#define CANCEL_OFF (0)
#endif

#ifndef SUCCESS
#define SUCCESS (0)
#endif

#ifndef FAILURE
#define FAILURE (-1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#endif  
