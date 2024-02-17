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

#include "Callsign.h"
#include "Buffer.h"

class CUser
{
public:
	// constructor
	CUser();
	CUser(const CCallsign &, const CCallsign &, const CCallsign &, const CCallsign &);
	CUser(const CUser &);

	// destructor
	~CUser() {}

	// get
	const std::string GetCallsign(void) const { return m_My.GetCS(); }
	const std::string GetViaNode(void)  const { return m_Rpt1.GetCS(); }
	char GetOnModule(void)              const { return m_Rpt2.GetCSModule(); }
	const std::string GetViaPeer(void)  const { return m_Xlx.GetCS(); }
	std::time_t GetLastHeardTime(void)  const { return m_LastHeardTime; }

	// operation
	void HeardNow(void)     { m_LastHeardTime = time(nullptr); }

	// operators
	bool operator ==(const CUser &) const;
	bool operator <(const CUser &) const;

	// reporting
	void WriteXml(std::ofstream &);
	void JsonReport(nlohmann::json &report);

protected:
	// data
	CCallsign   m_My;
	CCallsign   m_Rpt1;
	CCallsign   m_Rpt2;
	CCallsign   m_Xlx;
	time_t      m_LastHeardTime;
};
