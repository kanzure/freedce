/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   SMB parameters and setup
   Copyright (C) Andrew Tridgell 1992-1997
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

#ifndef _RPC_MISC_H		/* _RPC_MISC_H */
#define _RPC_MISC_H

/*
 * macros to wrap prs_xxxx routines.
 */
#define prs_uint8(name, ps, depth, data8) \
	if (!_prs_uint8(name, ps, depth, data8)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_uint16(name, ps, depth, data16) \
	if (!_prs_uint16(name, ps, depth, data16)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_hash1(x_ps, x_offset, x_sess_key) \
	if (!_prs_hash1(x_ps, x_offset, x_sess_key)) \
	{\
		x_ps->offset = 0;\
		return False;\
	}
#define prs_uint32(name, ps, depth, data32) \
	if (!_prs_uint32(name, ps, depth, data32)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_uint8s(charmode, name, ps, depth, data8s, len) \
	if (!_prs_uint8s(charmode, name, ps, depth, data8s, len)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_uint16s(charmode, name, ps, depth, data16s, len) \
	if (!_prs_uint16s(charmode, name, ps, depth, data16s, len)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_uint32s(charmode, name, ps, depth, data32s, len) \
	if (!_prs_uint32s(charmode, name, ps, depth, data32s, len)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_buffer2(charmode, name, ps, depth, str) \
	if (!_prs_buffer2(charmode, name, ps, depth, str)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_string2(charmode, name, ps, depth, str) \
	if (!_prs_string2(charmode, name, ps, depth, str)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_unistr2(charmode, name, ps, depth, str) \
	if (!_prs_unistr2(charmode, name, ps, depth, str)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_unistr(name, ps, depth, str) \
	if (!_prs_unistr(name, ps, depth, str)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_string(name, ps, depth, str, len, max_buf_size) \
	if (!_prs_string(name, ps, depth, str, len, max_buf_size)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_uint16_pre(x_name, x_ps, x_depth, x_data16, x_offset) \
	if (!_prs_uint16_pre(x_name, x_ps, x_depth, x_data16, x_offset)) \
	{\
		x_ps->offset = 0;\
		return False;\
	}
#define prs_uint16_post(name, ps, depth, data16, ptr_uint16, data_size) \
	if (!_prs_uint16_post(name, ps, depth, data16, ptr_uint16, data_size)) \
	{\
		ps->offset = 0;\
		return False;\
	}
#define prs_uint32_pre(x_name, x_ps, x_depth, x_data32, x_offset) \
	if (!_prs_uint32_pre(x_name, x_ps, x_depth, x_data32, x_offset)) \
	{\
		x_ps->offset = 0;\
		return False;\
	}
#define prs_uint32_post(name, ps, depth, data32, ptr_uint32, data_size) \
	if (!_prs_uint32_post(name, ps, depth, data32, ptr_uint32, data_size)) \
	{\
		ps->offset = 0;\
		return False;\
	}

/* STRHDR - string header */
typedef struct header_info
{
	uint16 str_str_len;
	uint16 str_max_len;
	uint32 buffer;		/* non-zero */

}
STRHDR;

/* STRHDR2 - string header, 32-bit lengths */
typedef struct header2_info
{
	uint32 str_str_len;
	uint32 str_max_len;
	uint32 buffer;

}
STRHDR2;

/* UNIHDR - unicode string header */
typedef struct unihdr_info
{
	uint16 uni_str_len;
	uint16 uni_max_len;
	uint32 buffer;		/* usually has a value of 4 */

}
UNIHDR;

/* UNIHDR2 - unicode string header and undocumented buffer */
typedef struct unihdr2_info
{
	UNIHDR unihdr;
	uint32 buffer;		/* 32 bit buffer pointer */

}
UNIHDR2;

/* clueless as to what maximum length should be */
#define MAX_UNISTRLEN 256
#define MAX_STRINGLEN 256
#define MAX_BUFFERLEN 512

/* UNISTR - unicode string size and buffer */
typedef struct unistr_info
{
	uint16 buffer[MAX_UNISTRLEN];	/* unicode characters. ***MUST*** be null-terminated */

}
UNISTR;

/* BUFHDR - buffer header */
typedef struct bufhdr_info
{
	uint32 buf_max_len;
	uint32 buf_len;

}
BUFHDR;

/* BUFHDR2 - another buffer header, with info level */
typedef struct bufhdr2_info
{
	uint32 info_level;
	uint32 length;		/* uint8 chars */
	uint32 buffer;

}
BUFHDR2;

/* BUFFER2 - unicode string, size (in uint8 ascii chars) and buffer */
/* pathetic.  some stupid team of \PIPE\winreg writers got the concept */
/* of a unicode string different from the other \PIPE\ writers */
typedef struct buffer2_info
{
	uint32 buf_max_len;
	uint32 undoc;
	uint32 buf_len;
	uint8 buffer[MAX_UNISTRLEN];

}
BUFFER2;

/* BUFFER3 */
typedef struct buffer3_info
{
	uint32 buf_max_len;
	uint8 buffer[MAX_BUFFERLEN];	/* data */
	uint32 buf_len;

}
BUFFER3;

/* BUFFER4 - simple length and buffer */
typedef struct buffer4_info
{
	uint32 buf_len;
	uint8 buffer[MAX_BUFFERLEN];

}
BUFFER4;

/* BUFFER5 */
typedef struct buffer5_info
{
	uint32 buf_len;
	uint16 *buffer;		/* data */
}
BUFFER5;

/* UNISTR2 - unicode string size (in uint16 unicode chars) and buffer */
typedef struct unistr2_info
{
	uint32 uni_max_len;
	uint32 undoc;
	uint32 uni_str_len;
	uint16 buffer[MAX_UNISTRLEN];	/* unicode characters. **NOT** necessarily null-terminated */

}
UNISTR2;

/* STRING2 - string size (in uint8 chars) and buffer */
typedef struct string2_info
{
	uint32 str_max_len;
	uint32 undoc;
	uint32 str_str_len;
	uint8 buffer[MAX_STRINGLEN];	/* uint8 characters. **NOT** necessarily null-terminated */

}
STRING2;

/* UNISTR3 - XXXX not sure about this structure */
typedef struct unistr3_info
{
	uint32 uni_str_len;
	UNISTR str;

}
UNISTR3;

#endif /* _RPC_MISC_H */
