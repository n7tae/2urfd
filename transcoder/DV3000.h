#pragma once

// tcd - a hybid transcoder using DVSI hardware and Codec2 software
// Copyright © 2022 Thomas A. Early N7TAE

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#include "DVSIDevice.h"

class CDV3000 : public CDVDevice
{
public:
	CDV3000(Encoding t);
	virtual ~CDV3000();

protected:
	void PushWaitingPacket(unsigned int channel, std::shared_ptr<CTranscoderPacket> packet);
	std::shared_ptr<CTranscoderPacket> PopWaitingPacket(unsigned int channel);
	void ProcessPacket(const SDV_Packet &p);
	bool SendAudio(const uint8_t channel, const int16_t *audio) const;
	bool SendData(const uint8_t channel, const uint8_t *data) const;

private:
	CPacketQueue waiting_packet;	// the packet currently being processed in each vocoder
};
