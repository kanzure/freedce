/**************************************************************************
 *
 * DCE Threads Compatibility Library for Linux
 *
 * Maintainer:
 *	Miroslaw Dobrzanski-Neumann <mirek-dn@t-online.de>
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
 *      Miroslaw Dobrzanski-Neumann <mirek-dn@t-online.de>
 *
 ***************************************************************************/


/*
 *    
 *  COPYRIGHT NOTICE
 *    
 *  Copyright (C) 2000 Dobrzanski-Neumann <mirek-dn@t-online.de>
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


#ifndef __PTHREAD_DCE_ATFORK_H
#define __PTHREAD_DCE_ATFORK_H


#include <features.h>


__BEGIN_DECLS


#define ATFORK_STACK_SIZE 256

/** Pthread Draft 4 atfork handler. */
typedef void (* fork_handler_4_t)(void*);


/** Pthread Draft 7 atfork handler. */
typedef void (* fork_handler_7_t)(void);


/** Pthread Draft 4 handler control block */
struct atfork_cb_4_t
{
	fork_handler_4_t pre;    /**< pre fork handler */
	fork_handler_4_t parent; /**< after fork handler in parent */
	fork_handler_4_t child;  /**< after fork handler in child */
	void*	         data;   /**< parameter passed to each handler */
};


/** Pthread Draft 7 handler control block */
struct atfork_cb_7_t
{
	fork_handler_7_t pre;    /**< pre fork handler */
	fork_handler_7_t parent; /**< after fork handler in parent */
	fork_handler_7_t child;  /**< after fork handler in child */
};

/** atfrok handler control block */
struct atfork_cb_t
{
	int	draft4; /**< discriminator */
	union {
		struct atfork_cb_4_t fh4; /**< Draft 4 Variant */
		struct atfork_cb_7_t fh7; /**< Draft 7 Variant */
	} cb;
};

/**
 * Registers a atfork handler.
 * Note this is an internal function for compatibility.
 * between both Drafts 4 and 7
 */

int pthd4_pthread_atfork __P((struct atfork_cb_t *));


__END_DECLS


#endif

