//  Copyright © 2015 Jean-Luc Deltombe (LX3JL). All rights reserved.

// urfd -- The universal reflector
// Copyright © 2023 Thomas A. Early N7TAE
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

#include "Lookup.h"

class CLookupDmr : public CLookup
{
public:
	~CLookupDmr() {}
	uint32_t FindDmrid(const UCallsign &ucs) const;
	const UCallsign *FindCallsign(uint32_t dmrid) const;

protected:
	void ClearContents();
	void LoadParameters();
	void UpdateContent(std::stringstream &ss, Eaction action);

private:
	std::unordered_map<uint32_t, UCallsign> m_CallsignMap;
	std::unordered_map<UCallsign, uint32_t, CCallsignHash, CCallsignEqual> m_DmridMap;
};
