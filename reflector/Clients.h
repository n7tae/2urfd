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

#include "Client.h"


////////////////////////////////////////////////////////////////////////////////////////
// define


////////////////////////////////////////////////////////////////////////////////////////
// class

class CClients
{
public:
	// constructors
	CClients();

	// destructors
	virtual ~CClients();

	// locks
	void Lock(void)                     { m_Mutex.lock(); }
	void Unlock(void)                   { m_Mutex.unlock(); }

	// manage Clients
	int     GetSize(void) const         { return (int)m_Clients.size(); }
	void    AddClient(std::shared_ptr<CClient>);
	void    RemoveClient(std::shared_ptr<CClient>);
	bool    IsClient(std::shared_ptr<CClient>) const;

	// pass-through
	std::list<std::shared_ptr<CClient>>::iterator begin()              { return m_Clients.begin(); }
	std::list<std::shared_ptr<CClient>>::iterator end()                { return m_Clients.end(); }
	std::list<std::shared_ptr<CClient>>::const_iterator cbegin() const { return m_Clients.cbegin(); }
	std::list<std::shared_ptr<CClient>>::const_iterator cend()   const { return m_Clients.cend(); }

	// find clients
	std::shared_ptr<CClient> FindClient(const CIp &);
	std::shared_ptr<CClient> FindClient(const CIp &, const EProtocol);
	std::shared_ptr<CClient> FindClient(const CIp &, const EProtocol, const char);
	std::shared_ptr<CClient> FindClient(const CCallsign &, const CIp &, const EProtocol);
	std::shared_ptr<CClient> FindClient(const CCallsign &, char, const CIp &, const EProtocol);
	std::shared_ptr<CClient> FindClient(const CCallsign &, const EProtocol);

	// iterate on clients
	std::shared_ptr<CClient> FindNextClient(const EProtocol, std::list<std::shared_ptr<CClient>>::iterator &);
	std::shared_ptr<CClient> FindNextClient(const CIp &, const EProtocol, std::list<std::shared_ptr<CClient>>::iterator &);
	std::shared_ptr<CClient> FindNextClient(const CCallsign &, const CIp &, const EProtocol, std::list<std::shared_ptr<CClient>>::iterator &);

protected:
	// data
	std::mutex           m_Mutex;
	std::list<std::shared_ptr<CClient>> m_Clients;
};
