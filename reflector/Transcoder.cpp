// tcd - a hybid transcoder using DVSI hardware and Codec2 software
// Copyright © 2021 Thomas A. Early N7TAE
// Copyright © 2021 Doug McLain AD8DP

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

#include <unistd.h>
#include <sys/select.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <thread>

#include "TranscoderPacket.h"

#include "Global.h"

int32_t CTranscoder::calcNumerator(int32_t db) const
{
	float num = 256.0f * powf(10.0f, (float(db)/20.0f));

	return int32_t(roundf(num));
}

CTranscoder::CTranscoder() : keep_running(true) {}

bool CTranscoder::Start()
{
	m_tcmods = g_Configure.GetString(g_Keys.tc.tcmodules);
	if (InitVocoders()) // this will also create the inQmap queues
	{
		keep_running = false;
		return true;
	}

	try
	{
		reflectorFuture = std::async(std::launch::async, &CTranscoder::ReadReflectorThread, this);
		c2Future        = std::async(std::launch::async, &CTranscoder::ProcessC2Thread,     this);
		imbeFuture      = std::async(std::launch::async, &CTranscoder::ProcessIMBEThread,   this);
	}
	catch (std::exception &e)
	{
		keep_running = false;
		std::cerr << "ERROR: Failure launching treads: " << e.what() << std::endl;
		return true;
	}
	return false;
}

void CTranscoder::Stop()
{
	keep_running = false;

	if (reflectorFuture.valid())
		reflectorFuture.get();
	if (c2Future.valid())
		c2Future.get();
	if (imbeFuture.valid())
		imbeFuture.get();
	inQmap.clear();

	dstar_device->CloseDevice();
	dmrsf_device->CloseDevice();
	dstar_device.reset();
	dmrsf_device.reset();
	for (const auto m : m_tcmods)
	{
		c2_16[m].reset();
		c2_32[m].reset();
		p25vocoder[m].reset();
	}
}

bool CTranscoder::DiscoverFtdiDevices(std::list<std::pair<std::string, std::string>> &found)
{
	int iNbDevices = 0;
	auto status = FT_CreateDeviceInfoList((LPDWORD)&iNbDevices);
	if (FT_OK != status)
	{
		std::cerr << "Could not create FTDI device list" << std::endl;
		return true;
	}

	std::cout << "Detected " << iNbDevices << " USB-FTDI-based DVSI devices" << std::endl;
	if ( iNbDevices > 0 )
	{
		// allocate the list
		FT_DEVICE_LIST_INFO_NODE *list = new FT_DEVICE_LIST_INFO_NODE[iNbDevices];
		if (nullptr == list)
		{
			std::cerr << "Could not create new device list" << std::endl;
			return true;
		}

		// fill
		status = FT_GetDeviceInfoList(list, (LPDWORD)&iNbDevices);
		if (FT_OK != status)
		{
			std::cerr << "Could not get FTDI device list" << std::endl;
			return true;
		}

		for ( int i = 0; i < iNbDevices; i++ )
		{
			std::cout << "Found " << list[i].Description << ", SN=" << list[i].SerialNumber << std::endl;
			found.emplace_back(std::pair<std::string, std::string>(list[i].SerialNumber, list[i].Description));
		}

		// and delete
		delete[] list;
	}

	// done
	return false;
}

bool CTranscoder::InitVocoders()
{
	// M17 "devices", one for each module
	for ( auto c : m_tcmods)
	{
		c2_16[c] = std::make_unique<CCodec2>(false);
		c2_32[c] = std::make_unique<CCodec2>(true);
		p25vocoder[c] = std::make_unique<imbe_vocoder_impl>();
		inQmap.emplace(c, std::make_unique<CPacketQueue>());
	}

	// the 3000 or 3003 devices
	std::list<std::pair<std::string, std::string>> deviceset;

	if (DiscoverFtdiDevices(deviceset))
		return true;

	if (deviceset.empty()) {
		std::cerr << "could not find a device!" << std::endl;
		return true;
	}

	if (2 != deviceset.size())
	{
		std::cerr << "Could not find exactly two DVSI devices" << std::endl;
		return true;
	}

	const auto desc(deviceset.front().second);
	if (deviceset.back().second.compare(desc))
	{
		if (desc.compare(0, 9, "USB-3006 ")) // the USB-3006 device doesn't need this check
		{
			std::cout << "Both devices should to be the same type: " << desc << " != " << deviceset.back().second << std::endl;
		}
	}

	Edvtype dvtype = Edvtype::dv3003;
	if (0==desc.compare("ThumbDV") || 0==desc.compare("DVstick-30") || 0==desc.compare("USB-3000") || 0==desc.compare("FT230X Basic UART"))
		dvtype = Edvtype::dv3000;

	if (m_tcmods.size() > ((Edvtype::dv3000 == dvtype) ? 1 : 3))
	{
		std::cerr << "Too many transcoded modules for the devices" << std::endl;
		return true;
	}

	for (unsigned int i=0; i<m_tcmods.size(); i++)
	{
		auto c = m_tcmods.at(i);
		if (c < 'A' || c > 'Z') {
			std::cerr << "Transcoded modules[" << i << "] is not an uppercase letter!" << std::endl;
			return true;
		}
	}

	//initialize each device
	while (! deviceset.empty())
	{
		if (Edvtype::dv3000 == dvtype)
		{
			dstar_device = std::make_unique<CDV3000>(Encoding::dstar);
			dmrsf_device = std::make_unique<CDV3000>(Encoding::dmrsf);
		}
		else
		{
			dstar_device = std::make_unique<CDV3003>(Encoding::dstar);
			dmrsf_device = std::make_unique<CDV3003>(Encoding::dmrsf);
		}

		if (dstar_device)
		{
			if (dstar_device->OpenDevice(deviceset.front().first, deviceset.front().second, dvtype, int8_t(g_Configure.GetInt(g_Keys.tc.dstargainin)), int8_t(g_Configure.GetInt(g_Keys.tc.dstargainout))))
				return true;
			deviceset.pop_front();
		}
		else
		{
			std::cerr << "Could not create DVSI devices!" << std::endl;
			return true;
		}
		if (dmrsf_device)
		{
			if (dmrsf_device->OpenDevice(deviceset.front().first, deviceset.front().second, dvtype, int8_t(g_Configure.GetInt(g_Keys.tc.dmrgainin)), int8_t(g_Configure.GetInt(g_Keys.tc.dmrgainout))))
				return true;
			deviceset.pop_front();
		}
		else
		{
			std::cerr << "Could not create DVSI devices!" << std::endl;
			return true;
		}
	}

	// and start them (or it) up!
	dstar_device->Start();
	dmrsf_device->Start();

	deviceset.clear();

	return false;
}

