//  Copyright © 2015 Jean-Luc Deltombe (LX3JL). All rights reserved.

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

#include "Defines.h"
#include "Timer.h"
#include "SEProtocol.h"
#include "DVHeaderPacket.h"
#include "DVFramePacket.h"

////////////////////////////////////////////////////////////////////////////////////////

// note on protocol revisions:
//
//  rev 0:
//      this is standard protocol implementation
//
//  rev 1:
//      this is specific UP4DAR umplementation
//      the protocol is detected using byte(10) of connect packet (value is 11)
//      the protocol require a specific non-standard disconnect acqknowleding packet
//
//  rev 2:
//      this is specific to KI4KLF dxrfd reflector
//      the protocol is detected by looking at "XRF" in connect packet callsign
//      the protocol require a specific connect ack packet
//      the protocol also implement a workaround for detecting stream's module
//          as dxrfd does not set DV header RPT2 properly.
//      the protocol assumes that a dxrfd can only be linked to one module at a time


////////////////////////////////////////////////////////////////////////////////////////
// class

class CDextraProtocol : public CSEProtocol
{
public:
	// initialization
	bool Initialize(const char *type, const EProtocol ptype, const uint16_t port, const bool has_ipv4, const bool has_ipv6);

	// task
	void Task(void);

protected:
	// queue helper
	void HandleQueue(void);

	// keepalive helpers
	void HandleKeepalives(void);

	// stream helpers
	void OnDvHeaderPacketIn(std::unique_ptr<CDvHeaderPacket> &, const CIp &);

	// packet decoding helpers
	bool IsValidConnectPacket(    const CBuffer &, CCallsign &, char &, EProtoRev &);
	bool IsValidDisconnectPacket( const CBuffer &, CCallsign *);
	bool IsValidKeepAlivePacket(  const CBuffer &, CCallsign *);
	bool IsValidDvHeaderPacket(   const CBuffer &, std::unique_ptr<CDvHeaderPacket> &);
	bool IsValidDvFramePacket(    const CBuffer &, std::unique_ptr<CDvFramePacket> &);

	// packet encoding helpers
	void EncodeKeepAlivePacket(CBuffer *);
	void EncodeConnectPacket(CBuffer *, const char *);
	void EncodeConnectAckPacket(CBuffer *, EProtoRev);
	void EncodeConnectNackPacket(CBuffer *);
	void EncodeDisconnectPacket(CBuffer *, char);
	void EncodeDisconnectedPacket(CBuffer *);
	bool EncodeDvHeaderPacket(const CDvHeaderPacket &, CBuffer &) const;
	bool EncodeDvFramePacket(const CDvFramePacket &, CBuffer &) const;

	// time
	CTimer m_LastKeepaliveTime;
};
