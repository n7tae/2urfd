/*
 *   Copyright (C) 2016,2017 by Jonathan Naylor G4KLX
 *   Copyright (C) 2024 Thomas A. Early
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

#include <string>
#include <memory>
#include <cstdint>

class CYSFPayload
{
public:
	CYSFPayload() : m_uplink(nullptr), m_downlink(nullptr), m_source(nullptr), m_dest(nullptr) {}
	~CYSFPayload();
	bool processHeaderData(uint8_t* bytes);

	void writeVDMode2Data(uint8_t* data, const uint8_t* dt);
	bool readVDMode1Data(const uint8_t* data, uint8_t* dt);
	bool readVDMode2Data(const uint8_t* data, uint8_t* dt);

	void writeHeader(uint8_t* data, const uint8_t* csd1, const uint8_t* csd2);

	void writeDataFRModeData1(const uint8_t* dt, uint8_t* data);
	void writeDataFRModeData2(const uint8_t* dt, uint8_t* data);
	bool readDataFRModeData1(const uint8_t* data, uint8_t* dt);
	bool readDataFRModeData2(const uint8_t* data, uint8_t* dt);

	std::string getSource();
	std::string getDest();

	void setUplink(const std::string& callsign);
	void setDownlink(const std::string& callsign);

	void reset();

private:
	uint8_t *m_uplink, *m_downlink, *m_source, *m_dest;
};
