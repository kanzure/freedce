#ifndef __DCETHREADS_CONF_H
#define __DCETHREADS_CONF_H

#ifdef COMPILING_DCETHREADS
/*
 * this definitions are only needed when compiling
 * the library itself
 */
@TOP@
#undef YIELD_AFTER_PTHREAD_CREATE
#undef USE_CANCELATION_WRAPPER
@BOTTOM@
/*
 * end of private configuration
 */
#endif /* COMPILING_DCETHREADS */

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE 1
#endif

#ifndef _REENTRANT
#  define _REENTRANT 1
#endif

#ifndef HAVE_FEATURES_H
#undef HAVE_FEATURES_H
#endif
#ifdef HAVE_FEATURES_H
#  include <features.h>
#endif

#ifndef HAVE_SYS_CDEFS_H
#undef HAVE_SYS_CDEFS_H
#endif
/*
 * sys/cdefs.h is often included by features.h
 */
#if !defined(_SYS_CDEFS_H) && defined(HAVE_SYS_CDEFS_H)
#  include <sys/cdefs.h>
#endif

/*
 * if the system has own idea of what __P should be we use it
 */
#ifndef __P
#  define __P(x) x
#endif

#ifndef __BEGIN_DECLS
#  ifdef __cplusplus
#    define __BEGIN_DECLS  extern "C" {
#    define __END_DECLS    }
#  else
#    define __BEGIN_DECLS
#    define __END_DECLS
#  endif
#endif

#endif /* __DCETHREADS_CONF_H */
