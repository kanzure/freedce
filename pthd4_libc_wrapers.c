/**************************************************************************
 *
 * DCE Threads Compatibility Library for Linux
 * 
 * A DCE Threads emulation layer ontop of LinuxThreads.
 *
 **************************************************************************
 *
 * Maintainer:			Jim Doyle <jrd@bu.edu>
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
 *	Jim Doyle                 <jrd@bu.edu>
 *      John Rousseau             (rousseau@world.std.com>
 * 	Andrew Sandoval		  <sandoval@perigee.net>
 *	Michael T. Peterson	  <mtp@big.aa.net>
 *
 ***************************************************************************/

/*
 * Many changes to support linux threads 0.8 / glibc2.1
 *
 * by Miroslaw Dobrzanski-Neumann <mne@mosaic-ag.com> 
 *
 * the DCE jackets for D4 call semantic
 *
 * jackets marked with XXX should be OK please check it if you can
 * jackets marked with FIXME are to be fixed
 *
 * some of the jackets below have the same semantic as the
 * current linux thread implementation
 * leave them here please the servies here are mentioned by the DCE
 *
 * all the jackets (should) have the same versions number as the
 * libc calls please look in libc for the correct version and
 * modify accordingly the Versions file
 */

#include </usr/include/pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdarg.h>

#ifndef PIC
const int __dcethread_provide_wrappers = 0;
#endif

#define PSEUDO_CANCELABLE_SYSCALL(res_type, name, param_list, params)	\
res_type __##name param_list;					\
res_type								\
name param_list								\
{									\
	res_type result;						\
	int oldtype;							\
	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);	\
	pthread_testcancel ();						\
	result = __##name params;					\
	pthread_setcanceltype (oldtype, NULL);				\
	return result;							\
}

#define CANCELABLE_SYSCALL(res_type, name, param_list, params)		\
res_type __libc_##name param_list;					\
res_type								\
name param_list								\
{									\
	res_type result;						\
	int oldtype;							\
	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);	\
	result = __libc_##name params;					\
	pthread_setcanceltype (oldtype, NULL);				\
	return result;							\
}

#define CANCELABLE_SYSCALL_VA(res_type, name, param_list, params, last_arg)	\
res_type __libc_##name param_list;						\
res_type									\
name param_list									\
{										\
	res_type result;							\
	int oldtype;								\
	va_list ap;								\
	pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);		\
	va_start (ap, last_arg);						\
	result = __libc_##name params;						\
	va_end (ap);								\
	pthread_setcanceltype (oldtype, NULL);					\
	return result;								\
}


#define NON_CANCELABLE_SYSCALL(res_type, name, param_list, params)	\
res_type __libc_##name param_list;				\
res_type							\
name param_list							\
{								\
	res_type result;					\
	result = __libc_##name params;				\
	return result;						\
}

#define NON_CANCELABLE_SYSCALL_VA(res_type, name, param_list, params, last_arg)	\
res_type __libc_##name param_list;					\
res_type								\
name param_list								\
{									\
	res_type result;						\
	va_list ap;							\
	va_start (ap, last_arg);					\
	result = __libc_##name params;					\
	va_end (ap);							\
	return result;							\
}

