/**************************************************************************
 *
 * DCE Threads Compatibility Library for Linux
 * 
 * A DCE Threads emulation layer ontop of Pthreads on Linux.
 *
 **************************************************************************
 * Maintainer:                    Loic Domaigne <LoicWorks@gmx.net> 
 *-------------------------------------------------------------------------
 *
 * This module provides wrappers around well known system calls like 
 * open(2), close(2) etc. in order to define DCE wrappers for D4 call 
 * semantic. 
 *
 * This module has been inspired from the original pthd4_libc_wrapers.c 
 * of Jim Doyle / Miroslaw Dobrzanski-Neumann. However it has been 
 * rewritten from scratch to make it portable accross GNU/Linux system 
 * (present and future).  
 * 
 * Special Thanks to Roland McGrath <roland@redhat.com> for his tips 
 * that make this modules so portable within GNU/Linux system. 
 *
 ****************************************************************************
 * Change Log 
 *---------------------------------------------------------------------------
 *
 * 2003.05.03: loic 
 *   Handled Single Unix Specification prototype for sendXXX/recvXXX 
 *   functions.
 *
 * 2004.12.06: loic
 *   rewrite entirely pthd4_libc_wrapers.c module 
 *
 ****************************************************************************
 * COPYRIGHT NOTICE
 *---------------------------------------------------------------------------
 *
 * This software derives from source from several other implementations
 * and efforts to support DCE Threads including:
 *
 *    OSF/DCE V1.1 Public Domain RPC Release
 *    Michael T. Peterson's PCthreads package and DCE RPC port
 *    Andrew Sandoval's port of DCE RPC to Linux
 *
 * This DCE Threads package is provided under the GNU General Public 
 * License. The interfaces to Exceptions and Threads were taken
 * from the header files of the OSF DCE V1.1 RPC Public Domain Release
 *
 * Contributors to this package include:
 *
 *	Jim Doyle                   <jrd@bu.edu>
 *      John Rousseau               <rousseau@world.std.com>
 * 	Andrew Sandoval		    <sandoval@perigee.net>
 *	Michael T. Peterson	    <mtp@big.aa.net>
 *      Miroslaw Dobrzanski-Neumann <mirek-dn@t-online.de> 
 *
 *****************************************************************************/

#include "dce/dcethreads_conf.h"

static char rcsid [] __attribute__((__unused__)) = "$Id: pthd4_libc_wrapers.c,v 1.4 2009/02/27 19:14:07 lkcl Exp $";

#ifdef HAVE_OS_WIN32
#include </usr/i586-mingw32msvc/include/pthread.h>    /* Import platform win32 threads*/
#else
#include </usr/include/pthread.h>          /* Import platform LinuxThreads */
#endif

#include "pthread_dce_atfork.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdarg.h>

#include <dlfcn.h>

#ifndef PIC
const int __dcethread_provide_wrappers = 0;
#endif


/*############################################################################
  NON CANCELABLE SYSTEM CALLS
  ############################################################################

  The following define a wrapper for the system calls that are NOT cancelable 
  in the DCEthread semantic, but are in Pthreads. The list of non-cancelable
  system calls in DCE is:

    close(2)
    creat(2)
    fnctl(2)
    fsync(2)
    lseek(2)
    msync(2)
    open(2)
    pause(2)
    system(3)
    tcdrain(3)
    wait(2)
    waitpid(2)

    For those system calls, a wrapper is defined in order to provide the 
    DCEthreads semantics instead of the Pthreads semantic. 

--- NOTE -------------------------------------------------------------------
  
  Some of the functions listed above are normally async-signal-safe, but 
  the wrapper use non-async-signal-safe functions like pthread_setcanceltype(3)
  or dlsym(3).
  
  However, this _might_ be a non-issue on GNU/Linux system. 

*/

/****************************************************************************
 * NON_CANCELABLE_SYSCALL- wrapper to define non cancelable syscalls        *
 ****************************************************************************
 *                                                                          *
 * this macro define the wrapper of system calls in such a way that         *
 * cancellation is disabled.                                                *
 *                                                                          *
 * to be used with syscall having a fixed number of arguments               *
 ****************************************************************************/
 
