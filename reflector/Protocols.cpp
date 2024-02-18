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


#include "DExtraProtocol.h"
#include "DCSProtocol.h"
#include "URFProtocol.h"
#include "DMRMMDVMProtocol.h"
#include "YSFProtocol.h"
#include "M17Protocol.h"
#include "P25Protocol.h"
#include "Protocols.h"
#include "Global.h"

////////////////////////////////////////////////////////////////////////////////////////
// destructor

CProtocols::~CProtocols()
{
	Close();
}

////////////////////////////////////////////////////////////////////////////////////////
// initialization

bool CProtocols::Init(void)
{
	m_Mutex.lock();
	{
		m_Protocols.emplace_back(std::make_unique<CDcsProtocol>("DCS"));
		if (! m_Protocols.back()->Initialize("DCS", EProtocol::dcs, uint16_t(g_Configure.GetUnsigned(g_Keys.dcs.port)), DSTAR_IPV4, DSTAR_IPV6))
			return false;

		m_Protocols.emplace_back(std::make_unique<CDextraProtocol>("DExtra"));
		if (! m_Protocols.back()->Initialize("XRF", EProtocol::dextra, uint16_t(g_Configure.GetUnsigned(g_Keys.dextra.port)), DSTAR_IPV4, DSTAR_IPV6))
			return false;

		m_Protocols.emplace_back(std::make_unique<CDmrmmdvmProtocol>("MMDVM"));
		if (! m_Protocols.back()->Initialize(nullptr, EProtocol::mmdvm, uint16_t(g_Configure.GetUnsigned(g_Keys.mmdvm.port)), DMR_IPV4, DMR_IPV6))
			return false;

		m_Protocols.emplace_back(std::make_unique<CM17Protocol>("M17"));
		if (! m_Protocols.back()->Initialize("URF", EProtocol::m17, uint16_t(g_Configure.GetUnsigned(g_Keys.m17.port)), M17_IPV4, M17_IPV6))
			return false;

		m_Protocols.emplace_back(std::make_unique<CP25Protocol>("P25"));
		if (! m_Protocols.back()->Initialize("P25", EProtocol::p25, uint16_t(g_Configure.GetUnsigned(g_Keys.p25.port)), P25_IPV4, P25_IPV6))
			return false;

		m_Protocols.emplace_back(std::make_unique<CURFProtocol>("URF"));
		if (! m_Protocols.back()->Initialize("URF", EProtocol::urf, uint16_t(g_Configure.GetUnsigned(g_Keys.urf.port)), URF_IPV4, URF_IPV6))
			return false;

		m_Protocols.emplace_back(std::make_unique<CYsfProtocol>("YSF"));
		if (! m_Protocols.back()->Initialize("YSF", EProtocol::ysf, uint16_t(g_Configure.GetUnsigned(g_Keys.ysf.port)), YSF_IPV4, YSF_IPV6))
			return false;
	}
	m_Mutex.unlock();

	// done
	return true;
}

void CProtocols::Close(void)
{
	m_Mutex.lock();
	while (m_Protocols.size())
	{
		m_Protocols.front()->Close();
		m_Protocols.pop_front();
	}
	m_Mutex.unlock();
}
