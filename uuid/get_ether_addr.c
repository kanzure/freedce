//
// dce_802_addr.c for Linux 2.x
// 
// Jim Doyle, Boston University, Jan 17 1998
//
// Fetch a IEEE 802 MAC Address for building UUID's
//
//
//
//

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#ifndef UUID_BUILD_STANDALONE
#include <dce/dce.h>
#include <dce/dce_utils.h>
#else
#include "uuid.h"
#endif
#include <net/if.h>
#include <sys/ioctl.h>          
#include <stdlib.h>

//
// Invalid Host MAC Addresses
//

static  char null_802_hwaddr[6] = {0, 0, 0, 0, 0, 0};
/*static  char broadcast_802_hwaddr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; */

static void linux_get_802_addr(dce_802_addr_t *, error_status_t *);
static void file_get_802_addr(dce_802_addr_t *, error_status_t *);

void
dce_get_802_addr (
    dce_802_addr_t	*addr,
    error_status_t	*st
)
{

  // Try to get a MAC address from one of our Ethernet interfaces

  linux_get_802_addr(addr, st);

  // If there are no Ethernet-style interfaces, fall-back and try to
  // fetch the address from a file.

  if (*st != error_status_ok)
    {
      file_get_802_addr(addr, st);
    }  
}

//
// Fetch a valid MAC address from one of the hosts Ethernet interfaces
//

static void 
linux_get_802_addr(
    dce_802_addr_t	*addr,
    error_status_t	*st
    )
{
  int    afinet_socket;

  struct ifconf           ifc;
  struct ifreq            *ifr, *last_ifr;
  struct ifreq            ifreq;
  unsigned char           buf[1024]; 
  int                     n_ifs, i; 
  int                     prev_size = sizeof(struct ifreq); 

  char * this_ifc_name;
  int this_ifc_flags;
  int this_ifc_hashwaddr;
  char this_ifc_hwaddr[6];
  int continue_search;

  *st = utils_s_802_cant_read;

  //
  // Open an INET socket
  //

  afinet_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (afinet_socket < 0)
    {
      printf ("Cant open socket\n");
      exit(1);
    }

  //
  // Get the list of network interfaces
  //

  ifc.ifc_len = sizeof (buf);
  ifc.ifc_buf = (caddr_t) buf;

  if (ioctl (afinet_socket, (int) SIOCGIFCONF, (caddr_t) &ifc) < 0)
    {                                                 
      printf ("ioctl(SIOCGIFCONF) failed\n");
      exit(1);

    }

  n_ifs = ifc.ifc_len / sizeof (struct ifreq); 
  last_ifr = (struct ifreq *) (ifc.ifc_buf + ifc.ifc_len);  

  //
  // Find an interface that has a valid IEEE 802 address
  //
  // Iterate of the list of the system's netif. Find the first
  // active interface that has an Ethernet address ; use it.
  //

  continue_search = 1;

  for (i=0, ifr = ifc.ifc_req; 
       (ifr < last_ifr) && continue_search;
       i++, ifr = (struct ifreq *)(( (char *) ifr ) + prev_size))          
    {
      this_ifc_name = ifr->ifr_ifrn.ifrn_name;



      //
      // Get Flags for this interface
      //
      
      memcpy(&ifreq, ifr, sizeof(ifreq)); 
      if (ioctl (afinet_socket,  SIOCGIFFLAGS, &ifreq) < 0)
	{
	  printf("Error ioctl(SIOCGIFFLAGS)\n");
	  exit(1);
	}

      this_ifc_flags = ifreq.ifr_ifru.ifru_flags;

      //
      // Get Hardware address for this interface
      //

      memcpy(&ifreq, ifr, sizeof(ifreq)); 
      this_ifc_hashwaddr = 1;
      if (ioctl (afinet_socket,  SIOCGIFHWADDR, &ifreq) < 0)
	{
	  this_ifc_hashwaddr = 0;
	}

      continue_search = this_ifc_flags &  (IFF_LOOPBACK | IFF_POINTOPOINT);

      if (this_ifc_hashwaddr)
	{
       	  memcpy(this_ifc_hwaddr, &ifreq.ifr_hwaddr.sa_data, 6);
	}

    }

  continue_search &= memcmp(this_ifc_hwaddr, null_802_hwaddr, 6);
  continue_search &= memcmp(this_ifc_hwaddr, null_802_hwaddr, 6);

  if (continue_search == 0)
    {
       addr->eaddr[0] = this_ifc_hwaddr[0] & 0xff;
       addr->eaddr[1] = this_ifc_hwaddr[1] & 0xff;
       addr->eaddr[2] = this_ifc_hwaddr[2] & 0xff;
       addr->eaddr[3] = this_ifc_hwaddr[3] & 0xff;
       addr->eaddr[4] = this_ifc_hwaddr[4] & 0xff;
       addr->eaddr[5] = this_ifc_hwaddr[5] & 0xff;

       *st = error_status_ok;
    }


  close(afinet_socket);

}


//
// Fetch a dummy MAC address from the file /etc/ieee_802_addr
//

static void
file_get_802_addr (
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
	*st = utils_s_802_cant_read;
	return;
    }

    if (read (fd, buf, 12) < 12) {
	*st = utils_s_802_cant_read;
	return;
    }
    close(fd);

    buf[12] = 0;

    if (sscanf (buf, "%2x%2x%2x%2x%2x%2x", 
		&e[0], &e[1], &e[2], &e[3], &e[4], &e[5]) != 6) {
	*st = utils_s_802_addr_format;
	return;
    }

    for (i = 0; i < 6; i++)
       addr->eaddr[i] = e[i];
}

