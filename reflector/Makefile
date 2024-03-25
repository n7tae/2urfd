# Copyright (c) 2021 by Thomas A. Early N7TAE
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# locations for the executibles and other files are set here
# NOTE: IF YOU CHANGE THESE, YOU WILL NEED TO UPDATE THE service.* FILES AND
# if you change these locations, make sure the sgs.service file is updated!
# you will also break hard coded paths in the dashboard file, index.php.

include ../urfd.mk

EXE = $(REFLECTOR)

INICHECK = inicheck

DBUTIL = dbutil

ifeq ($(debug), true)
CFLAGS = -ggdb3 -DDEBUG -W -Werror -I../common -std=c++17 -MMD -MD
else
CFLAGS = -W -Werror -I../common -std=c++17 -MMD -MD
endif

LDFLAGS=-pthread -lcurl

ifeq ($(DHT), true)
LDFLAGS += -lopendht
else
CFLAGS += -DNO_DHT
endif

REFSRCS = BlackWhiteSet.cpp BPTC19696.cpp Buffer.cpp Callsign.cpp Client.cpp Clients.cpp CodecStream.cpp Configure.cpp CRC.cpp CurlGet.cpp DCSClient.cpp DCSProtocol.cpp DExtraClient.cpp DExtraProtocol.cpp DMRMMDVMClient.cpp DMRMMDVMProtocol.cpp DPlusClient.cpp DPlusProtocol.cpp DVFramePacket.cpp DVHeaderPacket.cpp GateKeeper.cpp Golay2087.cpp Golay24128.cpp Hamming.cpp InterlinkMap.cpp InterlinkMapItem.cpp IP.cpp Lookup.cpp LookupDmr.cpp LookupYsf.cpp M17Client.cpp M17CRC.cpp M17Packet.cpp M17Protocol.cpp Main.cpp P25Client.cpp P25Protocol.cpp Packet.cpp PacketStream.cpp Peer.cpp Peers.cpp Protocol.cpp Protocols.cpp QR1676.cpp Reflector.cpp RS129.cpp Semaphore.cpp UDPSocket.cpp URFClient.cpp URFPeer.cpp URFProtocol.cpp User.cpp Users.cpp Utils.cpp Version.cpp WiresXCmd.cpp WiresXCmdHandler.cpp WiresXInfo.cpp YSFClient.cpp YSFConvolution.cpp YSFFich.cpp YSFNode.cpp YSFPayload.cpp YSFProtocol.cpp YSFUtils.cpp

ifeq ($(DSD), false)
CFLAGS += -DNO_DSD
else
REFSRCS += DStarDirectClient.cpp DStarDirectProtocol.cpp
endif

SRCS = $(REFSRCS) $(wildcard ../common/*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
DBUTILOBJS = Configure.o CurlGet.o Lookup.o LookupDmr.o LookupYsf.o YSFNode.o Callsign.o

all : $(EXE) $(INICHECK) $(DBUTIL)

$(EXE) : $(OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(INICHECK) : Configure.cpp CurlGet.o
	$(CXX) -DINICHECK $(CFLAGS) $< CurlGet.o -o $@ -lcurl

$(DBUTIL) : Main.cpp $(DBUTILOBJS)
	$(CXX) -DUTILITY $(CFLAGS) $< $(DBUTILOBJS) -o $@ -pthread -lcurl

%.o : %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean :
	$(RM) *.o *.d ../common/*.o ../common/*.d $(EXE) $(INICHECK) $(DBUTIL)

-include $(DEPS)
