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

#include <future>
#include <atomic>

#include "Defines.h"
#include "Timer.h"
#include "Protocol.h"
#include "DVHeaderPacket.h"
#include "DVFramePacket.h"

enum class EDSDAction { logon, change, logoff };

class CDSDAction
{
public:
	CDSDAction() = delete;
	CDSDAction(EDSDAction type, std::shared_ptr<CClient> oldclient, std::shared_ptr<CClient> newclient) : m_Type(type), m_OldClient(oldclient), m_NewClient(newclient), m_Completed(false) { Update(); }
	std::shared_ptr<CClient> GetOldClient() const { return m_OldClient; }
	std::shared_ptr<CClient> GetNewClient() const { return m_NewClient; }
	double GetTime() const { return m_LastHeardTime.time(); }
	EDSDAction GetType() const { return m_Type; }
	void Update() { m_LastHeardTime.start(); }
	void Completed() { m_Completed = true; }
	bool ActionIsCompleted() const { return m_Completed; }

	std::future<void> m_future;

private:
	const EDSDAction m_Type;
	CTimer m_LastHeardTime;
	std::shared_ptr<CClient> m_OldClient, m_NewClient;
	std::atomic_bool m_Completed;
};
