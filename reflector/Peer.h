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

class CPeer
{
public:
	// constructors
	CPeer() = delete;
	CPeer(const CCallsign &, EProtocol, const CIp &, const char *, const CVersion &);
	CPeer(const CPeer &) = delete;

	// destructor
	virtual ~CPeer();

	// operators
	bool operator ==(const CPeer &) const;

	// get
	const CCallsign &GetCallsign(void) const            { return m_Callsign; }
	const CIp &GetIp(void) const                        { return m_Ip; }
	char *GetReflectorModules(void)                     { return m_ReflectorModules; }
	std::time_t GetConnectTime(void) const              { return m_ConnectTime; }

	// set

	// identity
	EProtocol GetProtocol(void) const                   { return m_Protocol; }
	virtual EProtoRev GetProtocolRevision(void) const   { return EProtoRev::original; }
	const std::string &GetProtocolName(void) const;

	// status
	virtual bool IsAMaster(void) const;
	virtual void Alive(void);
	virtual bool IsAlive(void) const                    { return false; }
	virtual void Heard(void)                            { m_LastHeardTime = std::time(nullptr); }

	// clients access
	int     GetNbClients(void) const                    { return (int)m_Clients.size(); }
	void    ClearClients(void)                          { m_Clients.clear(); }

	// pass-through
	std::list<std::shared_ptr<CClient>>::iterator begin()              { return m_Clients.begin(); }
	std::list<std::shared_ptr<CClient>>::iterator end()                { return m_Clients.end(); }
	std::list<std::shared_ptr<CClient>>::const_iterator cbegin() const { return m_Clients.cbegin(); }
	std::list<std::shared_ptr<CClient>>::const_iterator cend() const   { return m_Clients.cend(); }

	// reporting
	virtual void WriteXml(std::ofstream &);
	void JsonReport(nlohmann::json &report);

protected:
	// data
	const CCallsign       m_Callsign;
	const EProtocol       m_Protocol;
	const CIp             m_Ip;
	char                  m_ReflectorModules[27];
	CVersion              m_Version;
	std::list<std::shared_ptr<CClient>> m_Clients;

	// status
	CTimer                m_LastKeepaliveTime;
	std::time_t           m_ConnectTime;
	std::time_t           m_LastHeardTime;
};
