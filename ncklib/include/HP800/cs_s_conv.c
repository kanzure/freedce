/*
 * 
 * (c) Copyright 1991 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1991 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1991 DIGITAL EQUIPMENT CORPORATION
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *                 permission to use, copy, modify, and distribute this
 * file for any purpose is hereby granted without fee, provided that
 * the above copyright notices and this notice appears in all source
 * code copies, and that none of the names of Open Software
 * Foundation, Inc., Hewlett-Packard Company, or Digital Equipment
 * Corporation be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Neither Open Software Foundation, Inc., Hewlett-
 * Packard Company, nor Digital Equipment Corporation makes any
 * representations about the suitability of this software for any
 * purpose.
 * 
 */
/*
 */
/*
**  NAME
**
**     cs_s_conv.c
**
**  FACILITY:
**
**     Remote Procedure Call (RPC)
**     I18N Character Set Conversion Call   (RPC)
**
**  ABSTRACT:
**
**
*/
#include <commonp.h>		/* include nbase.h lbase.h internally	*/
#include <com.h>		/* definition of rpc_binding_rep_p_t	*/
#include <dce/rpcsts.h>
#include <codesets.h>		/* Data definitions for I18N NSI 
							sub-component   */
#include <stdio.h>		/* definition of NULL			*/
#include <stdlib.h>		/* definition of MB_CUR_MAX		*/
#include <iconv.h>		/* definition of iconv routines		*/
#include <langinfo.h>		/* definition of nl_langinfo routine	*/
#include <string.h>		/* definition of strncpy routine	*/

#include <codesets.h>
#include <cs_s.h>		/* Private defs for code set interoperability */
#include <hp_conv.h>		/* Private defs for HP */

/*
**
** Binary Search for code set name
**
*/
PRIVATE
void name_binary_search
#ifdef _DCE_PROTO_
(
        codeset_iconv_name_t	*table,
        int			low,
        int			high,
        char			*codeset_name,
        char			**iconv_name
)
#else
(table, low, high, codeset_name, iconv_name)
        codeset_iconv_name_t	*table;
        int			low;
        int			high;
        char			*codeset_name;
        char			**iconv_name;
#endif
{
        int     middle, k;

        if (low <= high)
        {
                middle = (low + high) / 2;

                if ((k = strcoll(codeset_name, table[middle].codeset_name)) == 0)
                {
                        *iconv_name = table[middle].iconv_name;
                        return;
                }
                else
                {
                        if (k < 0)
                                name_binary_search(table, low, middle-1, codeset_name, iconv_name);
                        else
                                name_binary_search(table, middle+1, high, codeset_name, iconv_name);
                }
        }
}


void stub_conversion
#ifdef _DCE_PROTOTYPE_
(
	rpc_binding_handle_t	h,
	boolean32		server_side,
	unsigned32		from_tag,
	unsigned32		to_tag,
	byte_t			*conv_ldata,
	unsigned32		conv_l_data_len,
	byte_t			*conv_wdata,
	unsigned32		*conv_p_w_data_len,
	error_status_t		*status
)
#else
(h, server_side, to_conversion, from_tag, to_tag, conv_ldata, conv_l_data_len,
		conv_wdata, conv_p_w_data_len, status)
	rpc_binding_handle_t	h;
	boolean32		server_side;
	unsigned32		from_tag;
	unsigned32		to_tag;
	byte_t			*conv_ldata;
	unsigned32		conv_l_data_len;
	byte_t			*conv_wdata;
	unsigned32		*conv_p_w_data_len;
	error_status_t		*status;
