//  Copyright © 2016 Jean-Luc Deltombe (LX3JL). All rights reserved.

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

#include "Version.h"
#include "Timer.h"
#include "IP.h"
#include "Callsign.h"
#include "Client.h"

class CURFPeer
{
public:
	// constructors
	CURFPeer();
	CURFPeer(const CCallsign &, const CIp &, const std::string &, const CVersion &);
	CURFPeer(const CURFPeer &) = delete;

	// destructor
	virtual ~CURFPeer();

	// operators
	bool operator ==(const CURFPeer &) const;

	// get
	const CCallsign &GetCallsign(void) const            { return m_Callsign; }
	const CIp &GetIp(void) const                        { return m_Ip; }
	const std::string &GetReflectorModules(void)        { return m_ReflectorModules; }
	std::time_t GetConnectTime(void) const              { return m_ConnectTime; }

	// set

	// identity
	EProtocol GetProtocol(void) const           { return EProtocol::urf; };
	EProtoRev GetProtocolRevision(const CVersion *) { return EProtoRev::original; };
	const char *GetProtocolName(void) const     { return "URF"; };

	// status
	bool IsAMaster(void) const;
	void Alive(void);
	bool IsAlive(void) const;
	void Heard(void)                            { m_LastHeardTime = std::time(nullptr); }

	// clients access
	int     GetNbClients(void) const                    { return (int)m_Clients.size(); }
	void    ClearClients(void)                          { m_Clients.clear(); }

	// pass-through
	std::list<std::shared_ptr<CClient>>::iterator begin()              { return m_Clients.begin(); }
	std::list<std::shared_ptr<CClient>>::iterator end()                { return m_Clients.end(); }
	std::list<std::shared_ptr<CClient>>::const_iterator cbegin() const { return m_Clients.cbegin(); }
	std::list<std::shared_ptr<CClient>>::const_iterator cend() const   { return m_Clients.cend(); }

	// reporting
	void WriteXml(std::ofstream &);
	void JsonReport(nlohmann::json &report);

protected:
	// data
	CCallsign             m_Callsign;
	CIp                   m_Ip;
	std::string           m_ReflectorModules;
	CVersion              m_Version;
	std::list<std::shared_ptr<CClient>> m_Clients;

	// status
	CTimer                m_LastKeepaliveTime;
	std::time_t           m_ConnectTime;
	std::time_t           m_LastHeardTime;
};
