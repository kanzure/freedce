/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   SMB parameters and setup
   Copyright (C) Luke Kenneth Casson Leighton 1996-2001
   Copyright (C) Paul Ashton 1997
   
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

#ifndef _NT_DOMAIN_H /* _NT_DOMAIN_H */
#define _NT_DOMAIN_H 

/* dce/rpc authentication support */
#include "rpc_ntlmssp.h"

/* miscellaneous structures / defines */
#include "rpc_misc.h"

struct parse_struct;

/* If we have these macros, we can add additional info to the header. */
#ifdef HAVE_FILE_MACRO
#define FILE_MACRO (__FILE__)
#else
#define FILE_MACRO ("")
#endif

#ifdef HAVE_FUNCTION_MACRO
#define FUNCTION_MACRO  (__FUNCTION__)
#else
#define FUNCTION_MACRO  ("")
#endif

#define CHECK_STRUCT(data) \
{ \
	if ((data)->struct_start != 0xfefefefe || \
	    (data)->struct_end != 0xdcdcdcdc) \
	{ \
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1,("uninitialised structure (%s, %d)\n", \
		FUNCTION_MACRO, __LINE__)); \
		sleep(30); \
	} \
}

typedef BOOL (* prs_align_fn) (struct parse_struct *ps, int object_size);

typedef struct parse_struct
{
	uint32 struct_start;

	char *data;		/* memory buffer */
	size_t data_size;	/* current memory buffer size */
	/* array memory offsets */
	uint32 start;
	uint32 end;

	uint32 offset;		/* offset currently being accessed in memory buffer */
	uint8 align;		/* data alignment */
	prs_align_fn align_fn;  /* function used for alignment of objects */
	BOOL io;		/* parsing in or out of data stream */
	BOOL error;		/* error occurred while parsing (out of memory bounds) */
	BOOL bigendian;		/* big-endian data */
	BOOL side;		/* client (False) or server (True) */

	struct parse_struct *next;

	int depth; /* The current struct depth */

	uint32 struct_end;

}
prs_struct;

/*
 * Defines for io member of prs_struct.
 */

#define MARSHALL 0
#define UNMARSHALL 1

#define MARSHALLING(ps) (!(ps)->io)
#define UNMARSHALLING(ps) ((ps)->io)

#define PRS_CLIENT 0
#define PRS_SERVER 1
#define PRS_IS_CLIENT(ps) (!(ps)->side)
#define PRS_IS_SERVER(ps) ((ps)->side)

typedef BOOL (* GenericParseCB) (const char *name, void *item, prs_struct *ps);


#endif /* _NT_DOMAIN_H */
