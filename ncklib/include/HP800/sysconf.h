/*
 * 
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
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
 */
/*
**
**  NAME:
**
**      sysconf.h
**
**  FACILITY:
**
**      Remote Procedure Call (RPC) 
**
**  ABSTRACT:
**
**  This file contains all definitions specific to the HPUX hppa platform
**
**
*/

#ifndef _SYSCONF_H
#define _SYSCONF_H	1

/*****************************************************************************/

#include <dce/dce.h>

#include <errno.h>
#include <stdio.h>
#include <sys/file.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <dce/pthread_exc.h>
#include <dce/exc_handling.h>

/****************************************************************************/

/* define some macros to override defaults */

#define RPC_C_SOCKET_GUESSED_RCVBUF    (8 * 1024)
#define RPC_C_SOCKET_GUESSED_SNDBUF    (8 * 1024)

#define RPC_C_SOCKET_MAX_RCVBUF (56 * 1024)
#define RPC_C_SOCKET_MAX_SNDBUF (56 * 1024)


/****************************************************************************/

/* define some macros to support atfork handler */

#define ATFORK_SUPPORTED

#define ATFORK(handler) rpc__cma_atfork(handler)

extern void rpc__cma_atfork _DCE_PROTOTYPE_ (( void * ));

/****************************************************************************/

#ifndef UNIX
#define UNIX	1
#endif /* UNIX */

#ifndef BSD
#define BSD	1
#endif /* BSD */

#ifndef STDARG_PRINTF
#define STDARG_PRINTF	1
#endif /* STDARG_PRINTF */

#define srandom(seed)     srand ((int) seed)
#define random            rand 

#define RPC_DEFAULT_NLSPATH "/usr/lib/nls/C/%s.cat"

#endif /* _SYSCONF_H */
