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


#include "dce/dcethreads_conf.h"

static char rcsid [] __attribute__((__unused__)) = "$Id: pthread_dce_atfork.c,v 1.2 2000/09/02 21:35:55 mirek-dn Exp $";


#include </usr/include/pthread.h>

#include "pthread_dce_atfork.h"
#include "dce/pthread_dce_common.h"
#include <stddef.h>
#include <errno.h>

#define ATFORK_STACK_SIZE 256

static size_t             _atfork_stack_size = -1;
static struct atfork_cb_t _atfork_stack[ATFORK_STACK_SIZE] =
	{ {.draft4 = 0, { .fh4 = {NULL, NULL, NULL ,NULL}}} };

static pthread_once_t  _atfork_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t _atfork_lock = PTHREAD_MUTEX_INITIALIZER;


static void _pthd4_atfork_handler_pre(void)
{
	struct atfork_cb_t* ptr;
	for(ptr = _atfork_stack + _atfork_stack_size; ptr-- != _atfork_stack; )
		if (ptr->draft4) {
			if (ptr->cb.fh4.pre)
				(ptr->cb.fh4.pre)(ptr->cb.fh4.data);
		}
		else {
			if (ptr->cb.fh7.pre)
				(ptr->cb.fh7.pre)();
		}
}


static void _pthd4_atfork_handler_parent(void)
{
	struct atfork_cb_t* ptr;
	for (ptr = _atfork_stack; ptr != _atfork_stack + _atfork_stack_size; ptr++)
		if (ptr->draft4) {
			if (ptr->cb.fh4.parent)
				(ptr->cb.fh4.parent)(ptr->cb.fh4.data);
		}
		else {
			if (ptr->cb.fh7.parent)
				(ptr->cb.fh7.parent)();
		}
}


static void _pthd4_atfork_handler_child(void)
{
	struct atfork_cb_t* ptr;
	for (ptr = _atfork_stack; ptr != _atfork_stack + _atfork_stack_size; ptr++)
		if (ptr->draft4) {
			if (ptr->cb.fh4.child)
				(ptr->cb.fh4.child)(ptr->cb.fh4.data);
		}
		else {
			if (ptr->cb.fh7.child)
				(ptr->cb.fh7.child)();
		}
}


static void _pthd4_install_atfork_handler(void)
{
	int res;
	res = __pthread_atfork(
			_pthd4_atfork_handler_pre,
			_pthd4_atfork_handler_parent,
			_pthd4_atfork_handler_child
			    );
	if (!res) {
		_atfork_stack_size = 0;
		errno = res;
	}
}


int pthd4_pthread_atfork __P((struct atfork_cb_t * cb))
{
	pthread_once(&_atfork_once, &_pthd4_install_atfork_handler);

	if ((typeof(_atfork_stack_size))-1 == _atfork_stack_size)
		return errno;

	pthread_mutex_lock(&_atfork_lock);

	if (ATFORK_STACK_SIZE <= _atfork_stack_size) {
		errno = ENOMEM;
		pthread_mutex_unlock(&_atfork_lock);
		return errno;
	}

	_atfork_stack[_atfork_stack_size++] = *cb;
	pthread_mutex_unlock(&_atfork_lock);
	return SUCCESS;
}

