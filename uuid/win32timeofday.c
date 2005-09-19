#ifdef HAVE_OS_WIN32
#include <windows.h>
#include <time.h>

int win32_gettimeofday(struct timeval *tp, void *unused) {
  SYSTEMTIME syst;
  time_t tlocal;
  struct tm tmlocal;
  unused = 0;
  GetLocalTime(&syst);
  tmlocal.tm_sec = syst.wSecond;
  tmlocal.tm_min = syst.wMinute;
  tmlocal.tm_hour = syst.wHour;
  tmlocal.tm_mday = syst.wDay;
  tmlocal.tm_mon = syst.wMonth - 1;
  tmlocal.tm_year = syst.wYear - 1900;
  tmlocal.tm_isdst = -1;
  tlocal = mktime (&tmlocal); /* convert to UTC */
  tp->tv_sec = tlocal;
  tp->tv_usec = syst.wMilliseconds * 1000;
  return 1;
}
#else
int win32_gettimeofday(struct timeval *tp, void *unused) {
	tp = 0;
	unused = 0;
}
#endif /* HAVE_OS_WIN32 */

