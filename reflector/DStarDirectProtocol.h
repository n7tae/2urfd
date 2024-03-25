// urfd -- The universal reflector
// Copyright © 2024 Thomas A. Early N7TAE
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
#include "Protocol.h"
#include "DVHeaderPacket.h"
#include "DVFramePacket.h"
#include "DSDAction.h"
#include "Random.h"

class CDStarDirectProtocol : public CProtocol
{
public:
	// constructor
	CDStarDirectProtocol(const std::string &);

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
	bool IsValidKeepAlivePacket(  const CBuffer &, CCallsign *);
	bool IsValidDvHeaderPacket(   const CBuffer &, std::unique_ptr<CDvHeaderPacket> &);
	bool IsValidDvFramePacket(    const CBuffer &, std::unique_ptr<CDvFramePacket> &);

	// packet encoding helpers
	void EncodeKeepAlivePacket(CBuffer *);
	bool EncodeDvHeaderPacket(const CDvHeaderPacket &, CBuffer &) const;
	bool EncodeDvFramePacket(const CDvFramePacket &, CBuffer &) const;

	// response action
	void SendResponse(CDSDAction &);
	void DoAction(CDSDAction &);
	void CheckActionMap(void);

	const CCallsign m_IRCLogin;

	// time
	CTimer m_LastKeepaliveTime;

	// action
	std::unordered_map<uint16_t, CDSDAction> m_ActionMap;

	// misc
	CRandom m_Random;
};
