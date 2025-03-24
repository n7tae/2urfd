#pragma once

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

#include <unordered_map>
#include <memory>
#include <atomic>
#include <future>
#include <mutex>
#include <list>
#include <utility>

#include "imbe_vocoder.h"
#include "codec2.h"
#include "DV3000.h"
#include "DV3003.h"

class CTranscoder
{
public:
	std::mutex dstar_mux, dmrst_mux;

	CTranscoder();
	bool Start();
	void Stop();
	void Transcode(std::shared_ptr<CTranscoderPacket> packet);
	void RouteDstPacket(std::shared_ptr<CTranscoderPacket> packet);
	void RouteDmrPacket(std::shared_ptr<CTranscoderPacket> packet);
	void Dump(const std::shared_ptr<CTranscoderPacket> packet, const std::string &title) const;

private:
	std::atomic<bool> keep_running;
	std::future<void> reflectorFuture, c2Future, imbeFuture;
	std::unordered_map<char, int16_t[160]> audio_store;
	std::unordered_map<char, uint8_t[8]> data_store;
	std::unordered_map<char, std::unique_ptr<CPacketQueue>> inQmap;
	std::unordered_map<char, std::unique_ptr<CCodec2>> c2_16, c2_32;
	std::unique_ptr<CDVDevice> dstar_device, dmrsf_device;

	CPacketQueue codec2_queue;
	CPacketQueue imbe_queue;
	int32_t ambe_in_num, ambe_out_num;
	std::unordered_map<char, std::unique_ptr<imbe_vocoder_impl>> p25vocoder;

	int32_t calcNumerator(int32_t db) const;
	bool DiscoverFtdiDevices(std::list<std::pair<std::string, std::string>> &found);
	bool InitVocoders();

	// processing threads
	void ProcessC2Thread();
	void ProcessIMBEThread();

	void Codec2toAudio(std::shared_ptr<CTranscoderPacket> packet);
	void AudiotoCodec2(std::shared_ptr<CTranscoderPacket> packet);
	void IMBEtoAudio(std::shared_ptr<CTranscoderPacket> packet);
	void AudiotoIMBE(std::shared_ptr<CTranscoderPacket> packet);
	void ReadReflectorThread();
	void UpdateStream(std::shared_ptr<CTranscoderPacket> packet);

	std::mutex m_tcmux;	// for the Transcode input

	std::string m_tcmods;
};
