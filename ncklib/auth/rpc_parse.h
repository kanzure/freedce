/* 
   Unix SMB/Netbios implementation.
   Version 1.9.
   SMB parameters and setup
   Copyright (C) Andrew Tridgell              1992-2000
   Copyright (C) Luke Kenneth Casson Leighton 1996-2000
   Copyright (C) Elrond                            2000
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _RPC_PARSE_H
#define _RPC_PARSE_H 


#define RPC_IO_DECLARE(fn, t, lds) \
	BOOL fn(const char *desc, t *lds, prs_struct *ps, int depth)
#define RPC_MARSHALLER_INTRO_NOALIGN(name, stru) \
	if (! stru) \
		return False; \
	prs_set_depth(ps, depth); \
	prs_debug(ps, -1, desc, name); \
	prs_inc_depth(ps); \
	depth = prs_depth(ps)
#define RPC_MARSHALLER_INTRO(name, stru) \
	RPC_MARSHALLER_INTRO_NOALIGN(name, stru); prs_align(ps)
#define RPC_MARSH_SUBCALL(fn, lds, field) \
	(fn(#field, &(lds->field), ps, depth))
#define RPC_MARSH_SUBCALLP(fn, lds, field) \
	(fn(#field, lds->field, ps, depth))
#define RPC_MARSH_SUBCALLP_ALLOC1(fn, t, lds, field) \
	( \
	  ( \
	    (UNMARSHALLING(ps) \
	     ? ((lds->field = g_new1(t)) != NULL) \
	     : (lds->field != NULL) \
	    ) || ( \
		DEBUG(1, ("RPC_MARSH_SUBCALLP_ALLOC1(%s) had %s==NULL\n", \
			  #fn, #field)), \
		False \
	    ) \
	  ) && \
	    RPC_MARSH_SUBCALLP(fn, lds, field) \
	)


#endif /* _RPC_PARSE_H */