void CTranscoder::Transcode(std::shared_ptr<CTranscoderPacket> tcp)
{
	
	auto qit = inQmap.find(tcp->GetModule());
	if (inQmap.end() == qit)
	{
		Dump(tcp, "ERROR: Transcoder received a packet for a unconfigured module:");
		return;
	}
	qit->second->push(tcp);
}

void CTranscoder::ReadReflectorThread()
{
	auto go_time = std::chrono::steady_clock::now();

	while (keep_running)
	{
		go_time += std::chrono::milliseconds(20);
		std::this_thread::sleep_until(go_time);

		for (auto &pair : inQmap)
		{
			if (pair.second->IsEmpty())
				continue;

			auto packet = pair.second->pop();
			switch (packet->GetCodecIn())
			{
			case ECodecType::dstar:
				dstar_device->AddPacket(packet);
				break;
			case ECodecType::dmr:
				dmrsf_device->AddPacket(packet);
				break;
			case ECodecType::p25:
				imbe_queue.push(packet);
				break;
			case ECodecType::c2_1600:
			case ECodecType::c2_3200:
				codec2_queue.push(packet);
				break;
			default:
				Dump(packet, "ERROR: Received a reflector packet with unknown Codec:");
				break;
			}
		}
	}
	std::cout << "Read reflector thread shut down" << std::endl;
}

// This is only called when codec_in was dstar or dmr. Obviously, the incoming
// ambe packet was already decoded to audio.
// This might complete the packet. If so, send it back to the reflector
void CTranscoder::AudiotoCodec2(std::shared_ptr<CTranscoderPacket> packet)
{
	// the second half is silent in case this is frame is last.
	uint8_t m17data[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x01, 0x43, 0x09, 0xe4, 0x9c, 0x08, 0x21 };
	const auto m = packet->GetModule();
	if (packet->IsSecond())
	{
		// get the first half from the store
		memcpy(m17data, data_store[packet->GetModule()], 8);
		// and then calculate the second half
		c2_32[m]->codec2_encode(m17data+8, packet->GetAudioSamples());
		packet->SetM17Data(m17data);
	}
	else /* the packet is first */
	{
		// calculate the first half...
		c2_32[m]->codec2_encode(m17data, packet->GetAudioSamples());
		// and then copy the calculated data to the data_store
		memcpy(data_store[packet->GetModule()], m17data, 8);
		// set the m17_is_set flag if this is the last packet
		packet->SetM17Data(m17data);
	}

	// we might be all done...
	if (packet->AllCodecsAreSet()) 
		UpdateStream(packet);
}

