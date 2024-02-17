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

#include "Protocol.h"

class CProtocols
{
public:
	// destructors
	~CProtocols();

	// initialization
	bool Init(void);
	void Close(void);
	void Lock(void)   { m_Mutex.lock(); }
	void Unlock(void) { m_Mutex.unlock(); }

	// pass-through
	std::list<std::unique_ptr<CProtocol>>::iterator begin() { return m_Protocols.begin(); }
	std::list<std::unique_ptr<CProtocol>>::iterator end()   { return m_Protocols.end(); }

protected:
	// data
	std::mutex m_Mutex;
	std::list<std::unique_ptr<CProtocol>> m_Protocols;
};
