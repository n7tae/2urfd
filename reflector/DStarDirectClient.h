// urfd -- The universal reflector
// Copyright © 2024 Thomas A. Early N7TAE
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

#include "Defines.h"
#include "Client.h"

class CDStarDirectClient : public CClient
{
public:
	// constructors
	CDStarDirectClient(const CCallsign &callsign, const CCallsign &repeater, const CIp &ip, char mod);

	// destructor
	virtual ~CDStarDirectClient() {};

	// identity
	EProtocol GetProtocol(void) const           { return EProtocol::dsd; }
	const char *GetProtocolName(void) const     { return "DStarDirect"; }
	const CCallsign &GetRepeater(void) const    { return m_Repeater; }

	// status
	bool IsAlive(void) const;
protected:
	CCallsign m_Repeater;
};