// The original incoming coded was M17, so we will calculate the audio and then
// push the packet onto both the dstar and the dmr queue.
void CTranscoder::Codec2toAudio(std::shared_ptr<CTranscoderPacket> packet)
{
	const auto m = packet->GetModule();

	if (packet->IsSecond())
	{
		if (packet->GetCodecIn() == ECodecType::c2_1600)
		{
			// we've already calculated the audio in the previous packet
			// copy the audio from local audio store
			packet->SetAudioSamples(audio_store[m], false);
		}
		else /* codec_in is ECodecType::c2_3200 */
		{
			int16_t tmp[160];
			// decode the second 8 data bytes
			// and put it in the packet
			c2_32[m]->codec2_decode(tmp, packet->GetM17Data()+8);
			packet->SetAudioSamples(tmp, false);
		}
	}
	else /* it's a "first packet" */
	{
		if (packet->GetCodecIn() == ECodecType::c2_1600)
		{
			// c2_1600 encodes 40 ms of audio, 320 points, so...
			// we need some temporary audio storage for decoding c2_1600:
			int16_t tmp[320];
			// decode it into the temporary storage
			c2_16[m]->codec2_decode(tmp, packet->GetM17Data()); // 8 bytes input produces 320 audio points
			// move the first and second half
			// the first half is for the packet
			packet->SetAudioSamples(tmp, false);
			// and the second half goes into the audio store
			memcpy(audio_store[packet->GetModule()], &(tmp[160]), 320);
		}
		else /* codec_in is ECodecType::c2_3200 */
		{
			int16_t tmp[160];
			c2_32[m]->codec2_decode(tmp, packet->GetM17Data());
			packet->SetAudioSamples(tmp, false);
		}
	}
	// the only thing left is to encode the other codecs, so push the packet onto all the other queues
	dstar_device->AddPacket(packet);
	dmrsf_device->AddPacket(packet);
	imbe_queue.push(packet);
}

void CTranscoder::ProcessC2Thread()
{
	while (keep_running)
	{
		auto packet = codec2_queue.pop();

		const auto codecIn = packet->GetCodecIn();
		if (codecIn == ECodecType::c2_1600 or codecIn == ECodecType::c2_3200)
				// this is an original M17 packet, so decode it to audio
				// Codec2toAudio will send it on for AMBE processing
				Codec2toAudio(packet);
		else
				// codec_in was AMBE, so we need to calculate the the M17 data
				AudiotoCodec2(packet);
	}
	std::cout << "Codec2 process thread shut down" << std::endl;
}

void CTranscoder::AudiotoIMBE(std::shared_ptr<CTranscoderPacket> packet)
{
	uint8_t imbe[11];

	p25vocoder[packet->GetModule()]->encode_4400((int16_t *)packet->GetAudioSamples(), imbe);
	packet->SetP25Data(imbe);
	// we might be all done...
	if (packet->AllCodecsAreSet()) 
		UpdateStream(packet);
}

void CTranscoder::IMBEtoAudio(std::shared_ptr<CTranscoderPacket> packet)
{
	int16_t tmp[160] = { 0 };
	p25vocoder[packet->GetModule()]->decode_4400(tmp, (uint8_t*)packet->GetP25Data());
	packet->SetAudioSamples(tmp, false);
	dstar_device->AddPacket(packet);
	dmrsf_device->AddPacket(packet);
	codec2_queue.push(packet);
}

void CTranscoder::ProcessIMBEThread()
{
	while (keep_running)
	{
		auto packet = imbe_queue.pop();

		if (ECodecType::p25 == packet->GetCodecIn())
			IMBEtoAudio(packet);
		else
			AudiotoIMBE(packet);
	}
	std::cout << "IMBE process thread shut down" << std::endl;
}

void CTranscoder::UpdateStream(std::shared_ptr<CTranscoderPacket> packet)
{
	const auto m = packet->GetModule();
	auto stream = g_Reflector.GetStream(m);
	if (stream)
		stream->Update(packet->GetTimer().time());
	else
		std::cerr << "CTranscoder::UpdateStrream could not find a PacketStream[" << m << "]!" << std::endl;
}

void CTranscoder::RouteDstPacket(std::shared_ptr<CTranscoderPacket> packet)
{
	if (ECodecType::dstar == packet->GetCodecIn())
	{
		// codec_in is dstar, the audio has just completed, so now calc the M17 and DMR
		codec2_queue.push(packet);
		imbe_queue.push(packet);
		dmrsf_device->AddPacket(packet);
	}
	else
	{
		if (packet->AllCodecsAreSet()) 
			UpdateStream(packet);
	}
}

void CTranscoder::RouteDmrPacket(std::shared_ptr<CTranscoderPacket> packet)
{
	if (ECodecType::dmr == packet->GetCodecIn())
	{
		codec2_queue.push(packet);
		imbe_queue.push(packet);
		dstar_device->AddPacket(packet);
	}
	else
	{
		if (packet->AllCodecsAreSet()) 
			UpdateStream(packet);
	}
}

void CTranscoder::Dump(const std::shared_ptr<CTranscoderPacket> p, const std::string &title) const
{
	std::stringstream line;
	line << title << " Mod='" << p->GetModule() << "' SID=" << std::showbase << std::hex << ntohs(p->GetStreamId()) << std::noshowbase << " ET:" << std::setprecision(3) << p->GetTimeMS();

	ECodecType in = p->GetCodecIn();
	if (p->DStarIsSet())
		line << " DStar";
	if (ECodecType::dstar == in)
		line << '*';
	if (p->DMRIsSet())
		line << " DMR";
	if (ECodecType::dmr == in)
		line << '*';
	if (p->M17IsSet())
		line << " M17";
	if (ECodecType::c2_1600 == in)
		line << "**";
	else if (ECodecType::c2_3200 == in)
		line << '*';
	if (p->IsSecond())
		line << " IsSecond";
	if (p->IsLast())
		line << " IsLast";

	std::cout << line.str() << std::dec << std::endl;
}
