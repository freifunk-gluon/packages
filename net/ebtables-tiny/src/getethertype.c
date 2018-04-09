/*
* getethertype.c
*
* This file was part of the NYS Library.
*
** The NYS Library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/********************************************************************
* Description: Ethertype name service switch and the ethertypes
* database access functions
* Author: Nick Fedchik <fnm@ukrsat.com>
* Checker: Bart De Schuymer <bdschuym@pandora.be>
* Origin: uClibc-0.9.16/libc/inet/getproto.c
* Created at: Mon Nov 11 12:20:11 EET 2002
********************************************************************/

#include <stddef.h>
#include <strings.h>

#include "include/ethernetdb.h"


static const struct ethertypeent ethertypes[] = {
	{"IPv4",     0x0800 }, /* Internet IP (IPv4) */
	{"X25",      0x0805 },
	{"ARP",      0x0806 },
	{"FR_ARP",   0x0808 }, /* Frame Relay ARP        [RFC1701] */
	{"BPQ",      0x08FF }, /* G8BPQ AX.25 Ethernet Packet */
	{"DEC",      0x6000 }, /* DEC Assigned proto */
	{"DNA_DL",   0x6001 }, /* DEC DNA Dump/Load */
	{"DNA_RC",   0x6002 }, /* DEC DNA Remote Console */
	{"DNA_RT",   0x6003 }, /* DEC DNA Routing */
	{"LAT",      0x6004 }, /* DEC LAT */
	{"DIAG",     0x6005 }, /* DEC Diagnostics */
	{"CUST",     0x6006 }, /* DEC Customer use */
	{"SCA",      0x6007 }, /* DEC Systems Comms Arch */
	{"TEB",      0x6558 }, /* Trans Ether Bridging   [RFC1701] */
	{"RAW_FR",   0x6559 }, /* Raw Frame Relay        [RFC1701] */
	{"RARP",     0x8035 }, /* Reverse ARP            [RFC903] */
	{"AARP",     0x80F3 }, /* Appletalk AARP */
	{"ATALK",    0x809B }, /* Appletalk */
	{"802_1Q",   0x8100 }, /* 802.1Q Virtual LAN tagged frame */
	{"IPX",      0x8137 }, /* Novell IPX */
	{"NetBEUI",  0x8191 }, /* NetBEUI */
	{"IPv6",     0x86DD }, /* IP version 6 */
	{"PPP",      0x880B }, /* PPP */
	{"ATMMPOA",  0x884C }, /* MultiProtocol over ATM */
	{"PPP_DISC", 0x8863 }, /* PPPoE discovery messages */
	{"PPP_SES",  0x8864 }, /* PPPoE session messages */
	{"ATMFATE",  0x8884 }, /* Frame-based ATM Transport over Ethernet */
	{"LOOP",     0x9000 }, /* loop proto */
	{}
};


const struct ethertypeent *getethertypebyname(const char *name)
{
	size_t i;
	for (i = 0; ethertypes[i].e_name; i++)
		if (strcasecmp(ethertypes[i].e_name, name) == 0)
			return &ethertypes[i];

	return NULL;
}

const struct ethertypeent *getethertypebynumber(int type)
{
	size_t i;
	for (i = 0; ethertypes[i].e_name; i++)
		if (ethertypes[i].e_ethertype == type)
			return &ethertypes[i];

	return NULL;
}
