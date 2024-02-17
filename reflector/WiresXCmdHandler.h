//  Copyright © 2019 Jean-Luc Deltombe (LX3JL). All rights reserved.

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

#include "WiresXInfo.h"
#include "WiresXCmdQueue.h"
#include "WiresXPacketQueue.h"

////////////////////////////////////////////////////////////////////////////////////////

#define WIRESX_REPLY_DELAY  (1.000)

////////////////////////////////////////////////////////////////////////////////////////
// class

class CWiresxCmdHandler
{
public:
	// constructor
	CWiresxCmdHandler();

	// destructor
	~CWiresxCmdHandler();

	// initialization
	bool Init(void);
	void Close(void);

	// queues
	CWiresxCmdQueue *GetCmdQueue(void)          { m_CmdQueue.Lock(); return &m_CmdQueue; }
	void ReleaseCmdQueue(void)                  { m_CmdQueue.Unlock(); }
	CWiresxPacketQueue *GetPacketQueue(void)    { m_PacketQueue.Lock(); return &m_PacketQueue; }
	void ReleasePacketQueue(void)               { m_PacketQueue.Unlock(); }

	// get

	// task
	void Thread(void);
	void Task(void);

protected:
	// packet encoding
	bool ReplyToWiresxDxReqPacket(const CIp &, const CWiresxInfo &, char);
	bool ReplyToWiresxAllReqPacket(const CIp &, const CWiresxInfo &, int);
	bool ReplyToWiresxConnReqPacket(const CIp &, const CWiresxInfo &, char);
	bool ReplyToWiresxDiscReqPacket(const CIp &, const CWiresxInfo &);

	// packet encoding helpers
	bool EncodeAndSendWiresxPacket(const CIp &, const CBuffer &, const CWiresxInfo &);
	uint8_t WiresxCalcFt(uint, uint) const;
	void SendPacket(const CIp &, uint8_t *);

	// debug
	bool DebugTestDecodePacket(const CBuffer &);

protected:
	// data
	CWiresxInfo        m_ReflectorWiresxInfo;
	uint8_t            m_seqNo;

	// queues
	CWiresxCmdQueue    m_CmdQueue;
	CWiresxPacketQueue m_PacketQueue;

	// thread
	std::atomic<bool> keep_running;
	std::future<void> m_Future;
};
