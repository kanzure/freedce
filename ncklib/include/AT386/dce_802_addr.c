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
 * dce_get_802_addr
 *
 * Return the IEEE 802 (ie, ethernet hardware) address.
 */

#include <commonp.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dce/dce.h>
#include <dce/dce_utils.h>

void
dce_get_802_addr (
    dce_802_addr_t	*addr,
    error_status_t	*st
)
{
    int		fd;
    char	buf[13];
    int		e[6];
    int		i;

    *st = error_status_ok;

    fd = open (IEEE_802_FILE, O_RDONLY);
    if (fd < 0) {
        DIE ("(dce_get_802_addr) ABORTING, can't open "
             IEEE_802_FILE);
        *st = -1;
	return;
    }

    if (read (fd, buf, 12) < 12) {
        DIE ("(dce_get_802_addr) ABORTING, < 12 hex digits in "
             IEEE_802_FILE);
        *st = -1;
	return;
    }
    close(fd);

    buf[12] = 0;

    if (sscanf (buf, "%2x%2x%2x%2x%2x%2x", 
		&e[0], &e[1], &e[2], &e[3], &e[4], &e[5]) != 6) {
        DIE ("(dce_get_802_addr) ABORTING, parsing "
             IEEE_802_FILE);
        *st = -1;
	return;
    }

    for (i = 0; i < 6; i++)
       addr->eaddr[i] = e[i];
}

