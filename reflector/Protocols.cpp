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

#include "DStarDirectProtocol.h"
#include "DCSProtocol.h"
#include "DExtraProtocol.h"
#include "DPlusProtocol.h"
#include "DMRMMDVMProtocol.h"
#include "M17Protocol.h"
#include "P25Protocol.h"
#include "URFProtocol.h"
#include "YSFProtocol.h"
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
		if (g_Configure.GetUnsigned(g_Keys.dcs.port))
		{
			Get(EProtocol::dcs) = std::make_unique<CDcsProtocol>("DCS");
			if (! Get(EProtocol::dcs)->Initialize("DCS", EProtocol::dcs, uint16_t(g_Configure.GetUnsigned(g_Keys.dcs.port)), DSTAR_IPV4, DSTAR_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.dextra.port))
		{
			Get(EProtocol::dextra) = std::make_unique<CDextraProtocol>("DExtra");
			if (! Get(EProtocol::dextra)->Initialize("XRF", EProtocol::dextra, uint16_t(g_Configure.GetUnsigned(g_Keys.dextra.port)), DSTAR_IPV4, DSTAR_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.dplus.port))
		{
			Get(EProtocol::dplus) = std::make_unique<CDplusProtocol>("DPlus");
			if (! Get(EProtocol::dplus)->Initialize("REF", EProtocol::dplus, uint16_t(g_Configure.GetUnsigned(g_Keys.dplus.port)), DSTAR_IPV4, DSTAR_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.dsd.port))
		{
			Get(EProtocol::dsd) = std::make_unique<CDStarDirectProtocol>("DStarDirect");
			if (! Get(EProtocol::dsd)->Initialize("DSD", EProtocol::dsd, uint16_t(g_Configure.GetUnsigned(g_Keys.dsd.port)), DSTAR_IPV4, DSTAR_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.mmdvm.port))
		{
			Get(EProtocol::mmdvm) = std::make_unique<CDmrmmdvmProtocol>("MMDVM");
			if (! Get(EProtocol::mmdvm)->Initialize(nullptr, EProtocol::mmdvm, uint16_t(g_Configure.GetUnsigned(g_Keys.mmdvm.port)), DMR_IPV4, DMR_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.m17.port))
		{
			Get(EProtocol::m17) = std::make_unique<CM17Protocol>("M17");
			if (! Get(EProtocol::m17)->Initialize("URF", EProtocol::m17, uint16_t(g_Configure.GetUnsigned(g_Keys.m17.port)), M17_IPV4, M17_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.p25.port))
		{
			Get(EProtocol::p25) = std::make_unique<CP25Protocol>("P25");
			if (! Get(EProtocol::p25)->Initialize("P25", EProtocol::p25, uint16_t(g_Configure.GetUnsigned(g_Keys.p25.port)), P25_IPV4, P25_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.urf.port))
		{
			Get(EProtocol::urf) = std::make_unique<CURFProtocol>("URF");
			if (! Get(EProtocol::urf)->Initialize("URF", EProtocol::urf, uint16_t(g_Configure.GetUnsigned(g_Keys.urf.port)), URF_IPV4, URF_IPV6))
				return false;
		}

		if (g_Configure.GetUnsigned(g_Keys.ysf.port))
		{
			Get(EProtocol::ysf) = std::make_unique<CYsfProtocol>("YSF");
			if (! Get(EProtocol::ysf)->Initialize("YSF", EProtocol::ysf, uint16_t(g_Configure.GetUnsigned(g_Keys.ysf.port)), YSF_IPV4, YSF_IPV6))
				return false;
		}
	}
	m_Mutex.unlock();

	// done
	return true;
}

void CProtocols::Close(void)
{
	m_Mutex.lock();
	for (unsigned int i=0; i<toUType(EProtocol::SIZE); i++)
	{
		m_Protocols[i]->Close();
		m_Protocols[i].reset();
	}
	m_Mutex.unlock();
}
