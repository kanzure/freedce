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
 *      Jim Doyle                 <jrd@bu.edu>
 *      John Rousseau             (rousseau@world.std.com>
 *      Andrew Sandoval           <sandoval@perigee.net>
 *      Michael T. Peterson       <mtp@big.aa.net>                 
 *
 ***************************************************************************/

/*
 *
 *  COPYRIGHT NOTICE
 *
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


#ifndef __DCETHREADS_CONF_H
#  include "dcethreads_conf.h"
#endif


#include </usr/include/pthread.h>       /* Import platform LinuxThreads */


/* Enable Draft 4 POSIX Threads API compatibility */


#ifndef _DCE_PTHREADS_COMPAT_
#define _DCE_PTHREADS_COMPAT_ 1
#endif


#ifndef _DCE_PTHREAD_EXC_H_
#define _DCE_PTHREAD_EXC_H_


#include "pthread_dce_common.h"    /* Import common D4/D7 overlays */
#include "pthread_dce_exc.h"       /* Import DCE Threads */
#include "exc_handling.h"


#endif

