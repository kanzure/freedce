#define NCK_NEED_MARSHALLING 1

#include <dg.h>
#include <dce/rpc.h>
#include <dce/conv.h>
#include <dce/stubbase.h>


#include <pwd.h>                /* for getpwuid, etc, for "level_none" */

#include <ntlmsspauthcn.h>
#include <ntlmsspauth.h>

#include <ctype.h>
#include "proto.h"


void print_asc(int level, unsigned char  const *buf, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level,
				("%c", isprint(buf[i]) ? buf[i] : '.'));
	}
}

void dump_data(int level, const char *buf1, int len)
{
	unsigned char const *buf = (unsigned char const *)buf1;
	int i = 0;

	if (buf == NULL)
	{
		RPC_DBG_PRINTF(rpc_e_dbg_auth, level, ("dump_data: NULL, len=%d\n", len));
		return;
	}
	if (len < 0)
		return;
	if (len == 0)
	{
		RPC_DBG_PRINTF(rpc_e_dbg_auth, level, ("\n"));
		return;
	}

	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("[%03X] ", i));
	for (i = 0; i < len;)
	{
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("%02X ", (int)buf[i]));
		i++;
		if (i % 8 == 0)
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		if (i % 16 == 0)
		{
			print_asc(level, &buf[i - 16], 8);
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
			print_asc(level, &buf[i - 8], 8);
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("\n"));
			if (i < len)
				RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("[%03X] ", i));
		}
	}

	if (i % 16 != 0)	/* finish off a non-16-char-length row */
	{
		int n;

		n = 16 - (i % 16);
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		if (n > 8)
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		while (n--)
			RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("   "));

		n = MIN(8, i % 16);
		print_asc(level, &buf[i - (i % 16)], n);
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, (" "));
		n = (i % 16) - n;
		if (n > 0)
			print_asc(level, &buf[i - n], n);
		RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, level, ("\n"));
	}
}

void dump_data_pw(const char *msg, const uchar * data, size_t len)
{
	RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 100, ("%s", msg));
	if (data != NULL && len > 0)
	{
		dump_data(100, data, len);
	}
}

char *tab_depth(int depth)
{
	static pstring spaces;
	memset(spaces, ' ', depth * 4);
	spaces[depth * 4] = 0;
	return spaces;
}

/*******************************************************************
safe string copy into a known length string. maxlength does not
include the terminating zero.
********************************************************************/
char *safe_strcpy(char *dest,const char *src, size_t maxlength)
{
    size_t len;

    if (!dest) {
        RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1,("ERROR: NULL dest in safe_strcpy\n"));
        return NULL;
    }

    if (!src) {
        *dest = 0;
        return dest;
    }  

    len = strlen(src);

    if (len > maxlength) {
	    RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1,("ERROR: string overflow by %d in safe_strcpy [%.50s]\n",
		     (int)(len-maxlength), src));
	    len = maxlength;
    }
      
    memcpy(dest, src, len);
    dest[len] = 0;
    return dest;
}  

/*******************************************************************
safe string cat into a string. maxlength does not
include the terminating zero.
********************************************************************/
char *safe_strcat(char *dest, const char *src, size_t maxlength)
{
    size_t src_len, dest_len;

    if (!dest) {
        RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1,("ERROR: NULL dest in safe_strcat\n"));
        return NULL;
    }

    if (!src) {
        return dest;
    }  

    src_len = strlen(src);
    dest_len = strlen(dest);

    if (src_len + dest_len > maxlength) {
	    RPC_DBG_ADD_PRINTF(rpc_e_dbg_auth, 1,("ERROR: string overflow by %d in safe_strcat [%.50s]\n",
		     (int)(src_len + dest_len - maxlength), src));
	    src_len = maxlength - dest_len;
    }
      
    memcpy(&dest[dest_len], src, src_len);
    dest[dest_len + src_len] = 0;
    return dest;
}

/****************************************************************************
 protected memcpy that deals with NULL parameters.
 ****************************************************************************/
int memcpy_zero(void *to, const void *from, size_t size)
{
	if (to == NULL)
	{
		return False;
	}
	if (from == NULL)
	{
		memset(to, 0, size);
		return False;
	}

	memcpy(to, from, size);
	return True;
}

/*******************************************************************
 Put an ASCII string into a UNICODE array (uint16's).
 ********************************************************************/
void ascii_to_unistr(uint16 *dest, const char *src, int maxlen)
{
	uint16 *destend = dest + maxlen;
	register char c;

	while (dest < destend)
	{
		c = *(src++);
		if (c == 0)
		{
			break;
		}

		*(dest++) = ((uint16)c & 0xff);
	}

	*dest = 0;
}


/*******************************************************************
 Pull an ASCII string out of a UNICODE array (uint16's).
 ********************************************************************/
void unistr_to_ascii(char *dest, const uint16 *src, int len)
{
	char *destend = dest + len;
	register uint16 c;

	while (dest < destend)
	{
		c = *(src++);
		if (c == 0)
		{
			break;
		}

		*(dest++) = (char)c;
	}

	*dest = 0;
}


/*******************************************************************
 Put an ASCII string into a UNICODE buffer (little endian).
 ********************************************************************/

char *ascii_to_unibuf(char *dest, const char *src, int maxlen)
{
	char *destend = dest + maxlen;
	register char c;

	while (dest < destend)
	{
		c = *(src++);
		if (c == 0)
		{
			break;
		}

		*(dest++) = c;
		*(dest++) = 0;
	}

	*dest++ = 0;
	*dest++ = 0;
	return dest;
}


/*******************************************************************
 Pull an ASCII string out of a UNICODE buffer (little endian).
 ********************************************************************/

const char* unibuf_to_ascii(char *dest, const char *src, int maxlen)
{
	char *destend = dest + maxlen;
	register char c;

	while (dest < destend)
	{
		c = *(src++);
		if ((c == 0) && (*src == 0))
		{
			break;
		}

		*dest++ = c;
		src++;
	}

	*dest = 0;

	return src;
}
