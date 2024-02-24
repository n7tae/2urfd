#pragma once

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

#include <stdlib.h>
#include <sys/un.h>

#include "TCPacketDef.h"

class CUnixDgramReader
{
public:
	CUnixDgramReader();
	~CUnixDgramReader();
	bool Open(const char *path);
	bool Read(STCPacket *pack) const;
	bool Receive(STCPacket *pack, unsigned timeout) const;
	void Close();
	int GetFD() const;
private:
	int fd;
};

class CUnixDgramWriter
{
public:
	CUnixDgramWriter();
	~CUnixDgramWriter();
	void SetUp(const char *path);
	bool Send(const STCPacket *pack) const;
private:
	ssize_t Write(const void *buf, ssize_t size) const;

	struct sockaddr_un addr;
};