#endif
{
	iconvd			cd;
	int			size;
	int			inbytesleft;
	int			outbytesleft;
	unsigned_char_t		*table;
	int			i_ret;
	int			init_len;
	char			*iconv_from_cd;		/* HP specific */
	char			*iconv_to_cd;		/* HP specific */
	int			defChar1=0xFF;		/* HP specific */
	int			defChar2=0xFF;		/* HP specific */

	/*
 	 * Get the iconv code set names (HP specific)
 	 */
	rpc__cs_get_iconv_name(to_tag, &iconv_to_cd, status);
	if (*status != rpc_s_ok)
		return;

	rpc__cs_get_iconv_name(from_tag, &iconv_from_cd, status);
	if (*status != rpc_s_ok)
		return;

	/*
 	 * Setup conversion table (HP specific)
 	 */
	if ((size = iconvsize(iconv_to_cd, iconv_from_cd)) == -1)
	{
		/*
		 * If a conversion table is needed and the table
		 * does not exist, -1 is returned.
		 * This is an error case.
		 */
		*status = rpc_s_ss_iconv_error;
		return;
	}
	
	/*
	 * Check the size of a table (HP specific)
	 */
	if (size == 0) 
	{
		/*
		 * Conversion does not require a table, thus set NULL
		 */
		table = (unsigned char *)NULL;
	}
	else if ((table = (unsigned_char_t *)malloc((unsigned int)size))
					== (unsigned_char_t *)NULL)
	{
		*status = rpc_s_no_memory;
		return;
	}
	
	/*
	 * Set conversion descriptor
	 * HP specific argument: table, defChar1, defChar2
	 */
	if ((cd = iconvopen(
		iconv_to_cd, iconv_from_cd, table, defChar1, defChar2
		  )) == (iconvd)-1)
	{
		*status = rpc_s_ss_incompatible_codesets;
		return;
	}

	/* Set the number of bytes left in input buffer */
	init_len = inbytesleft = (int)conv_l_data_len * sizeof(unsigned_char_t);
	outbytesleft = (int)conv_l_data_len * sizeof(unsigned_char_t);

	/*
	 * Convert code set.
	 * HP specific routine name: ICONV
	 */
	i_ret = ICONV(cd, &conv_ldata, &inbytesleft, &conv_wdata, &outbytesleft);

	/*
	 * Check the conversion result.
	 * HP specific return values : Not the standard defined values.
	 */
	switch(i_ret)
	{
	case 0: /* all characters are successfully converted */
		*status = rpc_s_ok;
		break;

	case 1: /* multi-byte input or a lock-shift sequence spans
		 * the input buffer boundary
		 */
		*status = rpc_s_ss_invalid_char_input;
		break;

	case 2: /* an input character does not belong to the converted
		   from characters */
		*status = rpc_s_ss_invalid_char_input;
		break;

	case 3: /* no room in the output buffer to place the converted
		   character */
		*status = rpc_s_ss_short_conv_buffer;
		break;

	default: /* not reachable */
		*status = rpc_s_ss_incompatible_codesets;
		break;
	}

	if ((i_ret = iconvclose(cd)) == -1)
		*status = rpc_s_ss_iconv_error;

	if (size)
		free((unsigned char *)table);

	if (conv_p_w_data_len != NULL)
		*conv_p_w_data_len = init_len - inbytesleft;

	return;
}



INTERNAL
void rpc__cs_get_iconv_name
#ifdef _DCE_PROTOTYPE_
(
	unsigned32		tag,
	char			**iconv_name,
	error_status_t		*status
)
#else
(tag, iconv_name, status)
	unsigned32		tag;
	char			**iconv_name;
	error_status_t		*status;
#endif
{
	char			*rgy_name;
	codeset_iconv_name_t	**mapping_table;

	/* mapping_table = (codeset_iconv_name_t **)codeset_iconv_table;
	*/

	dce_cs_rgy_to_loc (
		tag,
		(idl_char **)&rgy_name,
		NULL,
		NULL,
		status );

	if (*status != dce_cs_c_ok)
		return;


	name_binary_search (
		(codeset_iconv_name_t *)codeset_iconv_table,
		0,
		CODESET_ICONV_TABLE_ENTRY,
		rgy_name,
		iconv_name
	);

	*status = rpc_s_ok;
}
