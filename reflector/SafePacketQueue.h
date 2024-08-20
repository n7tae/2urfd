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

#include <queue>
#include <mutex>
#include <condition_variable>

/************************************************************
 *                 THIS IS IMPORTANT
 * This template is primarily designed for std::unique_ptr!
 * If you are going to use it for std::shared_ptr, then
 * please consider that when you Push(), what you pushed
 * from will be nullptr after the Push()!
\************************************************************/

template <class T>
class CSafePacketQueue
{
public:
 	CSafePacketQueue(void) : q() , m() , c() {}

	~CSafePacketQueue(void) {}

	void Push(T t)
	{
		std::lock_guard<std::mutex> lock(m);
		q.push(std::move(t));
		c.notify_one();
	}

	T Pop(void)
	{
		std::lock_guard<std::mutex> lock(m);
		if (q.empty())
			return nullptr;
		else
		{
			T val = std::move(q.front());
			q.pop();
			return val;
		}
	}

	// If the queue is empty, wait until an element is available.
	T PopWait(void)
	{
		std::unique_lock<std::mutex> lock(m);
		while(q.empty())
		{
			// release lock as long as the wait and reacquire it afterwards.
			c.wait(lock);
		}
		T val = std::move(q.front());
		q.pop();
		return val;
	}

	bool IsEmpty(void)
	{
		std::unique_lock<std::mutex> lock(m);
		return q.empty();
	}

private:
	std::queue<T> q;
	mutable std::mutex m;
	std::condition_variable c;
};
