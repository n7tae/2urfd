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

#include <nlohmann/json.hpp>

#include "Defines.h"
#include "Timer.h"
#include "IP.h"
#include "Callsign.h"
#include "Buffer.h"
#include "Packet.h"

enum class EProtoRev { original, revised, ambe };

class CClient
{
public:
	// constructors
	CClient() = delete;
	CClient(const CClient &) = delete;
	CClient(const CCallsign &, EProtocol, const CIp &, char = ' ');

	// destructor
	virtual ~CClient() {};

	// operators
	bool operator ==(const CClient &) const;

	// get
	const CCallsign &GetCallsign(void) const            { return m_Callsign; }
	const CIp &GetIp(void) const                        { return m_Ip; }
	bool HasModule(void) const                          { return m_Callsign.HasModule(); }
	char GetClientModule(void) const                    { return m_Callsign.GetModule(); }
	bool HasReflectorModule(void) const                 { return m_ReflectorModule != ' '; }
	char GetReflectorModule(void) const                 { return m_ReflectorModule; }
	std::time_t GetConnectTime(void) const              { return m_ConnectTime; }
	std::time_t GetLastHeardTime(void) const            { return m_LastHeardTime; }

	// set
	void SetModule(char c)                               { m_Callsign.SetModule(c); }
	void SetReflectorModule(char c)                      { m_ReflectorModule = c; }

	// identity
	EProtocol GetProtocol(void) const                    { return m_Protocol; }
	const std::string &GetProtocolName(void) const;
	virtual EProtoRev GetProtocolRevision(void) const    { return EProtoRev::ambe; }
	virtual bool IsPeer(void) const                      { return false; }
	virtual bool IsDextraDongle(void) const              { return false; }
	virtual void SetDextraDongle(void)                   { }

	// status
	virtual void Alive(void);
	virtual bool IsAlive(void) const                    { return false; }
	virtual bool IsAMaster(void) const                  { return (m_ModuleMastered != ' '); }
	virtual void SetMasterOfModule(char c)              { m_ModuleMastered = c; }
	virtual void NotAMaster(void)                       { m_ModuleMastered = ' '; }
	virtual void Heard(void)                            { m_LastHeardTime = std::time(nullptr); }

	// reporting
	virtual void WriteXml(std::ofstream &);
	void JsonReport(nlohmann::json &report);

protected:
	// data

	// identity
	CCallsign       m_Callsign;
	const CIp       m_Ip;
	char            m_ReflectorModule;
	const EProtocol m_Protocol;

	// status
	char        m_ModuleMastered;
	CTimer      m_LastKeepaliveTime;
	std::time_t m_ConnectTime;
	std::time_t m_LastHeardTime;
};
