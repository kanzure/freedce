/*The following definitions come from  libsmb/smbdes.c  */

#ifndef _NETLOGON_SMBHASH_H
#define _NETLOGON_SMBHASH_H

void nl_smbhash(unsigned char *out, const unsigned char *in, const unsigned char *key, int forw);
void nl_cred_hash1(unsigned char *out, const unsigned char *in, const unsigned char *key);
void nl_cred_hash2(unsigned char *out,unsigned char *in,unsigned char *key);

#endif
