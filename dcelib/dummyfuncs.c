
#include <stdio.h>
#include <dce/rpc.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

/*
 * Dummy functions for DCE 1.1 RPC on Linux
 *
 */

void rpc_ns_binding_export(
	/* [in] */ unsigned32 entry_name_syntax __attribute__((__unused__)),
	/* [in] */ unsigned_char_p_t entry_name __attribute__((__unused__)),
	/* [in] */ rpc_if_handle_t if_spec __attribute__((__unused__)),
	/* [in] */ rpc_binding_vector_p_t binding_vector
	__attribute__((__unused__)),
	/* [in] */ uuid_vector_p_t object_uuid_vector __attribute__((__unused__)),
	/* [out] */ unsigned32 *status
)
{
	*status = rpc_s_name_service_unavailable;
}
 


void
rpc_ns_binding_import_begin(
    /* [in] */ unsigned32 entry_name_syntax __attribute__((__unused__)),
    /* [in] */ unsigned_char_p_t entry_name __attribute__((__unused__)),
    /* [in] */ rpc_if_handle_t if_spec __attribute__((__unused__)),
    /* [in] */ uuid_p_t object_uuid __attribute__((__unused__)),
    /* [out] */ rpc_ns_handle_t *import_context __attribute__((__unused__)),
    /* [out] */ unsigned32 *status
)
{
  *status = rpc_s_name_service_unavailable;
}

void
rpc_ns_binding_import_done(
    /* [in, out] */ rpc_ns_handle_t *import_context
	 __attribute__((__unused__)),
    /* [out] */ unsigned32 *status    
)
{
  *status = rpc_s_name_service_unavailable;
}

void
rpc_ns_mgmt_handle_set_exp_age(
    /* [in] */ rpc_ns_handle_t ns_handle __attribute__((__unused__)),
    /* [in] */ unsigned32 expiration_age __attribute__((__unused__)),
    /* [out] */ unsigned32 *status

)
{
  *status = rpc_s_name_service_unavailable;
}

void
rpc_ns_binding_import_next(
    /* [in] */ rpc_ns_handle_t import_context __attribute__((__unused__)),
    /* [out] */ rpc_binding_handle_t *binding __attribute__((__unused__)),
    /* [out] */ unsigned32 *status
	       )
{
  *status = rpc_s_name_service_unavailable;
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

    return (0);
}

#include <sched.h>

void
pthd4_yield(void)
{
#ifdef __linux__
	sched_yield();
#else
	sleep(0);
#endif
}

int pthd4_delay_np(struct timespec *delay)
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

    while ((res == -1) && (errno == EINTR)) {
        request.tv_sec = remainder.tv_sec;
        request.tv_nsec = remainder.tv_nsec;
        res = nanosleep(&request, &remainder);
    }

    if (res == 0) {
        return (0);
    }
    else {
      errno = EINVAL;
      return (-1);
    }
}


