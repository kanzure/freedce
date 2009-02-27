/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   SMB parameters and setup
   Copyright (C) Andrew Tridgell              1992-2000
   Copyright (C) John H Terpstra              1996-2000
   Copyright (C) Luke Kenneth Casson Leighton 1996-2001
   Copyright (C) Paul Ashton                  1998-2000
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _SMB_H
#define _SMB_H

#define BUFFER_SIZE (0xFFFF)
#define SAFETY_MARGIN 1024
#define LARGE_WRITEX_HDR_SIZE 65

#ifndef _BOOL
#define False (0)
#define True (1)
#define Auto (2)
typedef int BOOL;
#define _BOOL       /* So we don't typedef BOOL again in vfs.h */
#endif

/* limiting size of ipc replies */
#define REALLOC(ptr,size) Realloc(ptr,MAX((size),4*1024))

#define SIZEOFWORD 2

#ifndef _PSTRING

#define PSTRING_LEN 1024
#define FSTRING_LEN 128

typedef char pstring[PSTRING_LEN];
typedef char fstring[FSTRING_LEN];
#define _PSTRING
#endif


/* 64 bit time (100usec) since ????? - cifs6.txt, section 3.5, page 30 */
typedef struct nttime_info
{
	uint32 low;
	uint32 high;

} NTTIME;


#ifdef HAVE_STDARG_H
int slprintf(char *str, int n, char *format, ...)
#ifdef __GNUC__
	__attribute__ ((format(printf, 3, 4)))
#endif
	;
#else
int slprintf();
#endif

#ifdef HAVE_STDARG_H
int fdprintf(int fd, char *format, ...)
#ifdef __GNUC__
	__attribute__ ((format(printf, 2, 3)))
#endif
	;
#else
int fdprintf();
#endif

#ifndef SIGNAL_CAST
#define SIGNAL_CAST (RETSIGTYPE (*)(int))
#endif

#ifndef SELECT_CAST
#define SELECT_CAST
#endif




#include "smb_macros.h"


#define AGENT_CMD_CON       0
#define AGENT_CMD_CON_ANON  2
#define AGENT_CMD_CON_REUSE 1

struct pwd_info;

#include "ntdomain.h"

#if 0
struct ntdom_info
{
	unsigned char usr_sess_key[16];	/* Current user session key. */
	uint32 ntlmssp_cli_flgs;	/* ntlmssp client flags */
	uint32 ntlmssp_srv_flgs;	/* ntlmssp server flags */

	unsigned char sess_key[16];	/* Client NETLOGON session key. */
	DOM_CRED clnt_cred;	/* Client NETLOGON credential. */

	int max_recv_frag;
	int max_xmit_frag;
};
#endif


#endif /* _SMB_H */
