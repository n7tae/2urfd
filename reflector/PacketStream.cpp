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


#include "PacketStream.h"
#include "Global.h"

////////////////////////////////////////////////////////////////////////////////////////
// constructor

CPacketStream::CPacketStream(char module, bool istc) : m_PSModule(module), m_IsTranscoded(istc)
{
	m_uiStreamId = 0;
	m_uiPacketCntr = 0;
	m_OwnerClient = nullptr;
	std::cout << "PacketStream[" << m_PSModule << "] allocated and " << (m_IsTranscoded ? "IS " : "is NOT ") << "transcoded." << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////
// open / close

bool CPacketStream::OpenPacketStream(const CDvHeaderPacket &DvHeader, std::shared_ptr<CClient>client)
{
	// not already open?
	if ( 0 == m_uiStreamId )
	{
		// update status
		m_uiStreamId = DvHeader.GetStreamId();
		m_uiPacketCntr = 0;
		m_DvHeader = DvHeader;
		m_OwnerClient = client;
		m_LastPacketTime.start();
		if (m_IsTranscoded)
			ResetStats();
		return true;
	}
	return false;
}

void CPacketStream::ClosePacketStream(void)
{
	// update status
	m_uiStreamId = 0;
	m_OwnerClient.reset();
	if(m_IsTranscoded)
		ReportStats();
	while (not m_HeaderQueue.empty())
	{
		auto header = std::move(m_HeaderQueue.front());
		std::cerr << "ERROR: Module " << m_PSModule << " Header packet from " << header->GetMyCallsign() << " with SID " << std::hex << std::showbase << header->GetStreamId() << std::dec << std::noshowbase << " was not processed!" << std::endl;
		m_HeaderQueue.pop_front();
	}
	while (not m_TCQueue.empty())
	{
		auto tp = m_TCQueue.front().tcpacket;
		std::cerr << "ERROR: Module " << m_PSModule << " Frame packet with SID " << std::hex << std::showbase << tp->GetStreamId() << std::dec << std::noshowbase << " was not processed!" << std::endl;
		m_TCQueue.pop_front();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// Statistics
void CPacketStream::ResetStats()
{
	m_RTMin = 1.0e6;
	m_RTMax = -1.0;
	m_RTSum = 0.0;
	m_RTSumSq = 0.0;
}

void CPacketStream::ReportStats()
{
	if (m_uiPacketCntr > 0)
	{
		double norm = 1.0 / double(m_uiPacketCntr);
		double min = 1000.0 * m_RTMin;
		double max = 1000.0 * m_RTMax;
		double average = m_RTSum * norm;
		double ave = 1000.0 * average;
		double dev = 1000.0 * sqrt((m_RTSumSq * norm) - (average * average));
		auto prec = std::cout.precision();
		std::cout.precision(1);
		std::cout << std::fixed << "TC round-trip time(ms): " << min << '/' << ave << '(' << dev << ")/" << max << ", " << m_uiPacketCntr << " total packets" << std::endl;
		std::cout.precision(prec);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// action

void CPacketStream::Update(double rt)
{
	// update statistics
	if (rt < m_RTMin)
		m_RTMin = rt;
	else if (rt > m_RTMax)
		m_RTMax = rt;
	m_RTSum += rt;
	m_RTSumSq += rt * rt;
	m_uiPacketCntr++;
	if (m_TCQueue.empty())
	{
		std::cerr << "The transcoder sent an update, but the transcoder queue is empty!" << std::endl;
	}
	else
	{
		while (m_TCQueue.front().tcpacket->AllCodecsAreSet())
		{
			// if there is a waiting frame pack, now is the time for it
			// there should just be one, but if not we'll do 'em all
			while (not m_HeaderQueue.empty())
			{
				m_Queue.Push(std::move(m_HeaderQueue.front()));
				m_HeaderQueue.pop_front();
			}
			// get both pointers from the front
			auto fp = std::move(m_TCQueue.front().fpacket);
			auto tcp = m_TCQueue.front().tcpacket;
			// throw away the STCFP container
			m_TCQueue.pop_front();
			// update the frame with the codec data
			fp->SetCodecData(tcp->GetTCPacket());
			// push it back to the reflector where it can be
			// distriubed to all clients using the client's protocol
			m_Queue.Push(std::move(fp));
			if (m_TCQueue.empty())
				return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// push

void CPacketStream::Push(std::unique_ptr<CPacket> Packet)
{
	if (Packet->GetStreamId() != m_uiStreamId)
	{
#ifdef DEBUG
		std::cout << "PacketStream[" << m_PSModule << "]: Packet ignored. ";
		if (0 == m_uiStreamId)
			std::cout << "The stream is closed: ";
		if (Packet->IsDvFrame())
			std::cout << "Frame";
		else
			std::cout << "Header";
		std::cout << std::hex << std::showbase << "Packet SID=" << ntohs(Packet->GetStreamId()) << std::noshowbase << std::dec << std::endl;
#endif
		return;
	}

	m_LastPacketTime.start();
	
	if (m_IsTranscoded and Packet->IsLocalOrigin())
	{
		if (Packet->IsDvHeader())
		{
			// recast to a header packet
			std::unique_ptr<CDvHeaderPacket> Header(static_cast<CDvHeaderPacket *>(Packet.release()));
			// push this into the holding area for headers
			m_HeaderQueue.push_back(std::move(Header));
		}
		else
		{
			Packet->UpdatePids(m_uiPacketCntr);
			// recast to a frame packet
			std::unique_ptr<CDvFramePacket> frame(static_cast<CDvFramePacket *>(Packet.release()));
			// update the tcp param
			frame->SetTCParams(m_uiPacketCntr);
			// create the transcoder packet from the codec packet data
			auto tcp = std::make_shared<CTranscoderPacket>(*frame->GetCodecPacket());
			
			m_TCQueue.push_back(STCFP());
			m_TCQueue.back().fpacket = std::move(frame);
			m_TCQueue.back().tcpacket = tcp;
			// and send the transcoder packet to the TC
			g_Transcoder.Transcode(tcp);
		}
	}
	else
	{
		// bypass the transcoder
		m_Queue.Push(std::move(Packet));
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// get

const CIp *CPacketStream::GetOwnerIp(void)
{
	if ( m_OwnerClient != nullptr )
	{
		return &(m_OwnerClient->GetIp());
	}
	return nullptr;
}