# define strong_alias(name, aliasname) \
	extern __typeof (name) aliasname __attribute__ ((alias (#name)));




/* XXX close(2).  */
NON_CANCELABLE_SYSCALL (int, close, (int fd), (fd))
strong_alias (close, __close)


/* XXX fcntl(2).  */
NON_CANCELABLE_SYSCALL_VA (int, fcntl, (int fd, int cmd, ...),
		       (fd, cmd, va_arg (ap, long int)), cmd)
strong_alias (fcntl, __fcntl)


/* XXX fsync(2).  */
NON_CANCELABLE_SYSCALL (int, fsync, (int fd), (fd))


/* XXX lseek(2).  */
NON_CANCELABLE_SYSCALL (off_t, lseek, (int fd, off_t offset, int whence),
		    (fd, offset, whence))
strong_alias (lseek, __lseek)


/* XXX msync(2).  */
NON_CANCELABLE_SYSCALL (int, msync, (__ptr_t addr, size_t length, int flags),
		    (addr, length, flags))


/* FIXME nanosleep(2).  */
#if 0
CANCELABLE_SYSCALL (int, nanosleep, (const struct timespec *requested_time,
				     struct timespec *remaining),
		    (requested_time, remaining))
#endif


/* XXX open(2).  */
NON_CANCELABLE_SYSCALL_VA (int, open, (const char *pathname, int flags, ...),
		       (pathname, flags, va_arg (ap, mode_t)), flags)
strong_alias (open, __open)


/* XXX pause(2).  */
NON_CANCELABLE_SYSCALL (int, pause, (void), ())


/* XXX read(2). DCE */
CANCELABLE_SYSCALL (ssize_t, read, (int fd, void *buf, size_t count),
		    (fd, buf, count))
strong_alias (read, __read)


/* XXX system(3).  */
NON_CANCELABLE_SYSCALL (int, system, (const char *line), (line))


/* XXX tcdrain(2).  */
NON_CANCELABLE_SYSCALL (int, tcdrain, (int fd), (fd))


/* XXX wait(2).  */
NON_CANCELABLE_SYSCALL (__pid_t, wait, (__WAIT_STATUS_DEFN stat_loc), (stat_loc))
strong_alias (wait, __wait)


/* XXX waitpid(2).  */
NON_CANCELABLE_SYSCALL (__pid_t, waitpid, (__pid_t pid, int *stat_loc,
				       int options),
		    (pid, stat_loc, options))


/* XXX write(2). DCE */
CANCELABLE_SYSCALL (ssize_t, write, (int fd, const void *buf, size_t n),
		    (fd, buf, n))
strong_alias (write, __write)


/* The following system calls are thread cancellation points specified
   in XNS.  */

/* XXX accept(2). DCE */
CANCELABLE_SYSCALL (int, accept, (int fd, __SOCKADDR_ARG addr,
				  socklen_t *addr_len),
		    (fd, addr, addr_len))

/* XXX connect(2). DCE */
CANCELABLE_SYSCALL (int, connect, (int fd, __CONST_SOCKADDR_ARG addr,
				     socklen_t len),
		    (fd, addr, len))
strong_alias (connect, __connect)

/* XXX recv(2). DCE */
CANCELABLE_SYSCALL (int, recv, (int fd, __ptr_t buf, size_t n, int flags),
		    (fd, buf, n, flags))

/* XXX recvfrom(2). DCE */
CANCELABLE_SYSCALL (int, recvfrom, (int fd, __ptr_t buf, size_t n, int flags,
				    __SOCKADDR_ARG addr, socklen_t *addr_len),
		    (fd, buf, n, flags, addr, addr_len))

/* XXX recvmsg(2). DCE */
CANCELABLE_SYSCALL (int, recvmsg, (int fd, struct msghdr *message, int flags),
		    (fd, message, flags))

/* XXX send(2). DCE */
CANCELABLE_SYSCALL (int, send, (int fd, const __ptr_t buf, size_t n,
				int flags),
		    (fd, buf, n, flags))
strong_alias (send, __send)

/* XXX sendmsg(2). DCE */
CANCELABLE_SYSCALL (int, sendmsg, (int fd, const struct msghdr *message,
				   int flags),
		    (fd, message, flags))

/* XXX sendto(2). DCE */
CANCELABLE_SYSCALL (int, sendto, (int fd, const __ptr_t buf, size_t n,
				  int flags, __CONST_SOCKADDR_ARG addr,
				  socklen_t addr_len),
		    (fd, buf, n, flags, addr, addr_len))




/* XXX select(2) DCE */
PSEUDO_CANCELABLE_SYSCALL (int, select,
		(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout),
		(nfds, readfds, writefds, exceptfds, timeout));


