/* 
 *  Unix SMB/Netbios implementation.
 *  Version 1.9.
 *  RPC Pipe client / server routines
 *  Copyright (C) Andrew Tridgell              1992-2000,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-2000,
 *  Copyright (C) Paul Ashton                  1997-2000,
 *  Copyright (C) Sander Striker                    2000
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
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/* MKPROTO: rpc_parse_proto.h		(do not remove this line) */

#include <dg.h>
#include <dce/rpc.h>
#include <dce/conv.h>
#include <dce/stubbase.h>


#include "includes.h"

/*******************************************************************
 Reads or writes a DOM_GID structure.
********************************************************************/

RPC_IO_DECLARE(smb_io_gid, DOM_GID, gid)
{
	if (gid == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_gid");
	depth++;

	prs_align(ps);
	
	prs_uint32("g_rid", ps, depth, &(gid->g_rid));
	prs_uint32("attr ", ps, depth, &(gid->attr ));

	return True;
}
/*******************************************************************
 Reads or writes a DOM_SID structure.
********************************************************************/

RPC_IO_DECLARE(smb_io_dom_sid, DOM_SID, sid)
{
	int i;

	if (sid == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_dom_sid");
	depth++;

	prs_align(ps);
	
	prs_uint8 ("sid_rev_num", ps, depth, &(sid->sid_rev_num)); 
	prs_uint8 ("num_auths  ", ps, depth, &(sid->num_auths));

	for (i = 0; i < 6; i++)
	{
		fstring tmp;
		slprintf(tmp, sizeof(tmp) - 1, "id_auth[%d] ", i);
		prs_uint8 (tmp, ps, depth, &(sid->id_auth[i]));
	}

	/* oops! XXXX should really issue a warning here... */
	if (sid->num_auths > MAXSUBAUTHS) sid->num_auths = MAXSUBAUTHS;

	prs_uint32s(False, "sub_auths ", ps, depth, sid->sub_auths, sid->num_auths);

	return True;
}

/*******************************************************************
 Reads or writes a DOM_SID2 structure.
********************************************************************/

RPC_IO_DECLARE(smb_io_dom_sid2, DOM_SID2, sid)
{
	if (sid == NULL)
		return False;

	prs_debug(ps, depth, desc, "smb_io_dom_sid2");
	depth++;

	prs_align(ps);
	
	prs_uint32("num_auths", ps, depth, &(sid->num_auths));

	return smb_io_dom_sid("sid", &(sid->sid), ps, depth);
}


RPC_IO_DECLARE(net_io_user_info3, NET_USER_INFO_3, usr)
{
	uint32 i;

	if (usr == NULL)
		return False;

	prs_debug(ps, depth, desc, "net_io_user_info3");
	depth++;

	prs_align(ps);

	RPC_MARSH_SUBCALL(smb_io_time, usr, logon_time);	/* logon time */
	RPC_MARSH_SUBCALL(smb_io_time, usr, logoff_time);	/* logoff time */
	RPC_MARSH_SUBCALL(smb_io_time, usr, kickoff_time);	/* kickoff time */
	RPC_MARSH_SUBCALL(smb_io_time, usr, pass_last_set_time);	/* password last set time */
	RPC_MARSH_SUBCALL(smb_io_time, usr, pass_can_change_time);	/* password can change time */
	RPC_MARSH_SUBCALL(smb_io_time, usr, pass_must_change_time);	/* password must change time */

	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_user_name);	/* username unicode string header */
	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_full_name);	/* user's full name unicode string header */
	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_logon_script);	/* logon script unicode string header */
	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_profile_path);	/* profile path unicode string header */
	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_home_dir);	/* home directory unicode string header */
	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_dir_drive);	/* home directory drive unicode string header */

	prs_uint16("logon_count   ", ps, depth, &(usr->logon_count));	/* logon count */
	prs_uint16("bad_pw_count  ", ps, depth, &(usr->bad_pw_count));	/* bad password count */

	prs_uint32("user_id       ", ps, depth, &(usr->user_id));	/* User ID */
	prs_uint32("group_id      ", ps, depth, &(usr->group_id));	/* Group ID */
	prs_uint32("num_groups    ", ps, depth, &(usr->num_groups));	/* num groups */
	prs_uint32("buffer_groups ", ps, depth, &(usr->buffer_groups));	/* undocumented buffer pointer to groups. */
	prs_uint32("user_flgs     ", ps, depth, &(usr->user_flgs));	/* user flags */

	prs_uint8s(False, "user_sess_key", ps, depth, usr->user_sess_key, 16);	/* unused user session key */

	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_logon_srv);	/* logon server unicode string header */
	RPC_MARSH_SUBCALL(smb_io_unihdr, usr, hdr_logon_dom);	/* logon domain unicode string header */

	prs_uint32("buffer_dom_id ", ps, depth, &(usr->buffer_dom_id));	/* undocumented logon domain id pointer */
	prs_uint8s(False, "padding       ", ps, depth, usr->padding, 40);	/* unused padding bytes? */

	prs_uint32("num_other_sids", ps, depth, &(usr->num_other_sids));	/* 0 - num_sids */
	prs_uint32("buffer_other_sids", ps, depth, &(usr->buffer_other_sids));	/* NULL - undocumented pointer to SIDs. */

	smb_io_unistr2("user_name", &(usr->uni_user_name),
		       usr->hdr_user_name.buffer, ps, depth);	/* username unicode string */
	prs_align(ps);
	smb_io_unistr2("full_name", &(usr->uni_full_name),
		       usr->hdr_full_name.buffer, ps, depth);	/* user's full name unicode string */
	prs_align(ps);
	smb_io_unistr2("logon_script", &(usr->uni_logon_script),
		       usr->hdr_logon_script.buffer, ps, depth);	/* logon script unicode string */
	prs_align(ps);
	smb_io_unistr2("profile_path", &(usr->uni_profile_path),
		       usr->hdr_profile_path.buffer, ps, depth);	/* profile path unicode string */
	prs_align(ps);
	smb_io_unistr2("home_dir", &(usr->uni_home_dir),
		       usr->hdr_home_dir.buffer, ps, depth);	/* home directory unicode string */
	prs_align(ps);
	smb_io_unistr2("dir_drive", &(usr->uni_dir_drive),
		       usr->hdr_dir_drive.buffer, ps, depth);	/* home directory drive unicode string */
	prs_align(ps);

	prs_uint32("num_groups2   ", ps, depth, &(usr->num_groups2));	/* num groups */
	/*SMB_ASSERT_ARRAY(usr->gids, (size_t)usr->num_groups2);*/
	for (i = 0; i < usr->num_groups2; i++)
	{
		RPC_MARSH_SUBCALL(smb_io_gid, usr, gids[i]);
	}

	smb_io_unistr2("logon_srv", &(usr->uni_logon_srv),
		       usr->hdr_logon_srv.buffer, ps, depth);	/* logon server unicode string */
	prs_align(ps);
	smb_io_unistr2("logon_dom", &(usr->uni_logon_dom),
		       usr->hdr_logon_srv.buffer, ps, depth);	/* logon domain unicode string */
	prs_align(ps);

	RPC_MARSH_SUBCALL(smb_io_dom_sid2, usr, dom_sid);	/* domain SID */
	prs_align(ps);

	/*SMB_ASSERT_ARRAY(usr->other_sids, (size_t)usr->num_other_sids);*/

	for (i = 0; i < usr->num_other_sids; i++)
	{
		RPC_MARSH_SUBCALL(smb_io_dom_sid2, usr, other_sids[i]);
		prs_align(ps);
	}

	return True;
}

