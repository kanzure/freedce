
/* 
 *  Unix SMB/Netbios implementation.
 *  Version 1.9.
 *  RPC Pipe client / server routines
 *  Copyright (C) Andrew Tridgell              1992-2000,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-2001,
 *  Copyright (C) Paul Ashton                       1997,
 *  Copyright (C) Elrond                            2000.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "includes.h"


/*******************************************************************
 Reads or writes an NTTIME structure.
********************************************************************/

BOOL smb_io_time(char *desc, NTTIME *nttime, prs_struct *ps, int depth)
{
	if (nttime == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_time");
	depth++;

	prs_align(ps);
	
	prs_uint32("low ", ps, depth, &(nttime->low )); /* low part */
	prs_uint32("high", ps, depth, &(nttime->high)); /* high part */

	return True;
}

/*******************************************************************
creates a STRHDR structure.
********************************************************************/
BOOL make_str_hdr(STRHDR *hdr, int max_len, int len, uint32 buffer)
{
	hdr->str_max_len = max_len;
	hdr->str_str_len = len;
	hdr->buffer      = buffer;

	return True;
}

/*******************************************************************
 Reads or writes a STRHDR structure.
********************************************************************/

BOOL smb_io_strhdr(char *desc,  STRHDR *hdr, prs_struct *ps, int depth)
{
	if (hdr == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_strhdr");
	depth++;

	prs_align(ps);
	
	prs_uint16("str_str_len", ps, depth, &(hdr->str_str_len));
	prs_uint16("str_max_len", ps, depth, &(hdr->str_max_len));
	prs_uint32("buffer     ", ps, depth, &(hdr->buffer     ));

	/* oops! XXXX maybe issue a warning that this is happening... */
	if (hdr->str_max_len > MAX_STRINGLEN) hdr->str_max_len = MAX_STRINGLEN;
	if (hdr->str_str_len > MAX_STRINGLEN) hdr->str_str_len = MAX_STRINGLEN;

	return True;
}

/*******************************************************************
creates a STRHDR2 structure.
********************************************************************/
BOOL make_strhdr2(STRHDR2 *hdr, uint32 max_len, uint32 len, uint32 buffer)
{
	hdr->str_max_len = max_len;
	hdr->str_str_len = len;
	hdr->buffer      = buffer;

	return True;
}

/*******************************************************************
reads or writes a STRHDR2 structure.
********************************************************************/
BOOL smb_io_strhdr2(char *desc, STRHDR2 *hdr, prs_struct *ps, int depth)
{
	if (hdr == NULL) return False;

	prs_debug(ps, depth, desc, "smb_io_strhdr");
	depth++;

	prs_align(ps);
	
	prs_uint32("str_str_len", ps, depth, &(hdr->str_str_len));
	prs_uint32("str_max_len", ps, depth, &(hdr->str_max_len));
	prs_uint32("buffer     ", ps, depth, &(hdr->buffer     ));

	/* oops! XXXX maybe issue a warning that this is happening... */
	if (hdr->str_max_len > MAX_STRINGLEN) hdr->str_max_len = MAX_STRINGLEN;
	if (hdr->str_str_len > MAX_STRINGLEN) hdr->str_str_len = MAX_STRINGLEN;

	return True;
}

/*******************************************************************
creates a UNIHDR structure.
********************************************************************/

BOOL make_uni_hdr(UNIHDR *hdr, int len)
{
	if (hdr == NULL)
	{
		return False;
	}
	hdr->uni_str_len = 2 * len;
	hdr->uni_max_len = 2 * len;
	hdr->buffer      = len != 0 ? 1 : 0;

	return True;
}

/*******************************************************************
creates a UNIHDR structure from a UNISTR2.
********************************************************************/
BOOL make_unihdr_from_unistr2(UNIHDR *hdr, const UNISTR2 *str)
{
	int len;

	len = (str ? str->uni_str_len : 0);

	return make_uni_hdr(hdr, len);
}

/*******************************************************************
 Reads or writes a UNIHDR structure.
********************************************************************/

BOOL smb_io_unihdr(const char *desc,  UNIHDR *hdr, prs_struct *ps, int depth)
{
	if (hdr == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_unihdr");
	depth++;

	prs_align(ps);
	
	prs_uint16("uni_str_len", ps, depth, &(hdr->uni_str_len));
	prs_uint16("uni_max_len", ps, depth, &(hdr->uni_max_len));
	prs_uint32("buffer     ", ps, depth, &(hdr->buffer     ));

	/* oops! XXXX maybe issue a warning that this is happening... */
	if (hdr->uni_max_len > MAX_UNISTRLEN) hdr->uni_max_len = MAX_UNISTRLEN;
	if (hdr->uni_str_len > MAX_UNISTRLEN) hdr->uni_str_len = MAX_UNISTRLEN;

	return True;
}

/*******************************************************************
creates a BUFHDR structure.
********************************************************************/
BOOL make_buf_hdr(BUFHDR *hdr, int max_len, int len)
{
	hdr->buf_max_len = max_len;
	hdr->buf_len     = len;

	return True;
}

/*******************************************************************
 prs_uint16 wrapper. Call this and it sets up a pointer to where the
 uint16 should be stored, or gets the size if reading.
 ********************************************************************/

BOOL smb_io_hdrbuf_pre(char *desc, BUFHDR *hdr, prs_struct *ps, int depth, uint32 *offset)
{
	(*offset) = prs_offset(ps);
	if (ps->io) {

		/* reading. */
		smb_io_hdrbuf(desc, hdr, ps, depth);
	} else {
		ps->offset += sizeof(uint32) * 2;
	}

	return True;
}

/*******************************************************************
 smb_io_hdrbuf wrapper. Call this and it retrospectively stores the size.
 Does nothing on reading, as that is already handled by ...._pre()
 ********************************************************************/

BOOL smb_io_hdrbuf_post(char *desc, BUFHDR *hdr, prs_struct *ps, int depth, 
				uint32 ptr_hdrbuf, uint32 max_len, uint32 len)
{
	if (!ps->io) {
		/* storing: go back and do a retrospective job.  i hate this */
		uint32 old_offset = ps->offset;

		make_buf_hdr(hdr, max_len, len);
		ps->offset = ptr_hdrbuf;
		smb_io_hdrbuf(desc, hdr, ps, depth);
		ps->offset = old_offset;
	}

	return True;
}

/*******************************************************************
 Reads or writes a BUFHDR structure.
********************************************************************/
BOOL smb_io_hdrbuf(char *desc,  BUFHDR *hdr, prs_struct *ps, int depth)
{
	if (hdr == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_hdrbuf");
	depth++;

	prs_align(ps);
	
	prs_uint32("buf_max_len", ps, depth, &(hdr->buf_max_len));
	prs_uint32("buf_len    ", ps, depth, &(hdr->buf_len    ));

	/* oops! XXXX maybe issue a warning that this is happening... */
	if (hdr->buf_max_len > MAX_BUFFERLEN) hdr->buf_max_len = MAX_BUFFERLEN;
	if (hdr->buf_len     > MAX_BUFFERLEN) hdr->buf_len     = MAX_BUFFERLEN;

	return True;
}

/*******************************************************************
creates a BUFHDR2 structure.
********************************************************************/
BOOL make_bufhdr2(BUFHDR2 *hdr, uint32 info_level, uint32 length, uint32 buffer)
{
	hdr->info_level = info_level;
	hdr->length     = length;
	hdr->buffer     = buffer;

	return True;
}

/*******************************************************************
reads or writes a BUFHDR2 structure.
********************************************************************/
BOOL smb_io_bufhdr2(char *desc, BUFHDR2 *hdr, prs_struct *ps, int depth)
{
	if (hdr == NULL) return False;

	prs_debug(ps, depth, desc, "smb_io_bufhdr2");
	depth++;

	prs_align(ps);
	prs_uint32("info_level", ps, depth, &(hdr->info_level));
	prs_uint32("length    ", ps, depth, &(hdr->length    ));
	prs_uint32("buffer    ", ps, depth, &(hdr->buffer    ));

	return True;
}

/*******************************************************************
creates a UNIHDR2 structure.
********************************************************************/
BOOL make_uni_hdr2(UNIHDR2 *hdr, int len)
{
	if (hdr == NULL)
	{
		return False;
	}
	make_uni_hdr(&(hdr->unihdr), len);
	hdr->buffer = len > 0 ? 1 : 0;

	return True;
}

/*******************************************************************
creates a UNIHDR2 structure from a UNISTR2.
********************************************************************/
BOOL make_unihdr2_from_unistr2(UNIHDR2 *hdr, const UNISTR2 *str)
{
	int len;

	len = (str ? str->uni_str_len : 0);

	return make_uni_hdr2(hdr, len);
}

/*******************************************************************
 Reads or writes a UNIHDR2 structure.
********************************************************************/
BOOL smb_io_unihdr2(const char *desc,  UNIHDR2 *hdr2, prs_struct *ps, int depth)
{
	if (hdr2 == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_unihdr2");
	depth++;

	prs_align(ps);

	smb_io_unihdr("hdr", &(hdr2->unihdr), ps, depth);
	prs_uint32("buffer", ps, depth, &(hdr2->buffer));

	return True;
}

/*******************************************************************
creates a BUFFER3 structure from a uint32
********************************************************************/
BOOL make_buffer3_uint32(BUFFER3 *str, uint32 val)
{
	ZERO_STRUCTP(str);

	/* set up string lengths. */
	str->buf_max_len = sizeof(uint32);
	str->buf_len     = sizeof(uint32);

	SIVAL(str->buffer, 0, val);

	return True;
}

/*******************************************************************
creates a BUFFER3 structure.
********************************************************************/
BOOL make_buffer3_bytes(BUFFER3 *str, uint8 *buf, int len)
{
	ZERO_STRUCTP(str);

	/* max buffer size (allocated size) */
	str->buf_max_len = len;
	if (buf != NULL)
	{
		memcpy(str->buffer, buf, MIN(str->buf_len, sizeof(str->buffer)));
	}
	str->buf_len = buf != NULL ? len : 0;

	return True;
}

/*******************************************************************
 Reads or writes a BUFFER3 structure.
   the uni_max_len member tells you how large the buffer is.
   the uni_str_len member tells you how much of the buffer is really used.
********************************************************************/
BOOL smb_io_buffer3(char *desc,  BUFFER3 *buf3, prs_struct *ps, int depth)
{
	if (buf3 == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_buffer3");
	depth++;

	prs_align(ps);
	
	prs_uint32("uni_max_len", ps, depth, &(buf3->buf_max_len));
	if (buf3->buf_max_len > MAX_UNISTRLEN) buf3->buf_max_len = MAX_UNISTRLEN;

	prs_uint8s(True, "buffer     ", ps, depth, buf3->buffer, buf3->buf_max_len);

	prs_uint32("buf_len    ", ps, depth, &(buf3->buf_len));
	if (buf3->buf_len     > MAX_UNISTRLEN) buf3->buf_len     = MAX_UNISTRLEN;

	return True;
}

/*******************************************************************
reads or writes a BUFFER4 structure.
********************************************************************/
BOOL smb_io_buffer4(char *desc, BUFFER4 *buf4, uint32 buffer, prs_struct *ps, int depth)
{
	if ((buf4 == NULL) || (buffer == 0)) return False;

	prs_debug(ps, depth, desc, "smb_io_buffer4");
	depth++;

	prs_align(ps);
	prs_uint32("buf_len", ps, depth, &(buf4->buf_len));

	if (buf4->buf_len > MAX_BUFFERLEN)
	{
		buf4->buf_len = MAX_BUFFERLEN;
	}

	prs_uint8s(True, "buffer", ps, depth, buf4->buffer, buf4->buf_len);

	return True;
}

/*******************************************************************
initialise a BUFFER5 structure.
********************************************************************/
BOOL init_buffer5(BUFFER5 **str)
{
	BUFFER5 *buf5;
		
	buf5=(BUFFER5 *)malloc( sizeof(BUFFER5) );
	
	buf5->buf_len=0;
	buf5->buffer=NULL;	
	*str=buf5;

	return True;
}

/*******************************************************************
clear a BUFFER5 structure.
********************************************************************/
BOOL clear_buffer5(BUFFER5 **str)
{
	BUFFER5 *buf5;
	
	buf5=*str;	
	if (buf5->buffer != NULL )
	{
		free(buf5->buffer);
	}
	free(buf5);
	*str=NULL;

	return True;
}

/*******************************************************************
reads or writes a BUFFER5 structure.
the buf_len member tells you how large the buffer is.
********************************************************************/
BOOL smb_io_buffer5(char *desc, BUFFER5 *buf5, prs_struct *ps, int depth)
{
	prs_debug(ps, depth, desc, "smb_io_buffer4");
	depth++;

	if (buf5 == NULL) return False;

	prs_align(ps);
	prs_uint32("buf_len", ps, depth, &(buf5->buf_len));

	/* reading: alloc the buffer first */
	if ( ps->io )
	{
		buf5->buffer=(uint16 *)malloc( sizeof(uint16)*buf5->buf_len );
	}
	
	prs_uint16s(True, "buffer     ", ps, depth, buf5->buffer, buf5->buf_len);

	return True;
}

/*******************************************************************
 Frees a BUFFER5 structure (just the malloced part).
********************************************************************/

void free_buffer5(BUFFER5 *buf5)
{
	if (buf5->buffer)
		free(buf5->buffer);
}

/*******************************************************************
reads or writes a BUFFER2 structure.
     the uni_max_len member tells you how large the buffer is.
     the uni_str_len member tells you how much of the buffer is really used.
********************************************************************/
BOOL smb_io_buffer2(char *desc,  BUFFER2 *buf2, uint32 buffer, prs_struct *ps, int depth)
{
	if (buf2 == NULL) return False;

	if (buffer)
	{
		prs_debug(ps, depth, desc, "smb_io_buffer2");
		depth++;

		prs_align(ps);
		
		prs_uint32("buf_max_len", ps, depth, &(buf2->buf_max_len));
		prs_uint32("undoc      ", ps, depth, &(buf2->undoc      ));
		prs_uint32("buf_len    ", ps, depth, &(buf2->buf_len));

		/* oops! XXXX maybe issue a warning that this is happening... */
		if (buf2->buf_max_len > MAX_UNISTRLEN) buf2->buf_max_len = MAX_UNISTRLEN;
		if (buf2->buf_len     > MAX_UNISTRLEN) buf2->buf_len     = MAX_UNISTRLEN;

		/* buffer advanced by indicated length of string
		   NOT by searching for null-termination */
		prs_buffer2(True, "buffer     ", ps, depth, buf2);
	}
	else
	{
		prs_debug(ps, depth, desc, "smb_io_buffer2 - NULL");
		depth++;
		ZERO_STRUCTP(buf2);
	}

	return True;
}

/*******************************************************************
creates a UNISTR2 structure: sets up the buffer, too
BOOL make_buf_unistr2(UNISTR2 *str, uint32 *ptr, const char *buf)
{
	if (buf != NULL)
	{
		*ptr = 1;
		make_unistr2(str, buf, strlen(buf)+1);
	}
	else
	{
		*ptr = 0;
		make_unistr2(str, "", 0);
	}

	return True;
}
********************************************************************/

/*******************************************************************
creates a STRING2 structure.
********************************************************************/
BOOL make_string2(STRING2 *str, const char *buf, int len)
{
	ZERO_STRUCTP(str);

	/* set up string lengths. */
	str->str_max_len = len;
	str->undoc       = 0;
	str->str_str_len = len;

	/* store the string */
	if(len != 0)
	{
		memcpy(str->buffer, buf, len);
	}

	return True;
}

/*******************************************************************
creates a STRING2 structure: sets up the buffer, too
********************************************************************/
BOOL make_buf_string2(STRING2 *str, uint32 *ptr, const char *buf)
{
	if (buf != NULL)
	{
		*ptr = 1;
		make_string2(str, buf, strlen(buf)+1);
	}
	else
	{
		*ptr = 0;
		make_string2(str, "", 0);
	}

	return True;
}

/*******************************************************************
reads or writes a STRING2 structure.
XXXX NOTE: STRING2 structures need NOT be null-terminated.
     the str_str_len member tells you how long the string is;
     the str_max_len member tells you how large the buffer is.
********************************************************************/
BOOL smb_io_string2(char *desc,  STRING2 *str2, uint32 buffer, prs_struct *ps, int depth)
{
	if (str2 == NULL) return False;

	if (buffer)
	{
		prs_debug(ps, depth, desc, "smb_io_string2");
		depth++;

		prs_align(ps);
		
		prs_uint32("str_max_len", ps, depth, &(str2->str_max_len));
		prs_uint32("undoc      ", ps, depth, &(str2->undoc      ));
		prs_uint32("str_str_len", ps, depth, &(str2->str_str_len));

		/* oops! XXXX maybe issue a warning that this is happening... */
		if (str2->str_max_len > MAX_STRINGLEN) str2->str_max_len = MAX_STRINGLEN;
		if (str2->str_str_len > MAX_STRINGLEN) str2->str_str_len = MAX_STRINGLEN;

		/* buffer advanced by indicated length of string
		   NOT by searching for null-termination */
		prs_string2(True, "buffer     ", ps, depth, str2);
	}
	else
	{
		prs_debug(ps, depth, desc, "smb_io_string2 - NULL");
		depth++;
		ZERO_STRUCTP(str2);
	}

	return True;
}

/*******************************************************************
creates a UNISTR2 structure.
BOOL make_unistr2(UNISTR2 *str, const char *buf, int len)
{
	unistr2_assign_ascii(str, buf, len);

	return True;
}
********************************************************************/

/*******************************************************************
reads or writes a UNISTR2 structure.
XXXX NOTE: UNISTR2 structures need NOT be null-terminated.
     the uni_str_len member tells you how long the string is;
     the uni_max_len member tells you how large the buffer is.
********************************************************************/
BOOL smb_io_unistr2(const char *desc, UNISTR2 *uni2, uint32 buffer,
		    prs_struct *ps, int depth)
{
	if (uni2 == NULL) return False;

	if (buffer)
	{
		uint32 remaining = 0;
		prs_debug(ps, depth, desc, "smb_io_unistr2");
		depth++;

		prs_align(ps);
		
		if (MARSHALLING(ps))
		{
			/* oops! XXXX maybe issue a warning that this is happening... */
			if (uni2->uni_max_len > MAX_UNISTRLEN) uni2->uni_max_len = MAX_UNISTRLEN;
			if (uni2->uni_str_len > MAX_UNISTRLEN) uni2->uni_str_len = MAX_UNISTRLEN;
		}

		prs_uint32("uni_max_len", ps, depth, &(uni2->uni_max_len));
		prs_uint32("undoc      ", ps, depth, &(uni2->undoc      ));
		prs_uint32("uni_str_len", ps, depth, &(uni2->uni_str_len));

		if (UNMARSHALLING(ps))
		{
			/* oops! XXXX maybe issue a warning that this is happening... */
			if (uni2->uni_max_len > MAX_UNISTRLEN) uni2->uni_max_len = MAX_UNISTRLEN;
			if (uni2->uni_str_len > MAX_UNISTRLEN)
			{
				remaining = (uni2->uni_str_len - MAX_UNISTRLEN)
					* sizeof(uint16);
				uni2->uni_str_len = MAX_UNISTRLEN;
			}
		}

		/* buffer advanced by indicated length of string
		   NOT by searching for null-termination */
		prs_unistr2(True, "buffer     ", ps, depth, uni2);

		/* skip over the things, we ignored. */
		if (!prs_set_offset(ps, prs_offset(ps) + remaining))
			return False;
	}
	else
	{
		prs_debug(ps, depth, desc, "smb_io_unistr2 - NULL");
		depth++;
		ZERO_STRUCTP(uni2);
	}

	return True;
}

#if 0
BOOL smb_io_unistr2_with_hdr(const char *name, UNISTR2 *uni2,
			     prs_struct *ps, int flags)
{
	UNIHDR hdr;
	int depth;
	BOOL ret;

	if (!uni2)
		return False;

	/* These four should be one in the future */
	prs_debug(ps, -1, name, "smb_io_unistr2_with_hdr");
	depth = prs_depth(ps);
	depth++;
	prs_set_depth(ps, depth);

	if (MARSHALLING(ps))
	{
		make_unihdr_from_unistr2(&hdr, uni2);
	}

	if (!smb_io_unihdr(name, &hdr, ps, depth))
		goto fail;

	if (hdr.buffer)
	{
		if (!smb_io_unistr2_x(name, uni2, ps, 0))
			goto fail;
	}
	else
	{
		if (!smb_io_null_x(name, uni2, ps, 0))
			goto fail;
		unistr2_assign_ascii(uni2, "", 0);
	}

	ret = True;
	goto end;

fail:
	ret = False;

end:
	depth--;
	prs_set_depth(ps, depth);
	return ret;
}
#endif

/*******************************************************************
 Reads or writes a UNISTR3 structure.
********************************************************************/

BOOL smb_io_unistr3(char *desc,  UNISTR3 *name, prs_struct *ps, int depth)
{
	if (name == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_unistr3");
	depth++;

	prs_align(ps);
	
	prs_uint32("uni_str_len", ps, depth, &(name->uni_str_len));

	/* don't know if len is specified by uni_str_len member... */
	/* assume unicode string is unicode-null-terminated, instead */

	if(!prs_unistr3(True, "unistr", name, ps, depth))
		return False;

	return True;
}


