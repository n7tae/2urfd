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


#include "Timer.h"
#include "Global.h"

////////////////////////////////////////////////////////////////////////////////////////
// constructor

CGateKeeper::CGateKeeper()
{
	keep_running = true;
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor

CGateKeeper::~CGateKeeper()
{
	Close();
}


////////////////////////////////////////////////////////////////////////////////////////
// init & clode

bool CGateKeeper::Init(void)
{

	// load lists from files
	m_WhiteSet.LoadFromFile(g_Configure.GetString(g_Keys.files.white));
	m_BlackSet.LoadFromFile(g_Configure.GetString(g_Keys.files.black));
	m_InterlinkMap.LoadFromFile(g_Configure.GetString(g_Keys.files.interlink));

	// reset run flag
	keep_running = true;

	// start  thread;
	m_Future = std::async(std::launch::async, &CGateKeeper::Thread, this);

	return true;
}

void CGateKeeper::Close(void)
{
	// kill threads
	keep_running = false;
	if ( m_Future.valid() )
	{
		m_Future.get();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// authorisations

bool CGateKeeper::MayLink(const CCallsign &callsign, const CIp &ip, EProtocol protocol, char *modules) const
{
	bool ok;
	const std::string base(callsign.GetBase());

	// URF interlinks
	if ( protocol == EProtocol::urf )
		ok = IsPeerListedOk(base, ip, modules);
	else
		ok = IsNodeListedOk(base);

	// report
	if ( ! ok )
	{
		std::cout << "Gatekeeper blocking linking of " << callsign << " @ " << ip << " using protocol " << g_Reflector.GetProtocolName(protocol) << std::endl;
	}

	// done
	return ok;
}

bool CGateKeeper::MayTransmit(const CCallsign &callsign, const CIp &ip, const EProtocol protocol, char module) const
{
	bool ok;

	const std::string base(callsign.GetBase());

	if ( protocol == EProtocol::urf )
		ok = IsPeerListedOk(base, module);
	else
		ok = IsNodeListedOk(base);

	// report
	if ( !ok )
	{
		std::cout << "Gatekeeper blocking transmitting of " << callsign << " @ " << ip << " using protocol " << g_Reflector.GetProtocolName(protocol) << std::endl;
	}

	// done
	return ok;
}

////////////////////////////////////////////////////////////////////////////////////////
// thread

void CGateKeeper::Thread()
{
	while (keep_running)
	{
		// have lists files changed ?
		if ( m_WhiteSet.NeedReload() )
		{
			m_WhiteSet.ReloadFromFile();
		}
		if ( m_BlackSet.NeedReload() )
		{
			m_BlackSet.ReloadFromFile();
		}
		if ( m_InterlinkMap.NeedReload() )
		{
			m_InterlinkMap.ReloadFromFile();
		}
		// Wait 30 seconds
		for (int i=0; i<10 && keep_running; i++)
			std::this_thread::sleep_for(std::chrono::seconds(3));
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// operation helpers

bool CGateKeeper::IsNodeListedOk(const std::string &callsign) const
{
	bool ok = true;

	// next, check callsign
	if ( ok )
	{
		// first check if callsign is in white list
		// note if white list is empty, everybody is authorized
		m_WhiteSet.Lock();
		if ( ! m_WhiteSet.empty() )
		{
			ok = m_WhiteSet.IsMatched(callsign);
		}
		m_WhiteSet.Unlock();

		// then check if not blacklisted
		if (ok)
		{
			m_BlackSet.Lock();
			ok = ! m_BlackSet.IsMatched(callsign);
			m_BlackSet.Unlock();
		}
	}

	// done
	return ok;

}

bool CGateKeeper::IsPeerListedOk(const std::string &callsign, char module) const
{
	bool ok = true;

	// first check IP

	// next, check callsign
	if ( ok )
	{
		// look for an exact match in the list
		m_InterlinkMap.Lock();
		if ( !m_InterlinkMap.empty() )
		{
			ok = m_InterlinkMap.IsCallsignListed(callsign, module);
		}
		m_InterlinkMap.Unlock();
	}

	// done
	return ok;
}

bool CGateKeeper::IsPeerListedOk(const std::string &callsign, const CIp &ip, char *modules) const
{
	bool ok = true;

	// first check IP

	// next, check callsign
	if ( ok )
	{
		// look for an exact match in the list
		m_InterlinkMap.Lock();
		if ( ! m_InterlinkMap.empty() )
		{
			ok = m_InterlinkMap.IsCallsignListed(callsign, ip, modules);
		}
		m_InterlinkMap.Unlock();
	}

	// done
	return ok;
}