#define NON_CANCELABLE_SYSCALL(res_type, name, param_list, params)	\
									\
  res_type								\
  name param_list							\
  {									\
    res_type result;							\
    int      old_cancel_type;						\
    res_type (*glibc_function) param_list;				\
									\
    glibc_function = dlsym(RTLD_NEXT, #name);				\
    pthread_setcanceltype (PTHREAD_CANCEL_DISABLE, &old_cancel_type);	\
    result = glibc_function params;					\
    pthread_setcanceltype (old_cancel_type, NULL);			\
    return result;							\
  }									\



/****************************************************************************
 * NON_CANCELABLE_SYSCALL_VA- wrapper to define non cancelable syscalls     *
 ****************************************************************************
 *                                                                          *
 * idem as NON_CANCELABLE_SYSCALL but for system calls with a variable      *
 * number of arguments.                                                     *
 ****************************************************************************/

#define NON_CANCELABLE_SYSCALL_VA(res_type, name, param_list, params, last_arg)	\
  res_type								\
  name param_list							\
  {									\
    va_list  ap;							\
    res_type result;							\
    int      old_cancel_type;						\
    res_type (*glibc_function) param_list;				\
									\
    glibc_function = dlsym(RTLD_NEXT, #name);				\
    pthread_setcanceltype (PTHREAD_CANCEL_DISABLE, &old_cancel_type);	\
    va_start (ap, last_arg);						\
    result = glibc_function params;					\
    va_end (ap);							\
    pthread_setcanceltype (old_cancel_type, NULL);			\
    return result;							\
  }									\

/*============================================================================*/


/*--------------------------------------------------------*
 * close(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (int, close, (int fd), (fd));


/*--------------------------------------------------------*
 * creat(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (int, creat, 
			(const char*pathname, mode_t mode),
			(pathname, mode)
			);


/*--------------------------------------------------------*
 * fcntl(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL_VA (int, fcntl, 
			   (int fd, int cmd, ...),
			   (fd, cmd, va_arg (ap, long int)), 
			   cmd
			   );


/*--------------------------------------------------------*
 * fsync(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (int, fsync, (int fd), (fd));


/*--------------------------------------------------------*
 * lseek(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (off_t, lseek, 
			(int fd, off_t offset, int whence),
			(fd, offset, whence)
			);


/*--------------------------------------------------------*
 * msync(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (off_t, msync, 
			(__ptr_t addr, size_t length, int flags),
			(addr, length, flags)
			);


/*--------------------------------------------------------*
 * open(2)                                                *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL_VA (int, open, 
			   (const char *pathname, int flags, ...),
			   (pathname, flags, va_arg (ap, mode_t)), 
			   flags
			   );


/*--------------------------------------------------------*
 * pause(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (int, pause, (void), ());


/*--------------------------------------------------------*
 * system(3)                                              *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (int, system, (const char *line), (line));


/*--------------------------------------------------------*
 * wait(2)                                                *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (pid_t, wait, (int* status), (status));


/*--------------------------------------------------------*
 * wapid(2)                                               *
 *--------------------------------------------------------*/
NON_CANCELABLE_SYSCALL (pid_t, waitpid, 
			(pid_t pid, int *status, int options),
			(pid, status, options)
			);




/*############################################################################
  CANCELABLE SYSTEM CALLS
  ############################################################################

  The following define a wrapper for the system calls that are cancelable 
  in the DCEthread semantic. The list of the system calls are:

      nanosleep(2)
      read(2)
      write(2)
      accept(2)
      connect(2)
      recv(2)
      recvfrom(2)
      recvmsg(2)
      send(2)
      sendmsg(2)
      sendto(2)
      select(2)

  It happens that these system calls are exactly CP in Pthreads. 
  However, in some older glibc, there weren't true CP. This is exactly
  when the wrappers below are needed. 
  
  In newer glibc, the corresponding glibc functions are not wrapped, since 
  the they are true CP.  

--- NOTE --------------------------------------------------------------------
  
  Some of the functions listed above are normally async-signal-safe, but 
  the wrapper use non-async-signal-safe functions like pthread_testcancel(3)
  or dlsym(3).
  
  However, this _might_ be a non-issue on GNU/Linux system. 

*/

#if USE_CANCELATION_WRAPPER
 
#define CANCELABLE_SYSCALL(res_type, name, param_list, params)		\
     res_type								\
  name param_list							\
  {									\
    res_type result;							\
    res_type (*glibc_function) param_list;				\
									\
    glibc_function = dlsym(RTLD_NEXT, #name);				\
    pthread_testcancel();						\
    result = glibc_function params;					\
    pthread_testcancel ();						\
    return result;							\
  }									\



/*--------------------------------------------------------*
 * nanosleep(2)                                           *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (int, nanosleep, 
		    (const struct timespec *requested_time,
		     struct timespec       *remaining
		     ),
		    (requested_time, remaining)
		    );


/*--------------------------------------------------------*
 * read(2)                                                *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (ssize_t, 
		    read, 
		    (int fd, void *buf, size_t count),
		    (fd, buf, count)
		    );


/*--------------------------------------------------------*
 * write(2)                                               *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (ssize_t, 
		    write, 
		    (int fd, const void *buf, size_t n),
		    (fd, buf, n)
		    );


/*--------------------------------------------------------*
 * accept(2)                                              *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (int, accept, 
		    (int fd, struct sockaddr *addr, socklen_t *addr_len),
		    (fd, addr, addr_len)
		    );


/*--------------------------------------------------------*
 * connect(2)                                             *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (int, connect, 
		    (int fd, const struct sockaddr *addr, socklen_t addrlen),
		    (fd, addr, addrlen)
		    );


/*--------------------------------------------------------*
 * select(2)                                              *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (int, select,
		    (int nfds, 
		     fd_set *readfds, fd_set *writefds, fd_set *exceptfds, 
		     struct timeval *timeout
		     ),
		    (nfds, readfds, writefds, exceptfds, timeout)
		    );


/* 
 * For the networking API, we use the Single Unix Specification or the 
 * BSD proto depending on the environment 
 */ 

#ifdef _BSD_SOURCE 
/*-----------------------------------------*
 * use BSD prototype                       *
 *-----------------------------------------*/ 
#define net_type ssize_t 
#else 
/*-----------------------------------------*
 * use Single Unix Specification prototype *
 *-----------------------------------------*/ 
#define net_type ssize_t
#endif 


/*--------------------------------------------------------*
 * recv(2)                                                *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (net_type, 
		    recv, 
		    (int fd, __ptr_t buf, size_t n, int flags),
		    (fd, buf, n, flags)
		    );


/*--------------------------------------------------------*
 * recvfrom(2)                                            *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (net_type, recvfrom, 
		    (int fd, void*__restrict buf, size_t n, int flags,
		     __SOCKADDR_ARG addr, socklen_t *__restrict addr_len),
		    (fd, buf, n, flags, addr, addr_len))

/*--------------------------------------------------------*
 * recvmsg(2)                                            *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (net_type, recvmsg, 
		    (int fd, struct msghdr *message, int flags),
		    (fd, message, flags)
		    );


/*--------------------------------------------------------*
 * send(2)                                                *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (net_type, send, 
		    (int fd, const __ptr_t buf, size_t n, int flags),
		    (fd, buf, n, flags)
		    );


/*--------------------------------------------------------*
 * sendmsg(2)                                             *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (net_type, sendmsg, 
		    (int fd, const struct msghdr *message, int flags),
		    (fd, message, flags)
		    );


/*--------------------------------------------------------*
 * sendto(2)                                              *
 *--------------------------------------------------------*/
CANCELABLE_SYSCALL (net_type, sendto, 
		    (int fd, const __ptr_t buf, size_t n,
		     int flags, const struct sockaddr* addr,
		     socklen_t addr_len
		     ),
		    (fd, buf, n, flags, addr, addr_len)
		    );


#endif /* USE_CANCEL_WRAPPER */



/****************************************************************************
 * pthd4_wrapper_pthread_atfork- wrapper for pthread_atfork()               *
 ****************************************************************************
 *                                                                          *
 * This wrapper for pthread_atfork(3thr) is provided for binary             *
 * compatibility with Pthreads. Notice however that there is no             *
 * pthread_atfork(3thr) in D4.                                              *
 *                                                                          *
 ***************************************************************************/
int 
pthd4_wrapper_pthread_atfork __P ((fork_handler_7_t pre,
				   fork_handler_7_t parent,
				   fork_handler_7_t child))
{
  struct atfork_cb_t cb;
  cb.draft4        = !!0;
  cb.cb.fh7.pre    = pre;
  cb.cb.fh7.parent = parent;
  cb.cb.fh7.child  = child;
  return pthd4_pthread_atfork(&cb);
}
