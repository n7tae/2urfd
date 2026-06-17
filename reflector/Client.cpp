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


#include <string.h>
#include "Client.h"


////////////////////////////////////////////////////////////////////////////////////////
// constructors

CClient::CClient(const CCallsign &callsign, const CIp &ip, char reflectorModule) : CClient()
{
	m_ReflectorModule = reflectorModule;
	m_Callsign = callsign;
	m_Ip = ip;
}

CClient::CClient(const CClient &client)
{
	m_IsListenOnly = client.m_IsListenOnly;
	m_Callsign = client.m_Callsign;
	m_Ip = client.m_Ip;
	m_ReflectorModule = client.m_ReflectorModule;
	m_ModuleMastered = client.m_ModuleMastered;
	m_LastKeepaliveTime = client.m_LastKeepaliveTime;
	m_ConnectTime = client.m_ConnectTime;
	m_LastHeardTime = client.m_LastHeardTime;
}

////////////////////////////////////////////////////////////////////////////////////////
// status

void CClient::Alive(void)
{
	m_LastKeepaliveTime.start();
}


////////////////////////////////////////////////////////////////////////////////////////
// operators

bool CClient::operator ==(const CClient &client) const
{
	return ((client.m_Callsign == m_Callsign) &&
			(client.m_Ip == m_Ip) &&
			(client.m_ReflectorModule == m_ReflectorModule));
}

////////////////////////////////////////////////////////////////////////////////////////
// reporting

void CClient::WriteXml(std::ofstream &xmlFile)
{
	xmlFile <<
	"<NODE>\n" <<
	"\t<Callsign>"      << m_Callsign        << "</Callsign>\n" <<
	"\t<IP>"            << m_Ip.GetAddress() << "</IP>\n" <<
	"\t<LinkedModule>"  << m_ReflectorModule << "</LinkedModule>\n" <<
	"\t<Protocol>"      << GetProtocolName() << "</Protocol>\n" <<
	"\t<ListenOnly>"    << IsListenOnly()    << "</ListenOnly>\n" <<
	"\t<ConnectTime>"   << m_ConnectTime     << "</ConnectTime>\n" <<
	"\t<LastHeardTime>" << m_LastHeardTime   << "</LastHeardTime>\n" <<
	"</NODE>"           << std::endl;
}

void CClient::JsonReport(nlohmann::json &report)
{
	nlohmann::json jclient;
	jclient["Callsign"] = m_Callsign.GetCS();
	jclient["OnModule"] = std::string(1, m_ReflectorModule);
	jclient["Protocol"] = GetProtocolName();
	jclient["ListenOnly"] = IsListenOnly();
	char s[100];
	if (std::strftime(s, sizeof(s), "%FT%TZ", std::gmtime(&m_ConnectTime)))
		jclient["ConnectTime"] = s;
	if (std::strftime(s, sizeof(s), "%FT%TZ", std::gmtime(&m_LastHeardTime)))
		jclient["LastHeardTime"] = s;
	report["Clients"].push_back(jclient);
}
