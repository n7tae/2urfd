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

#include <memory>
#include <queue>
#include <list>

#include "Timer.h"
#include "DVFramePacket.h"
#include "DVHeaderPacket.h"
#include "TranscoderPacket.h"
#include "SafePacketQueue.h"
#include "Client.h"

#define STREAM_TIMEOUT      (1.600)

struct STCFP
{
	STCFP() : tcpacket(nullptr), fpacket(nullptr) {}
	std::shared_ptr<CTranscoderPacket> tcpacket;
	std::unique_ptr<CDvFramePacket> fpacket;
};

class CPacketStream
{
public:
	CPacketStream(char module, bool istc);

	void ResetStats();
	void ReportStats();

	// open / close
	bool OpenPacketStream(const CDvHeaderPacket &, std::shared_ptr<CClient>);
	void ClosePacketStream(void);

	void Tickle(void)                               { m_LastPacketTime.start(); }
	void Update(double rt);

	// get
	std::shared_ptr<CClient> GetOwnerClient(void)   { return m_OwnerClient; }
	const CIp       *GetOwnerIp(void);
	bool             IsExpired(void) const          { return (m_LastPacketTime.time() > STREAM_TIMEOUT); }
	bool             IsOpen(void) const             { return m_uiStreamId != 0; }
	uint16_t         GetStreamId(void) const        { return m_uiStreamId; }
	const CCallsign &GetUserCallsign(void) const    { return m_DvHeader.GetMyCallsign(); }
	char             GetRpt2Module(void) const      { return m_DvHeader.GetRpt2Module(); }
	char             GetModule(void) const          { return m_PSModule; }

	// push & pop
	void Push(std::unique_ptr<CPacket> packet);
	std::unique_ptr<CPacket> Pop()        { return m_Queue.Pop(); }
	std::unique_ptr<CPacket> PopWait()    { return m_Queue.PopWait(); }
	bool IsEmpty()                        { return m_Queue.IsEmpty(); }

protected:
	// data
	const char      m_PSModule;
	const bool      m_IsTranscoded;
	uint16_t        m_uiStreamId;
	uint32_t        m_uiPacketCntr;
	CTimer          m_LastPacketTime;
	CDvHeaderPacket m_DvHeader;
	std::shared_ptr<CClient> m_OwnerClient;
	CSafePacketQueue<std::unique_ptr<CPacket>> m_Queue;
	std::list<STCFP> m_TCQueue;
	std::list<std::unique_ptr<CDvHeaderPacket>> m_HeaderQueue;
	double m_RTMin, m_RTMax, m_RTSum, m_RTSumSq;
};
