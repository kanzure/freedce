#ifndef _NCACN_NP_H
#define _NCACN_NP_H

#include <twrp.h>

typedef struct sockaddr_np
{
	u_short snp_family;
	char serv_name[TWR_C_NP_SERVNAME_SIZE];
	char pipe_name[TWR_C_NP_PIPENAME_SIZE];
} sockaddr_np_t, *sockaddr_np_p_t;

#endif
