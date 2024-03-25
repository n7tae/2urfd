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


#include <string.h>

#include "Global.h"
#include "DStarDirectClient.h"
#include "DStarDirectProtocol.h"

CDStarDirectProtocol::CDStarDirectProtocol(const std::string &name) : CProtocol(name), m_IRCLogin(g_Configure.GetString(g_Keys.dsd.ircLogin)) {}

////////////////////////////////////////////////////////////////////////////////////////
// operation

bool CDStarDirectProtocol::Initialize(const char *type, const EProtocol ptype, const uint16_t port, const bool has_ipv4, const bool has_ipv6)
{
	// base class
	if (! CProtocol::Initialize(type, ptype, port, has_ipv4, has_ipv6))
		return false;

	// update time
	m_LastKeepaliveTime.start();

	// done
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
// task

void CDStarDirectProtocol::Task(void)
{
	CBuffer             Buffer;
	CIp                 Ip;
	CCallsign           Callsign;
	char                ToLinkModule;
	EProtoRev           ProtRev;
	std::unique_ptr<CDvHeaderPacket>    Header;
	std::unique_ptr<CDvFramePacket>     Frame;

	// any incoming packet ?
#if DSD_IPV6==true
#if DSD_IPV4==true
	if ( ReceiveDS(Buffer, Ip, 20) )
#else
	if ( Receive6(Buffer, Ip, 20) )
#endif
#else
	if ( Receive4(Buffer, Ip, 20) )
#endif
	{
		// crack the packet
		if ( IsValidDvFramePacket( Buffer, Frame) )
		{
			OnDvFramePacketIn( Frame, &Ip );

			if ( nullptr == Frame ) // the packet's stream was found (the frame was consumed), so
				return;             // we're done

			// see if we can find the streamID in the action map
			const auto streamID = Frame->GetStreamId();
			auto actionMapPair = m_ActionMap.find( streamID );
			if ( m_ActionMap.end() == actionMapPair )
				return;	// nope, we're done

			auto &actionItem = actionMapPair->second;

			if ( Frame->IsLastPacket() )
			{
				// we need to do the action and send the acknowledgement
				DoAction(actionItem);
			}
			else
			{
				actionItem.Update();	// tickle the action item
			}
		}
		else if ( IsValidDvHeaderPacket(Buffer, Header) )
		{
			if ( g_GateKeeper.MayTransmit(Header->GetMyCallsign(), Ip, EProtocol::dsd) )
			{
				// if the stream isn't open, this will open it
				OnDvHeaderPacketIn(Header, Ip);
			}
		}
		else if ( IsValidKeepAlivePacket(Buffer, &Callsign) )
		{
			//std::cout << "DSD keepalive packet from " << Callsign << " at " << Ip << std::endl;

			// find all clients with that callsign & ip and keep them alive
			CClients *clients = g_Reflector.GetClients();
			auto it = clients->begin();
			std::shared_ptr<CClient>client = nullptr;
			while ( (client = clients->FindNextClient(Callsign, Ip, EProtocol::dsd, it)) != nullptr )
			{
				client->Alive();
			}
			g_Reflector.ReleaseClients();
		}
		else
		{
			std::string title("Unknown DSD packet from ");
			title += Ip.GetAddress();
			Buffer.Dump(title);
		}
	}

	// handle end of streaming timeout
	CheckStreamsTimeout();

	// check for timeouts and completions
	CheckActionMap();

	// handle queue from reflector
	HandleQueue();

	// keep alive
	if ( m_LastKeepaliveTime.time() > DEXTRA_KEEPALIVE_PERIOD )
	{
		// handle keep alives
		HandleKeepalives();

		// update time
		m_LastKeepaliveTime.start();
	}
}

void CDStarDirectProtocol::CheckActionMap(void)
{
	// are any complete?
	for (auto it=m_ActionMap.begin(); it!=m_ActionMap.end(); )
	{
		if (it->second.ActionIsCompleted())
		{
			it->second.m_future.get();
			it = m_ActionMap.erase(it);
		}
		else
		{
			it++;
		}
	}

	// have any timed out?
	for (auto &pair : m_ActionMap)
	{
		if (pair.second.GetTime() > STREAM_TIMEOUT)
		{
			DoAction(pair.second);
			pair.second.Update();
		}
	}
}

void CDStarDirectProtocol::DoAction(CDSDAction &actionItem)
{
	switch (actionItem.GetType())
	{
		case EDSDAction::logon:
			g_Reflector.GetClients()->AddClient(actionItem.GetNewClient());
			g_Reflector.ReleaseClients();
			actionItem.m_future = std::async(std::launch::async, &CDStarDirectProtocol::SendResponse, this, std::ref(actionItem));
			break;
		case EDSDAction::logoff:
			g_Reflector.GetClients()->RemoveClient(actionItem.GetOldClient());
			g_Reflector.ReleaseClients();
			actionItem.m_future = std::async(std::launch::async, &CDStarDirectProtocol::SendResponse, this, std::ref(actionItem));
			break;
		case EDSDAction::change:
			auto clients = g_Reflector.GetClients();
			clients->RemoveClient(actionItem.GetOldClient());
			clients->AddClient(actionItem.GetNewClient());
			g_Reflector.ReleaseClients();
			actionItem.m_future = std::async(std::launch::async, &CDStarDirectProtocol::SendResponse, this, std::ref(actionItem));
			break;
	}
}

void CDStarDirectProtocol::SendResponse(CDSDAction &actionItem)
{
	auto client = std::static_pointer_cast<CDStarDirectClient>((actionItem.GetType()==EDSDAction::logoff) ? actionItem.GetOldClient() : actionItem.GetNewClient());

	// build the Header packet
	struct dstar_header header;
	header.Flag1 = header.Flag2 = header.Flag3 = 0;
	client->GetCallsign().GetCallsign(header.UR);
	client->GetRepeater().GetCallsign(header.RPT2);
	memcpy(header.RPT2, header.RPT1, CALLSIGN_LEN-1);
	header.RPT1[CALLSIGN_LEN-1] = 'G';
	m_ReflectorCallsign.GetCallsign(header.MY);
	header.MY[CALLSIGN_LEN-1] = client->GetClientModule();
	// calc CRC???
	header.Crc = 0;
	const uint16_t streamID = htons(m_Random.NewStreamID());
	CDvHeaderPacket hp(&header, streamID, 0x80u);
	// send it
	CBuffer buf;
	EncodeDvHeaderPacket(hp, buf);
	Send(buf, client->GetIp());

	// stream
	SDStarFrame dstar;
	const uint8_t silence[9] = { 0x9E, 0x8D, 0x32, 0x88, 0x26, 0x1A, 0x3F, 0x61, 0xE8 };
	memcpy(dstar.AMBE, silence, 9);
	std::string msg((EDSDAction::logoff==actionItem.GetType()) ? "Logged OFF " : "Logged ON ");
	msg += hp.GetMyCallsign().GetCS();
	msg.resize(20, ' ');

	for (uint8_t i=0; i<10; i++)
	{
		switch (i)
		{
		case 0:	// sync voice frame
			dstar.DVDATA[0] = 0x55;
			dstar.DVDATA[1] = 0x2d;
			dstar.DVDATA[2] = 0x16;
			break;
		case 1:
			dstar.DVDATA[0] = '@' ^ 0x70;
			dstar.DVDATA[1] = msg[0] ^ 0x4f;
			dstar.DVDATA[2] = msg[1] ^ 0x93;
			break;
		case 2:
			dstar.DVDATA[0] = msg[2] ^ 0x70;
			dstar.DVDATA[1] = msg[3] ^ 0x4f;
			dstar.DVDATA[2] = msg[4] ^ 0x93;
			break;
		case 3:
			dstar.DVDATA[0] = 'A' ^ 0x70;
			dstar.DVDATA[1] = msg[5] ^ 0x4f;
			dstar.DVDATA[2] = msg[6] ^ 0x93;
			break;
		case 4:
			dstar.DVDATA[0] = msg[7] ^ 0x70;
			dstar.DVDATA[1] = msg[8] ^ 0x4f;
			dstar.DVDATA[2] = msg[9] ^ 0x93;
			break;
		case 5:
			dstar.DVDATA[0] = 'B' ^ 0x70;
			dstar.DVDATA[1] = msg[10] ^ 0x4f;
			dstar.DVDATA[2] = msg[11] ^ 0x93;
			break;
		case 6:
			dstar.DVDATA[0] = msg[12] ^ 0x70;
			dstar.DVDATA[1] = msg[13] ^ 0x4f;
			dstar.DVDATA[2] = msg[14] ^ 0x93;
			break;
		case 7:
			dstar.DVDATA[0] = 'C' ^ 0x70;
			dstar.DVDATA[1] = msg[15] ^ 0x4f;
			dstar.DVDATA[2] = msg[16] ^ 0x93;
			break;
		case 8:
			dstar.DVDATA[0] = msg[17] ^ 0x70;
			dstar.DVDATA[1] = msg[18] ^ 0x4f;
			dstar.DVDATA[2] = msg[19] ^ 0x93;
			break;
		case 9:	// terminal voice packet
			i |= 0x40u;
			dstar.DVDATA[0] = 0x70;
			dstar.DVDATA[1] = 0x4f;
			dstar.DVDATA[2] = 0x93;
			break;
		}
		CDvFramePacket frame(&dstar, streamID, i);
		EncodeDvFramePacket(frame, buf);
		Send(buf, client->GetIp());
		if (i > 8u) std::this_thread::sleep_for(std::chrono::milliseconds(19));
	}
	actionItem.Completed();
}

////////////////////////////////////////////////////////////////////////////////////////
// queue helper

void CDStarDirectProtocol::HandleQueue(void)
{
	while (! m_Queue.IsEmpty() )
	{
		// get the packet
		auto packet = m_Queue.Pop();

		// encode it
		CBuffer buffer;
		bool encoded = false;
		if ( packet->IsDvFrame() )
			encoded = EncodeDvFramePacket((CDvFramePacket &)packet, buffer);
		else if ( packet->IsDvHeader() )
			encoded = EncodeDvHeaderPacket((CDvHeaderPacket &)packet, buffer);
		if ( encoded )
		{
			// and push it to all our clients linked to the module and who are not streaming in
			CClients *clients = g_Reflector.GetClients();
			auto it = clients->begin();
			std::shared_ptr<CClient>client = nullptr;
			while ( (client = clients->FindNextClient(EProtocol::dsd, it)) != nullptr )
			{
				// is this client busy ?
				if ( !client->IsAMaster() && (client->GetReflectorModule() == packet->GetPacketModule()) )
				{
					// no, send the packet
					int n = packet->IsDvHeader() ? 3 : 1;
					for ( int i = 0; i < n; i++ )
					{
						Send(buffer, client->GetIp());
					}
				}
			}
			g_Reflector.ReleaseClients();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// keepalive helpers

void CDStarDirectProtocol::HandleKeepalives(void)
{
	// DSD protocol sends and monitors keepalives packets
	// event if the client is currently streaming
	// so, send keepalives to all
	CBuffer keepalive;
	EncodeKeepAlivePacket(&keepalive);

	// iterate on clients
	CClients *clients = g_Reflector.GetClients();
	auto it = clients->begin();
	std::shared_ptr<CClient>client = nullptr;
	while ( (client = clients->FindNextClient(EProtocol::dsd, it)) != nullptr )
	{
		// send keepalive
		Send(keepalive, client->GetIp());

		// client busy ?
		if ( client->IsAMaster() )
		{
			// yes, just tickle it
			client->Alive();
		}
		// otherwise check if still with us
		else if ( !client->IsAlive() )
		{
			// remove it
			std::cout << "DSD client " << client->GetCallsign() << " keepalive timeout" << std::endl;
			clients->RemoveClient(client);
		}

	}
	g_Reflector.ReleaseClients();
}

////////////////////////////////////////////////////////////////////////////////////////
// streams helpers

void CDStarDirectProtocol::OnDvHeaderPacketIn(std::unique_ptr<CDvHeaderPacket> &Header, const CIp &Ip)
{
	/*
	All the possible reasons we're here:
		1. This is just a duplicate header in the client's stream. There are two possibilities:
		   A. Part of a valid open stream.
		   B. Part of a stream involved in an action (2 - 5).
		2. This is an existing client starting a new stream.
		   Action: Open the target module's stream
		3. This is an existing client changing reflector modules.
		4. This is an existing client unsubscribing.
		5. This is an existing client, but he has changed repeaters, "follow-me"
		6. This is a new client subscribing.
	*/
	const auto streamID = Header->GetStreamId();
	auto stream = GetStream(streamID);

	if ( stream )	// this is 1A
	{
		stream->Tickle();
		return;
	}

	auto actionMapItem = m_ActionMap.find(streamID);
	if ( m_ActionMap.end() != actionMapItem )	// this is 1B
	{
		actionMapItem->second.Update();
		return;
	}

	const auto usercs = Header->GetMyCallsign();
	auto repeater = Header->GetRpt2Callsign();

	auto client = g_Reflector.GetClients()->FindClient(usercs, EProtocol::dsd);
	g_Reflector.ReleaseClients();

	if ( nullptr == client)	// this is 6
	{
		const auto urmod = Header->GetUrCallsign().GetModule();
		if (g_Reflector.IsValidModule(urmod))
		{
			// make the new client
			client = std::make_shared<CDStarDirectClient>(usercs, EProtocol::dsd, repeater, Ip, urmod);
			// emplace the new action map item
			auto rv = m_ActionMap.emplace(std::piecewise_construct,
				std::forward_as_tuple(streamID),
				std::forward_as_tuple(EDSDAction::logon, nullptr, client) );
			if (! rv.second)
			{
				std::cerr << "SID COLLISION: The action map already contains an item with this streamID!" << std::endl;
				std::cout << usercs << " cannot login to module '" << urmod << "'" << std::endl;
			}
		}
		else
		{
			std::cout << usercs << " trying to login into module '" << urmod << "'" << std::endl;
		}
	}
	else
	{
		// we have found the client
		auto repeater = Header->GetRpt2Callsign();
		// this is an existing client. What's he doing?
		const auto urmod = Header->GetUrCallsign().GetModule();
		const auto clmod = client->GetClientModule();

		if (clmod == urmod)	// this is 2
		{	// client wants to talk, open the reflector module stream
			CCallsign my(Header->GetMyCallsign());
			CCallsign rpt1(Header->GetRpt1Callsign());
			CCallsign rpt2(Header->GetRpt2Callsign());

			// find this client
			std::shared_ptr<CClient>client = g_Reflector.GetClients()->FindClient(Ip, EProtocol::dsd);
			if ( client )
			{
				// get client callsign
				rpt1 = client->GetCallsign();
				// and try to open the stream
				if ( (stream = g_Reflector.OpenStream(Header, client)) != nullptr )
				{
					// keep the handle
					m_Streams[stream->GetStreamId()] = stream;
				}
			}
			// release
			g_Reflector.ReleaseClients();

			// update last heard
			g_Reflector.GetUsers()->Hearing(my, rpt1, rpt2);
			g_Reflector.ReleaseUsers();
		}
		else if (clmod != urmod)	// this is 3
		{
			if (g_Reflector.IsValidModule(urmod))
			{
				auto newclient = std::make_shared<CDStarDirectClient>(usercs, EProtocol::dsd, repeater, Ip, urmod);
				auto rv = m_ActionMap.emplace(std::piecewise_construct,
					std::forward_as_tuple(streamID),
					std::forward_as_tuple(EDSDAction::change, client, newclient) );
				if (! rv.second)
				{
					std::cerr << "SID COLLISION: The action map already contains an item with this streamID!" << std::endl;
					std::cout << usercs << " can't change to module '" << urmod << "'" << std::endl;
				}
			}
			else
			{
				std::cout << usercs << " is trying to change to module '" << urmod << "'" << std::endl;
			}
		}
		else if (' ' == urmod)	// this is 4.
		{
			auto rv = m_ActionMap.emplace(std::piecewise_construct,
				std::forward_as_tuple(streamID),
				std::forward_as_tuple(EDSDAction::logoff, client, nullptr) );
			if (! rv.second)
				std::cerr << "SID COLLISION: The action map already contains an item with this streamID!" << std::endl;
		}
		else
		{
			std::cerr << "Unexpected routing error: From client " << client->GetCallsign() << " on module '" << clmod << "', URModule is '" << urmod << "'" << std::endl;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// packet decoding helpers

bool CDStarDirectProtocol::IsValidKeepAlivePacket(const CBuffer &Buffer, CCallsign *callsign)
{
	bool valid = false;
	if (Buffer.size() == 9)
	{
		callsign->SetCallsign(Buffer.data(), 8);
		valid = callsign->IsValid();
	}
	return valid;
}

// is the right size, begins with "DSVT", byte 4 is 0x10, byte 8 is 0x20, RPT{1,2} and MYCALL are valid
// and URCALL is DSD<???>, where <???> is this reflector's configured callsign, URF???
bool CDStarDirectProtocol::IsValidDvHeaderPacket(const CBuffer &Buffer, std::unique_ptr<CDvHeaderPacket> &header)
{
	if ( 56==Buffer.size() && 0==Buffer.Compare((uint8_t *)"DSVT", 4) && 0x10U==Buffer.data()[4] && 0x20U==Buffer.data()[8] )
	{
		// create packet
		header = std::make_unique<CDvHeaderPacket>((struct dstar_header *)&(Buffer.data()[15]), *((uint16_t *)&(Buffer.data()[12])), 0x80);
		// check validity of packet
		if ( header && header->IsValid() )
			return header->GetUrCallsign().HasSameCallsign(m_ReflectorCallsign);
	}
	return false;
}

// is the right size, begins with "DSVT", bytes 4 and 8 are 0x20u
bool CDStarDirectProtocol::IsValidDvFramePacket(const CBuffer &Buffer, std::unique_ptr<CDvFramePacket> &dvframe)
{
	if ( 27==Buffer.size() && 0==Buffer.Compare((uint8_t *)"DSVT", 4) && 0x20U==Buffer.data()[4] && 0x20U==Buffer.data()[8] )
	{
		// create packet
		dvframe = std::make_unique<CDvFramePacket>((SDStarFrame *)&(Buffer.data()[15]), *((uint16_t *)&(Buffer.data()[12])), Buffer.data()[14]);
		// check validity of packet
		if ( dvframe && dvframe->IsValid() )
			return true;
	}
	return false;
}


////////////////////////////////////////////////////////////////////////////////////////
// packet encoding helpers

void CDStarDirectProtocol::EncodeKeepAlivePacket(CBuffer *Buffer)
{
	Buffer->Set(GetReflectorCallsign());
}

bool CDStarDirectProtocol::EncodeDvHeaderPacket(const CDvHeaderPacket &Packet, CBuffer &Buffer) const
{
	uint8_t tag[]	= { 'D','S','V','T',0x10,0x00,0x00,0x00,0x20,0x00,0x01,0x02 };
	struct dstar_header DstarHeader;

	Packet.ConvertToDstarStruct(&DstarHeader);

	Buffer.Set(tag, sizeof(tag));
	Buffer.Append(Packet.GetStreamId());
	Buffer.Append((uint8_t)0x80);
	Buffer.Append((uint8_t *)&DstarHeader, sizeof(struct dstar_header));

	return true;
}

bool CDStarDirectProtocol::EncodeDvFramePacket(const CDvFramePacket &Packet, CBuffer &Buffer) const
{
	uint8_t tag[] = { 'D','S','V','T',0x20,0x00,0x00,0x00,0x20,0x00,0x01,0x02 };

	Buffer.Set(tag, sizeof(tag));
	Buffer.Append(Packet.GetStreamId());
	uint8_t id = Packet.GetDstarPacketId() % 21;
	if (Packet.IsLastPacket())
		id |= 0x40U;
	Buffer.Append(id);
	Buffer.Append((uint8_t *)Packet.GetCodecData(ECodecType::dstar), 9);
	Buffer.Append((uint8_t *)Packet.GetDvData(), 3);

	return true;

}
