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

#include <sys/fcntl.h>
#include <netio.h>

#include <dce/dce.h>
#include <dce/dce_utils.h>

#define LAN_DEV0 "/dev/lan0"

void
dce_get_802_addr (
    dce_802_addr_t	*addr,
    error_status_t	*st
)
{
    int		fd;
    struct fis	iocnt_block;
    int		i;
    char	net_buf[sizeof(LAN_DEV0)+1];
    char	*p;

    (void)sprintf(net_buf, "%s", LAN_DEV0);
    p = net_buf + strlen(net_buf) - 1;

    /* 
     * Get 802.3 address from card by opening the driver and interrogating it.
     */
    for (i = 0; i < 10; i++, (*p)++) {
        if ((fd = open (net_buf, O_RDONLY)) != -1) {
	    iocnt_block.reqtype = LOCAL_ADDRESS;
	    ioctl (fd, NETSTAT, &iocnt_block);
	    close (fd);

            if (iocnt_block.vtype == 6)
                break;
        }
    }

    if (fd == -1 || iocnt_block.vtype != 6) {
        *st = utils_s_802_cant_read;
        return;
    }

    for (i = 0; i < 6; i++)
       addr->eaddr[i] = iocnt_block.value.s[i];

    *st = error_status_ok;
}

