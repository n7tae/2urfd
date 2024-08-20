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

#include <iostream>
#include "Packet.h"

// default constructor
CPacket::CPacket()
{
	m_uiStreamId = 0;
	m_uiDstarPacketId = 0;
	m_uiDmrPacketId = 0;
	m_uiDmrPacketSubid = 0;
	m_uiYsfPacketId = 0;
	m_uiYsfPacketSubId = 0;
	m_uiYsfPacketFrameId = 0;
	m_uiNXDNPacketId = 0;
	m_uiM17FrameNumber = 0;
	m_cModule = ' ';
	m_eOrigin = EOrigin::local;
	m_eCodecIn = ECodecType::none;
	m_bLastPacket = false;
};

// for the network
unsigned int CPacket::GetNetworkSize()
{
	return 20u;
}

CPacket::CPacket(const CBuffer &buf)
{
	if (buf.size() > 19)
	{
		auto data = buf.data();
		m_eCodecIn           = (ECodecType)data[4];
		m_eOrigin            = (EOrigin)data[5];
		m_bLastPacket        = data[6] ? true : false;
		m_cModule            = data[7];
		m_uiStreamId         = data[8]*0x100u + data[9];
		m_uiM17FrameNumber   = data[10]*0x1000000u + data[11]*0x10000u + data[12]*0x100 + data[13];
		m_uiDstarPacketId    = data[14];
		m_uiDmrPacketId      = data[15];
		m_uiDmrPacketSubid   = data[16];
		m_uiYsfPacketId      = data[17];
		m_uiYsfPacketSubId   = data[18];
		m_uiYsfPacketFrameId = data[19];
	}
	else
		std::cerr << "CPacket initialization failed because the buffer is too small!" << std::endl;
}

void CPacket::EncodeInterlinkPacket(const char *magic, CBuffer &buf) const
{
	buf.Set(magic);
	buf.resize(20);
	auto data = buf.data();
	data[4]  = (uint8_t)m_eCodecIn;
	data[5]  = (uint8_t)m_eOrigin;
	data[6]  = m_bLastPacket ? 1 : 0;
	data[7]  = m_cModule;
	data[8]  = m_uiStreamId / 0x100u;
	data[9]  = m_uiStreamId % 0x100u;
	data[10] = (m_uiM17FrameNumber >> 24) & 0xffu;
	data[11] = (m_uiM17FrameNumber >> 16) & 0xffu;
	data[12] = (m_uiM17FrameNumber >>  8) & 0xffu;
	data[13] = m_uiM17FrameNumber & 0xffu;
	data[14] = m_uiDstarPacketId;
	data[15] = m_uiDmrPacketId;
	data[16] = m_uiDmrPacketSubid;
	data[17] = m_uiYsfPacketId;
	data[18] = m_uiYsfPacketSubId;
	data[19] = m_uiYsfPacketFrameId;
}

// dstar constructor
CPacket::CPacket(uint16_t sid, uint8_t dstarpid)
{
	m_uiStreamId = sid;
	m_uiDstarPacketId = dstarpid;
	m_uiDmrPacketId = 0xFF;
	m_uiDmrPacketSubid  = 0xFF;
	m_uiYsfPacketId = 0xFF;
	m_uiYsfPacketSubId = 0xFF;
	m_uiYsfPacketFrameId = 0xFF;
	m_uiNXDNPacketId = 0xFF;
	m_uiM17FrameNumber = 0xFFFFFFFFU;
	m_cModule = ' ';
	m_eOrigin = EOrigin::local;
	m_eCodecIn = ECodecType::dstar;
	m_bLastPacket = (0x40U == (dstarpid & 0x40U));
};

// dmr constructor
CPacket::CPacket(uint16_t sid, uint8_t dmrpid, uint8_t dmrspid, bool lastpacket)
{
	m_uiStreamId = sid;
	m_uiDmrPacketId = dmrpid;
	m_uiDmrPacketSubid = dmrspid;
	m_uiDstarPacketId = 0xFF;
	m_uiYsfPacketId = 0xFF;
	m_uiYsfPacketSubId = 0xFF;
	m_uiYsfPacketFrameId = 0xFF;
	m_uiNXDNPacketId = 0xFF;
	m_uiM17FrameNumber = 0xFFFFFFFFU;
	m_cModule = ' ';
	m_eOrigin = EOrigin::local;
	m_eCodecIn = ECodecType::dmr;
	m_bLastPacket = lastpacket;
};

// ysf constructor
CPacket::CPacket(uint16_t sid, uint8_t ysfpid, uint8_t ysfsubpid, uint8_t ysffrid, bool lastpacket)
{
	m_uiStreamId = sid;
	m_uiYsfPacketId = ysfpid;
	m_uiYsfPacketSubId = ysfsubpid;
	m_uiYsfPacketFrameId = ysffrid;
	m_uiDstarPacketId = 0xFF;
	m_uiDmrPacketId = 0xFF;
	m_uiDmrPacketSubid = 0xFF;
	m_uiNXDNPacketId = 0xFF;
	m_uiM17FrameNumber = 0xFFFFFFFFU;
	m_cModule = ' ';
	m_eOrigin = EOrigin::local;
	m_eCodecIn = ECodecType::dmr;
	m_bLastPacket = lastpacket;
}

