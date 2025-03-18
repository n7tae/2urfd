/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX and John Hays K7VE
 *   Copyright 2016 by Jeremy McDermond (NH6Z)
 *   Copyright 2021 by Thomas Early N7TAE
 *
 *   Adapted by K7VE from G4KLX dv3000d
 */

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

#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cerrno>
#include <thread>

#include "DV3000.h"
#include "Configure.h"
#include "Transcoder.h"

extern CTranscoder g_Transcoder;

CDV3000::CDV3000(Encoding t) : CDVDevice(t) {}

CDV3000::~CDV3000()
{
	waiting_packet.Shutdown();
}

void CDV3000::PushWaitingPacket(unsigned int /* channel */, std::shared_ptr<CTranscoderPacket> packet)
{
	waiting_packet.push(packet);
}

std::shared_ptr<CTranscoderPacket> CDV3000::PopWaitingPacket(unsigned int /* channel */)
{
	return waiting_packet.pop();
}

bool CDV3000::SendAudio(const uint8_t /*channel*/, const int16_t *audio) const
{
	// Create Audio packet based on input int8_ts
	SDV_Packet p;
	p.start_byte = PKT_HEADER;
	p.header.payload_length = htons(1 + sizeof(p.payload.audio3k));
	p.header.packet_type = PKT_SPEECH;
	p.field_id = PKT_SPEECHD;
	p.payload.audio3k.num_samples = 160U;
	for (int i=0; i<160; i++)
		p.payload.audio3k.samples[i] = htons(audio[i]);

	// send audio packet to DV3000
	const DWORD size = packet_size(p);
	DWORD written;
	auto status = FT_Write(ftHandle, &p, size, &written);
	if (FT_OK != status)
	{
		FTDI_Error("Error writing audio packet", status);
		return true;
	}
	else if (size != written)
	{
		std::cerr << "Incomplete Speech Packet write on " << description << std::endl;
		return true;
	}

	return false;
}

bool CDV3000::SendData(const uint8_t /* channel */, const uint8_t *data) const
{
	// Create data packet
	SDV_Packet p;
	p.start_byte = PKT_HEADER;
	p.header.payload_length = htons(1 + sizeof(p.payload.ambe3k));
	p.header.packet_type = PKT_CHANNEL;
	p.field_id = PKT_CHAND;
	p.payload.ambe3k.num_bits = 72U;
	memcpy(p.payload.ambe3k.data, data, 9);

	// send data packet to DV3000
	const DWORD size = packet_size(p);
	DWORD written;
	auto status = FT_Write(ftHandle, &p, size, &written);
	if (FT_OK != status)
	{
		FTDI_Error("Error writing AMBE Packet", status);
		return true;
	}
	else if (size != written)
	{
		std::cerr << "Incomplete AMBE Packet write on " << description << std::endl;
		return true;
	}

	return false;
}

void CDV3000::ProcessPacket(const SDV_Packet &p)
{
	auto packet = PopWaitingPacket(PKT_CHANNEL0);
	if (packet)
	{
		if (PKT_CHANNEL == p.header.packet_type)
		{
			if (11!=ntohs(p.header.payload_length) || PKT_CHAND!=p.field_id || 72!=p.payload.ambe3k.num_bits)
				dump("Improper ambe packet:", &p, packet_size(p));
			buffer_depth--;
			if (Encoding::dstar == type)
				packet->SetDStarData(p.payload.ambe3k.data);
			else
				packet->SetDMRData(p.payload.ambe3k.data);

		}
		else if (PKT_SPEECH == p.header.packet_type)
		{
			if (322!=ntohs(p.header.payload_length) || PKT_SPEECHD!=p.field_id || 160!=p.payload.audio3k.num_samples)
				dump("Improper audio packet:", &p, packet_size(p));
			buffer_depth--;
			packet->SetAudioSamples(p.payload.audio3k.samples, true);
		}
		else
		{
			dump("ReadDevice() ERROR: Read an unexpected device packet:", &p, packet_size(p));
			return;
		}
		if (Encoding::dstar == type)	// is this a DMR or a DStar device?
		{
			g_Transcoder.dstar_mux.lock();
			g_Transcoder.RouteDstPacket(packet);
			g_Transcoder.dstar_mux.unlock();
		}
		else
		{
			g_Transcoder.dmrst_mux.lock();
			g_Transcoder.RouteDmrPacket(packet);
			g_Transcoder.dmrst_mux.unlock();
		}
	}
}
