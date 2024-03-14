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

#pragma once

#include <atomic>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <future>
#include <mutex>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <ctime>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <arpa/inet.h>

#include "TTypes.h"

////////////////////////////////////////////////////////////////////////////////////////
// defines

//// Module configuration
#define DSTAR_IPV4 true
#define DSD_IPV4 true
#define DMR_IPV4 true
#define YSF_IPV4 true
#define M17_IPV4 true
#define P25_IPV4 true
#define URF_IPV4 true


#define DSTAR_IPV6 true	// QnetGateway can use IPv6
#define DSD_IPV6 false
#define DMR_IPV6 false
#define YSF_IPV6 false
#define XLX_IPV6 false
#define M17_IPV6 true
#define P25_IPV6 false
#define URF_IPV6 true

// protocols ---------------------------------------------------

// SIZE always has to be the last element!
enum class EProtocol : unsigned { dcs, dextra, dplus, dsd, m17, mmdvm, p25, urf, ysf, SIZE };

// DCS
#define DCS_KEEPALIVE_PERIOD            1                                   // in seconds
#define DCS_KEEPALIVE_TIMEOUT           (DCS_KEEPALIVE_PERIOD*30)           // in seconds

// DExtra
#define DEXTRA_KEEPALIVE_PERIOD         3                                   // in seconds
#define DEXTRA_KEEPALIVE_TIMEOUT        (DEXTRA_KEEPALIVE_PERIOD*10)        // in seconds

// DPlus
#define DPLUS_KEEPALIVE_PERIOD          1                                   // in seconds
#define DPLUS_KEEPALIVE_TIMEOUT         (DPLUS_KEEPALIVE_PERIOD*10)         // in seconds

// DStarDirect
#define DSD_KEEPALIVE_PERIOD            2                                   // seconds
#define DSD_KEEPALIVE_TIMEOUT			20                                  // seconds

// DMRMmdvm
#define DMRMMDVM_KEEPALIVE_PERIOD       10                                  // in seconds
#define DMRMMDVM_KEEPALIVE_TIMEOUT      (DMRMMDVM_KEEPALIVE_PERIOD*10)      // in seconds
#define DMRMMDVM_REFLECTOR_SLOT         DMR_SLOT2
#define DMRMMDVM_REFLECTOR_COLOUR       1

// M17
#define M17_KEEPALIVE_PERIOD			3
#define M17_KEEPALIVE_TIMEOUT           (M17_KEEPALIVE_PERIOD*10)

// P25
#define P25_KEEPALIVE_PERIOD            3                                   // in seconds
#define P25_KEEPALIVE_TIMEOUT           (P25_KEEPALIVE_PERIOD*10)         // in seconds

// URF
#define URF_KEEPALIVE_PERIOD            1                                   // in seconds
#define URF_KEEPALIVE_TIMEOUT           (URF_KEEPALIVE_PERIOD*30)           // in seconds
#define URF_RECONNECT_PERIOD            5                                   // in seconds

// YSF
#define YSF_KEEPALIVE_PERIOD            3                                   // in seconds
#define YSF_KEEPALIVE_TIMEOUT           (YSF_KEEPALIVE_PERIOD*10)           // in seconds

////////////////////////////////////////////////////////////////////////////////////////
// macros

#define MIN(a,b) 				((a)<(b))?(a):(b)
#define MAKEWORD(low, high)		((uint16_t)(((uint8_t)(low)) | (((uint16_t)((uint8_t)(high))) << 8)))
#define MAKEDWORD(low, high)	((uint32_t)(((uint16_t)(low)) | (((uint32_t)((uint16_t)(high))) << 16)))
#define LOBYTE(w)				((uint8_t)(uint16_t)(w & 0x00FF))
#define HIBYTE(w)				((uint8_t)((((uint16_t)(w)) >> 8) & 0xFF))
#define LOWORD(dw)				((uint16_t)(uint32_t)(dw & 0x0000FFFF))
#define HIWORD(dw)				((uint16_t)((((uint32_t)(dw)) >> 16) & 0xFFFF))
