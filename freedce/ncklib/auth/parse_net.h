#ifndef __PARSE_NET_H__
#define __PARSE_NET_H__

#ifndef MAXSUBAUTHS
#define MAXSUBAUTHS 15          /* max sub authorities in a SID */
#endif

/* some hardcoded stuff */
#define LSA_MAX_GROUPS 32
#define LSA_MAX_SIDS 32
        

/* DOM_SID - security id */
typedef struct sid_info
{
	uint8 sid_rev_num;	/* SID revision number */
	uint8 num_auths;	/* number of sub-authorities */
	uint8 id_auth[6];	/* Identifier Authority */
	/*
	 * Note that the values in these uint32's are in *native* byteorder,
	 * not neccessarily little-endian...... JRA.
	 */
	uint32 sub_auths[MAXSUBAUTHS];	/* pointer to sub-authorities. */

} DOM_SID;

/* DOM_GID - group id + user attributes */
typedef struct gid_info
{
	uint32 g_rid;           /* a group RID */
	uint32 attr;
}
DOM_GID;

/* DOM_SID2 - security id */
typedef struct sid_info_2
{
	uint32 num_auths;	/* length, bytes, including length of len :-) */

	DOM_SID sid;

}
DOM_SID2;

/* NET_USER_INFO_3 */
typedef struct net_user_info_3
{
	NTTIME logon_time;            /* logon time */
	NTTIME logoff_time;           /* logoff time */
	NTTIME kickoff_time;          /* kickoff time */
	NTTIME pass_last_set_time;    /* password last set time */
	NTTIME pass_can_change_time;  /* password can change time */
	NTTIME pass_must_change_time; /* password must change time */

	UNIHDR hdr_user_name;    /* username unicode string header */
	UNIHDR hdr_full_name;    /* user's full name unicode string header */
	UNIHDR hdr_logon_script; /* logon script unicode string header */
	UNIHDR hdr_profile_path; /* profile path unicode string header */
	UNIHDR hdr_home_dir;     /* home directory unicode string header */
	UNIHDR hdr_dir_drive;    /* home drive unicode string header */

	uint16 logon_count;  /* logon count */
	uint16 bad_pw_count; /* bad password count */

	uint32 user_id;       /* User ID */
	uint32 group_id;      /* Group ID */
	uint32 num_groups;    /* num groups */
	uint32 buffer_groups; /* undocumented buffer pointer to groups. */
	uint32 user_flgs;     /* user flags */

	uint8 user_sess_key[16]; /* user session key */

	UNIHDR hdr_logon_srv; /* logon server unicode string header */
	UNIHDR hdr_logon_dom; /* logon domain unicode string header */

	uint32 buffer_dom_id; /* undocumented logon domain id pointer */
	uint8 padding[40];    /* expansion room */

	uint32 num_other_sids; /* 0 - num_sids */
	uint32 buffer_other_sids; /* NULL - undocumented pointer to SIDs. */
	
	UNISTR2 uni_user_name;    /* username unicode string */
	UNISTR2 uni_full_name;    /* user's full name unicode string */
	UNISTR2 uni_logon_script; /* logon script unicode string */
	UNISTR2 uni_profile_path; /* profile path unicode string */
	UNISTR2 uni_home_dir;     /* home directory unicode string */
	UNISTR2 uni_dir_drive;    /* home directory drive unicode string */

	uint32 num_groups2;        /* num groups */
	DOM_GID gids[LSA_MAX_GROUPS]; /* group info */

	UNISTR2 uni_logon_srv; /* logon server unicode string */
	UNISTR2 uni_logon_dom; /* logon domain unicode string */

	DOM_SID2 dom_sid;           /* domain SID */
	DOM_SID2 other_sids[LSA_MAX_SIDS]; /* undocumented - domain SIDs */

} NET_USER_INFO_3;

#endif /* __PARSE_NET_H__ */
