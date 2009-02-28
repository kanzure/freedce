#ifndef _NETLOGON_MISC_H
#define _NETLOGON_MISC_H
void nl_chk_dce_err(error_status_t, char *, char *, unsigned int);
void nl_print_asc(unsigned char  const *buf, int len);
void nl_dump_data(const char *buf1, int len);
#endif

