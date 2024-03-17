//  Copyright © 2015 Jean-Luc Deltombe (LX3JL). All rights reserved.
//
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


#include <string.h>
#include <iostream>

#include "Defines.h"
#include "DVHeaderPacket.h"

////////////////////////////////////////////////////////////////////////////////////////
// constructor

CDvHeaderPacket::CDvHeaderPacket()
{
	m_uiFlag1 = 0;
	m_uiFlag2 = 0;
	m_uiFlag3 = 0;
	m_uiCrc = 0;
}

// network
CDvHeaderPacket::CDvHeaderPacket(const CBuffer &buf) : CPacket(buf)
{
	if (buf.size() >= GetNetworkSize())
	{
		auto o = CPacket::GetNetworkSize();
		auto data = buf.data();
		m_uiFlag1 = data[o++];
		m_uiFlag2 = data[o++];
		m_uiFlag3 = data[o++];
		m_csUR.SetCallsign(data+o, CALLSIGN_LEN);
		o += CALLSIGN_LEN;
		m_csRPT1.SetCallsign(data+o, CALLSIGN_LEN);
		o += CALLSIGN_LEN;
		m_csRPT2.SetCallsign(data+o, CALLSIGN_LEN);
		o += CALLSIGN_LEN;
		m_csMY.SetCallsign(data+o, CALLSIGN_LEN);
		o += CALLSIGN_LEN;
		m_uiCrc = data[o] * 0x100u + data[o+1];
	}
	else
	{
		std::cerr << "CBuffer is too small to initialize a CDvHeaderPacket" << std::endl;
	}
}

unsigned int CDvHeaderPacket::GetNetworkSize()
{
	return CPacket::GetNetworkSize() + (4 * CALLSIGN_LEN) + 5;
}

void CDvHeaderPacket::EncodeInterlinkPacket(CBuffer &buf) const
{
	CPacket::EncodeInterlinkPacket("URFH", buf);
	buf.resize(GetNetworkSize());
	auto data = buf.data();
	auto off = CPacket::GetNetworkSize();
	data[off++] = m_uiFlag1;
	data[off++] = m_uiFlag2;
	data[off++] = m_uiFlag3;
	m_csUR.GetCallsign(data+off);	off += CALLSIGN_LEN;
	m_csRPT1.GetCallsign(data+off);	off += CALLSIGN_LEN;
	m_csRPT2.GetCallsign(data+off); off += CALLSIGN_LEN;
	m_csMY.GetCallsign(data+off);	off += CALLSIGN_LEN;
	data[off++] = (m_uiCrc / 0x100u) & 0xffu;
	data[off]   = m_uiCrc & 0xffu;
}

// dstar constructor
CDvHeaderPacket::CDvHeaderPacket(const struct dstar_header *buffer, uint16_t sid, uint8_t pid)
	: CPacket(sid, pid)
{
	m_uiFlag1 = buffer->Flag1;
	m_uiFlag2 = buffer->Flag2;
	m_uiFlag3 = buffer->Flag3;
	m_csUR.SetCallsign(buffer->UR, CALLSIGN_LEN);
	m_csRPT1.SetCallsign(buffer->RPT1, CALLSIGN_LEN);
	m_csRPT2.SetCallsign(buffer->RPT2, CALLSIGN_LEN);
	m_csMY.SetCallsign(buffer->MY, CALLSIGN_LEN);
	m_csMY.SetSuffix(buffer->SUFFIX, CALLSUFFIX_LEN);
	m_uiCrc = buffer->Crc;
}

// dmr constructor
CDvHeaderPacket::CDvHeaderPacket(uint32_t my, const CCallsign &ur, const CCallsign &rpt1, const CCallsign &rpt2, uint16_t sid, uint8_t pid, uint8_t spid)
	: CPacket(sid, pid, spid, false)
{
	m_uiFlag1 = 0;
	m_uiFlag2 = 0;
	m_uiFlag3 = 0;
	m_uiCrc = 0;
	m_csUR = ur;
	m_csRPT1 = rpt1;
	m_csRPT2 = rpt2;
	m_csMY = CCallsign("", my);
}

// YSF constructor
CDvHeaderPacket::CDvHeaderPacket(const CCallsign &my, const CCallsign &ur, const CCallsign &rpt1, const CCallsign &rpt2, uint16_t sid, uint8_t pid)
	: CPacket(sid, pid, 0, 0)
{
	m_uiFlag1 = 0;
	m_uiFlag2 = 0;
	m_uiFlag3 = 0;
	m_uiCrc = 0;
	m_csUR = ur;
	m_csRPT1 = rpt1;
	m_csRPT2 = rpt2;
	m_csMY = my;
}

// P25
CDvHeaderPacket::CDvHeaderPacket(const CCallsign &my, const CCallsign &ur, const CCallsign &rpt1, const CCallsign &rpt2, uint16_t sid)
	: CPacket(sid, false)
{
	m_uiFlag1 = 0;
	m_uiFlag2 = 0;
	m_uiFlag3 = 0;
	m_uiCrc = 0;
	m_csUR = ur;
	m_csRPT1 = rpt1;
	m_csRPT2 = rpt2;
	m_csMY = my;
}

// M17
CDvHeaderPacket::CDvHeaderPacket(const CM17Packet &m17) : CPacket(m17)
{
	m_uiFlag1 = m_uiFlag2 = m_uiFlag3 = 0;
	m_uiCrc = 0;
	m_csUR = CCallsign("CQCQCQ");
	m_csMY = m17.GetSourceCallsign();
	m_csRPT1 = m_csRPT2 = m17.GetDestCallsign();
	m_csRPT1.SetModule('G');
}

std::unique_ptr<CPacket> CDvHeaderPacket::Copy(void)
{
	return std::make_unique<CDvHeaderPacket>(*this);
}

////////////////////////////////////////////////////////////////////////////////////////
// conversion

void CDvHeaderPacket::ConvertToDstarStruct(struct dstar_header *buffer) const
{
	memset(buffer, 0, sizeof(struct dstar_header));
	buffer->Flag1 = m_uiFlag1;
	buffer->Flag2 = m_uiFlag2;
	buffer->Flag3 = m_uiFlag3;
	m_csUR.GetCallsign(buffer->UR);
	m_csRPT1.GetCallsign(buffer->RPT1);
	m_csRPT2.GetCallsign(buffer->RPT2);
	m_csMY.GetCallsign(buffer->MY);
	m_csMY.GetSuffix(buffer->SUFFIX);
	buffer->Crc = m_uiCrc;
}


////////////////////////////////////////////////////////////////////////////////////////
// get valid

bool CDvHeaderPacket::IsValid(void) const
{
	bool valid = CPacket::IsValid();

	valid &= m_csRPT1.IsValid();
	valid &= m_csRPT2.IsValid();
	valid &= m_csMY.IsValid();

	return valid;
}
