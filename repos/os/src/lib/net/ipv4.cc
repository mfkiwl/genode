/*
 * \brief  Internet protocol version 4
 * \author Stefan Kalkowski
 * \date   2010-08-19
 */

/*
 * Copyright (C) 2010-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <util/token.h>
#include <util/string.h>

#include <net/ipv4.h>

using namespace Net;

struct Scanner_policy_number
{
		static bool identifier_char(char c, unsigned  i ) {
			return Genode::is_digit(c) && c !='.'; }
};

typedef ::Genode::Token<Scanner_policy_number> Token;


Ipv4_packet::Ipv4_address Ipv4_packet::ip_from_string(const char *ip)
{
	Ipv4_address  ip_addr;
	Token         t(ip);
	char          tmpstr[4];
	int           cnt = 0;
	unsigned char ipb[4] = {0};

	while(t) {
		if (t.type() == Token::WHITESPACE || t[0] == '.') {
			t = t.next();
			continue;
		}
		t.string(tmpstr, sizeof(tmpstr));

		unsigned long tmpc = 0;
		Genode::ascii_to(tmpstr, tmpc);
		ipb[cnt] = tmpc & 0xFF;
		t = t.next();

		if (cnt == 4)
			break;
		cnt++;
	}

	if (cnt == 4) {
		ip_addr.addr[0] = ipb[0];
		ip_addr.addr[1] = ipb[1];
		ip_addr.addr[2] = ipb[2];
		ip_addr.addr[3] = ipb[3];
	}

	return ip_addr;
}


const Ipv4_packet::Ipv4_address Ipv4_packet::CURRENT((Genode::uint8_t)0x00);
const Ipv4_packet::Ipv4_address Ipv4_packet::BROADCAST((Genode::uint8_t)0xFF);
