//  Copyright © 2020 Marius Petrescu (YO2LOJ). All rights reserved.

// urfd -- The universal reflector
// Copyright © 2021 Thomas A. Early N7TAE
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include "IP.h"
#include "Buffer.h"

////////////////////////////////////////////////////////////////////////////////////////
// define

#define RAW_BUFFER_LENMAX 65536


////////////////////////////////////////////////////////////////////////////////////////
// class

class CRawSocket
{
public:
	// constructor
	CRawSocket();

	// destructor
	~CRawSocket();

	// open & close
	bool Open(uint16_t);
	void Close(void);
	int  GetSocket(void)        { return m_Socket; }

	// read

	// if ETH_P_ALL is used, the received data buffer will hold
	// the ethernet header (struct ethhdr) followed by the IP header (struct iphdr),
	// the protocol header (e.g tcp, udp, icmp) and the data.
	// For specific protocols, the data content may vary depending on the protocol
	// Returns the number of received bytes in buffer

protected:
	int Receive(uint8_t *, CIp *, int);

	// ICMP receive helper
	// parameters:
	//   buffer - packet receive buffer (starting with ip header)
	//   ip - remote address (filled in on receive)
	//   timeout - receive timeout in msec
	// return value:
	//   ICMP type, -1 if nothing was received

public:
	int IcmpReceive(uint8_t *, CIp *, int);

	// write
	// no write support - complexity makes it out of scope for now
	// to be added if needed

protected:
	// data
	int                 m_Socket;
	int                 m_Proto;
	struct sockaddr_in  m_SocketAddr;
};
