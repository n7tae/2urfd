//  Copyright © 2022 Thomas A. Early, N7TAE
//
// ----------------------------------------------------------------------------
//    This is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    with this software.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#pragma once

#include <opendht.h>

#include "TTypes.h"

#define URFD_PEERS_1   "urfd-peers-1"
#define URFD_USERS_1   "urfd-users-1"
#define URFD_CONFIG_2  "urfd-config-2"
#define URFD_CLIENTS_2 "urfd-clients-2"

enum class EUrfdValueID : uint64_t { Config=1, Peers=2, Clients=3, Users=4 };

using UrfdPeerTuple = std::tuple<std::string, std::string, std::time_t>;
enum class EUrfdPeerFields { Callsign, Modules, ConnectTime };
struct SUrfdPeers1
{
	std::time_t timestamp;
	unsigned int sequence;
	std::list<UrfdPeerTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

using UrfdClientTuple2 = std::tuple<std::string, std::string, std::string, char, std::time_t, std::time_t>;
enum class EUrfdClientFields2 { Callsign, Protocol, Ip, Module, ConnectTime, LastHeardTime };
struct SUrfdClients2
{
	std::time_t timestamp;
	unsigned int sequence;
	std::list<UrfdClientTuple2> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

using UrfdUserTuple = std::tuple<std::string, std::string, char, std::string, std::time_t>;
enum class EUrfdUserFields { Callsign, ViaNode, OnModule, ViaPeer, LastHeardTime };
struct SUrfdUsers1
{
	std::time_t timestamp;
	unsigned int sequence;
	std::list<UrfdUserTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

// 'SIZE' has to be last value for these scoped enums
enum class EUrfdPorts2 : unsigned { dcs, dextra, dmrplus, dplus, dsd, m17, mmdvm, nxdn, p25, urf, ysf, SIZE };
enum class EUrfdAlMod  : unsigned { nxdn, p25, ysf, SIZE };
enum class EUrfdTxRx   : unsigned { rx, tx, SIZE };
enum class EUrfdRefId  : unsigned { nxdn, p25, SIZE };
struct SUrfdConfig2
{
	std::time_t timestamp;
	std::string callsign, ipv4addr, ipv6addr, modules, transcodedmods, url, email, sponsor, country, version;
	std::array<uint16_t, toUType(EUrfdPorts2::SIZE)> port;
	std::array<char, toUType(EUrfdAlMod::SIZE)> almod;
	std::array<unsigned long, toUType(EUrfdTxRx::SIZE)> ysffreq;
	std::array<unsigned, toUType(EUrfdRefId::SIZE)> refid;
	std::unordered_map<char, std::string> description;

	MSGPACK_DEFINE(timestamp, callsign, ipv4addr, ipv6addr, modules, transcodedmods, url, email, sponsor, country, version, almod, ysffreq, refid, port, description)
};