// nxdn constructor
CPacket::CPacket(uint16_t sid, uint8_t pid, bool lastpacket)
{
	m_uiStreamId = sid;
	m_uiNXDNPacketId = pid;
	m_uiDmrPacketId = 0xFF;
	m_uiDmrPacketSubid = 0xFF;
	m_uiDstarPacketId = 0xFF;
	m_uiYsfPacketId = 0xFF;
	m_uiYsfPacketSubId = 0xFF;
	m_uiYsfPacketFrameId = 0xFF;
	m_uiM17FrameNumber = 0xFFFFFFFFU;
	m_cModule = ' ';
	m_eOrigin = EOrigin::local;
	m_eCodecIn = ECodecType::dmr;
	m_bLastPacket = lastpacket;
};

// p25 constructor
CPacket::CPacket(uint16_t sid, bool lastpacket)
{
	m_uiStreamId = sid;
	m_uiDstarPacketId = 0xFF;
	m_uiDmrPacketId = 0xFF;
	m_uiDmrPacketSubid  = 0xFF;
	m_uiYsfPacketId = 0xFF;
	m_uiYsfPacketSubId = 0xFF;
	m_uiYsfPacketFrameId = 0xFF;
	m_uiNXDNPacketId = 0xFF;
	m_uiM17FrameNumber = 0xFFFFFFFFU;
	m_cModule = ' ';
	m_eOrigin = EOrigin::local;
	m_eCodecIn = ECodecType::p25;
	m_bLastPacket = lastpacket;
};

// bm  constructor
CPacket::CPacket(uint16_t sid, uint8_t dstarpid, uint8_t dmrpid, uint8_t dmrsubpid, uint8_t ysfpid, uint8_t ysfsubpid, uint8_t ysffrid, ECodecType codecIn, bool lastpacket)
{
	m_uiStreamId = sid;
	m_uiDstarPacketId = dstarpid;
	m_uiDmrPacketId = dmrpid;
	m_uiDmrPacketSubid  = dmrsubpid;
	m_uiYsfPacketId = ysfpid;
	m_uiYsfPacketSubId = ysfsubpid;
	m_uiYsfPacketFrameId = ysffrid;
	m_uiM17FrameNumber = 0xFFFFFFFFU;
	m_uiNXDNPacketId = 0xFF;
	m_cModule = ' ';
	m_eOrigin = EOrigin::local;
	m_eCodecIn = codecIn;
	m_bLastPacket = lastpacket;
}

// m17 constructor
CPacket::CPacket(const CM17Packet &m17) : CPacket()
{
	m_uiStreamId = m17.GetStreamId();
	m_uiDstarPacketId = 0xFF;
	m_uiDmrPacketId = 0xFF;
	m_uiDmrPacketSubid  = 0xFF;
	m_uiYsfPacketId = 0xFF;
	m_uiYsfPacketSubId = 0xFF;
	m_uiYsfPacketFrameId = 0xFF;
	m_uiNXDNPacketId = 0xFF;
	m_eCodecIn = (0x6U == (0x6U & m17.GetFrameType())) ? ECodecType::c2_1600 : ECodecType::c2_3200;
	m_uiM17FrameNumber = 0xFFFFU & m17.GetFrameNumber();
	m_bLastPacket = m17.IsLastPacket();
}

////////////////////////////////////////////////////////////////////////////////////////
// pid conversion

void CPacket::UpdatePids(const uint32_t pid)
{
	// called while pushing this packet in a stream queue
	// so now packet sequence number is known and undefined pids can be updated
	// this is needed as dtsar & dmr pids are different and cannot be
	// derived from each other

	// dstar pid needs update ?
	if (  m_uiDstarPacketId ==  0xFF )
	{
		m_uiDstarPacketId = (pid % 21);
	}
	// dmr pids need update ?
	if ( m_uiDmrPacketId == 0xFF )
	{
		m_uiDmrPacketId = ((pid / 3) % 6);
		m_uiDmrPacketSubid = ((pid % 3) + 1);
	}
	// ysf pids need update ?
	if ( m_uiYsfPacketId == 0xFF )
	{
		m_uiYsfPacketId = ((pid / 5) % 8);
		m_uiYsfPacketSubId = pid % 5;
		m_uiYsfPacketFrameId = ((pid / 5) & 0x7FU) << 1;
	}
	if ( m_uiNXDNPacketId == 0xFF )
	{
		m_uiNXDNPacketId = pid % 4;
	}
	// m17 needs update?
	if (m_uiM17FrameNumber == 0xFFFFFFFFU)
	{
		// frames are every 20 milliseconds, so the m17 data will come every 40 milliseconds
		m_uiM17FrameNumber = (pid / 2) % 0x8000U;
	}
}
