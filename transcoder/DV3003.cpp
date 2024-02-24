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

#include "DV3003.h"
#include "Configure.h"
#include "Controller.h"

extern CController g_Cont;

CDV3003::CDV3003(Encoding t) : CDVDevice(t) {}

CDV3003::~CDV3003()
{
	for (int i=0; i<3; i++)
		waiting_packet[i].Shutdown();
}

void CDV3003::PushWaitingPacket(unsigned int channel, std::shared_ptr<CTranscoderPacket> packet)
{
	waiting_packet[channel].push(packet);
}

std::shared_ptr<CTranscoderPacket> CDV3003::PopWaitingPacket(unsigned int channel)
{
	return waiting_packet[channel].pop();
}

bool CDV3003::SendAudio(const uint8_t channel, const int16_t *audio) const
{
	// Create Audio packet based on input int8_ts
	SDV_Packet p;
	p.start_byte = PKT_HEADER;
	p.header.payload_length = htons(1 + sizeof(p.payload.audio));
	p.header.packet_type = PKT_SPEECH;
	p.field_id = channel + PKT_CHANNEL0;
	p.payload.audio.speechd = PKT_SPEECHD;
	p.payload.audio.num_samples = 160U;
	for (int i=0; i<160; i++)
		p.payload.audio.samples[i] = htons(audio[i]);

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

bool CDV3003::SendData(const uint8_t channel, const uint8_t *data) const
{
	// Create data packet
	SDV_Packet p;
	p.start_byte = PKT_HEADER;
	p.header.payload_length = htons(1 + sizeof(p.payload.ambe));
	p.header.packet_type = PKT_CHANNEL;
	p.field_id = channel + PKT_CHANNEL0;
	p.payload.ambe.chand = PKT_CHAND;
	p.payload.ambe.num_bits = 72U;
	memcpy(p.payload.ambe.data, data, 9);

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

void CDV3003::ProcessPacket(const SDV_Packet &p)
{
	unsigned int channel = p.field_id - PKT_CHANNEL0;
	auto packet = PopWaitingPacket(channel);
	if (packet)
	{
		if (PKT_CHANNEL == p.header.packet_type)
		{
			if (12!=ntohs(p.header.payload_length) || PKT_CHAND!=p.payload.ambe.chand || 72!=p.payload.ambe.num_bits)
				dump("Improper ambe packet:", &p, packet_size(p));
			buffer_depth--;
			if (Encoding::dstar == type)
				packet->SetDStarData(p.payload.ambe.data);
			else
				packet->SetDMRData(p.payload.ambe.data);

		}
		else if (PKT_SPEECH == p.header.packet_type)
		{
			if (323!=ntohs(p.header.payload_length) || PKT_SPEECHD!=p.payload.audio.speechd || 160!=p.payload.audio.num_samples)
				dump("Improper audio packet:", &p, packet_size(p));
			buffer_depth--;
			packet->SetAudioSamples(p.payload.audio.samples, true);
		}
		else
		{
			dump("ReadDevice() ERROR: Read an unexpected device packet:", &p, packet_size(p));
			return;
		}
		if (Encoding::dstar == type)	// is this a DMR or a DStar device?
		{
			g_Cont.dstar_mux.lock();
			g_Cont.RouteDstPacket(packet);
			g_Cont.dstar_mux.unlock();
		}
		else
		{
			g_Cont.dmrst_mux.lock();
			g_Cont.RouteDmrPacket(packet);
			g_Cont.dmrst_mux.unlock();
		}
	}
}
