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

#include <nlohmann/json.hpp>
#include <string.h>
#include "Reflector.h"
#include "URFPeer.h"
#include "URFClient.h"


////////////////////////////////////////////////////////////////////////////////////////
// constructor


CURFPeer::CURFPeer()
{
	m_ConnectTime = std::time(nullptr);
	m_LastHeardTime = std::time(nullptr);
}

CURFPeer::CURFPeer(const CCallsign &callsign, const CIp &ip, const std::string &modules, const CVersion &version)
{
	m_Callsign = callsign;
	m_Ip = ip;
	m_ReflectorModules.assign(modules);
	m_Version = version;
	m_LastKeepaliveTime.start();
	m_ConnectTime = std::time(nullptr);
	m_LastHeardTime = std::time(nullptr);
	// get protocol revision
	EProtoRev protrev = GetProtocolRevision(&version);
	//std::cout << "Adding URF peer with protocol revision " << protrev << std::endl;

	// and construct all xlx clients
	for ( const auto c : modules )
	{
		// create and append to vector
		m_Clients.push_back(std::make_shared<CURFClient>(callsign, ip, c, protrev));
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// destructors

CURFPeer::~CURFPeer()
{
	m_Clients.clear();
}

////////////////////////////////////////////////////////////////////////////////////////
// operators

bool CURFPeer::operator ==(const CURFPeer &peer) const
{
	if (peer.m_Callsign != m_Callsign)
		return false;
	if (peer.m_Ip != m_Ip)
		return false;
	if (! (peer.m_Version == m_Version))
		return false;
	auto it1 = cbegin();
	auto it2 = peer.cbegin();
	while (true)
	{
		if (it1==cend() && it2==peer.cend())
			break;
		if (it1==cend() || it2==peer.cend())
			return false;
		if (*it1 != *it2)
			return false;
		it1++;
		it2++;
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////////////
// status

bool CURFPeer::IsAMaster(void) const
{
	for ( auto it=cbegin(); it!=cend(); it++ )
	{
		if ((*it)->IsAMaster())
			return true;
	}
	return false;
}

bool CURFPeer::IsAlive(void) const
{
	return (m_LastKeepaliveTime.time() < URF_KEEPALIVE_TIMEOUT);
}

void CURFPeer::Alive(void)
{
	m_LastKeepaliveTime.start();
	for ( auto it=begin(); it!=end(); it++ )
	{
		(*it)->Alive();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// reporting

void CURFPeer::WriteXml(std::ofstream &xmlFile)
{
	xmlFile << "<PEER>" << std::endl;
	xmlFile << "\t<Callsign>" << m_Callsign << "</Callsign>" << std::endl;
	xmlFile << "\t<IP>" << m_Ip.GetAddress() << "</IP>" << std::endl;
	xmlFile << "\t<LinkedModule>" << m_ReflectorModules << "</LinkedModule>" << std::endl;
	xmlFile << "\t<Protocol>" << GetProtocolName() << "</Protocol>" << std::endl;
	xmlFile << "\t<ConnectTime>" << m_ConnectTime << "</ConnectTime>" << std::endl;
	xmlFile << "\t<LastHeardTime>" << m_LastHeardTime << "</LastHeardTime>" << std::endl;
	xmlFile << "</PEER>" << std::endl;
}

void CURFPeer::JsonReport(nlohmann::json &report)
{
	nlohmann::json jpeer;
	jpeer["Callsign"] = m_Callsign.GetCS();
	jpeer["Modules"] = m_ReflectorModules;
	jpeer["Protocol"] = GetProtocolName();
	char s[100];
	if (std::strftime(s, sizeof(s), "%FT%TZ", std::gmtime(&m_ConnectTime)))
		jpeer["ConnectTime"] = s;
	if (std::strftime(s, sizeof(s), "%FT%TZ", std::gmtime(&m_LastHeardTime)))
		jpeer["LastHeardTime"] = s;
	report["Peers"].push_back(jpeer);
}
