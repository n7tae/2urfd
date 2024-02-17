//  Copyright © 2019 Jean-Luc Deltombe (LX3JL). All rights reserved.

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

#include "WiresXPacket.h"

class CWiresxPacketQueue
{
public:
	// constructor
	CWiresxPacketQueue() {}

	// destructor
	~CWiresxPacketQueue() {}

	// lock
	void Lock()                 { m_Mutex.lock(); }
	void Unlock()               { m_Mutex.unlock(); }

	// pass thru
	CWiresxPacket front()           { return queue.front(); }
	void pop()                      { queue.pop(); }
	void push(CWiresxPacket packet) { queue.push(packet); }
	bool empty() const              { return queue.empty(); }

protected:
	// status
	std::mutex  m_Mutex;
	std::queue<CWiresxPacket> queue;
};
