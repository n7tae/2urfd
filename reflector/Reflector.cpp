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


#include <string.h>

#include "Global.h"

CReflector::CReflector()
{
#ifndef NO_DHT
	peers_put_count /* = clients_put_count = users_put_count */ = 0;
#endif
}

CReflector::~CReflector()
{
	keep_running = false;
	if ( m_XmlReportFuture.valid() )
	{
		m_XmlReportFuture.get();
	}

	for (auto it=m_Modules.cbegin(); it!=m_Modules.cend(); it++)
	{
		if (m_RouterFuture[*it].valid())
			m_RouterFuture[*it].get();
	}
	m_RouterFuture.clear();
	m_Stream.clear();
}


////////////////////////////////////////////////////////////////////////////////////////
// operation

bool CReflector::Start(void)
{
	// get config stuff
	const auto cs(g_Configure.GetString(g_Keys.names.callsign));
	m_Callsign.SetCallsign(cs, false);
	m_Modules.assign(g_Configure.GetString(g_Keys.modules.modules));
	std::string tcmods(g_Configure.GetString(g_Keys.tc.tcmodules));

#ifndef NO_DHT
	// start the dht instance
	refhash = dht::InfoHash::get(cs);
	node.run(17171, dht::crypto::generateIdentity(cs), true, 59973);
	std::ifstream myfile;
	const auto path(g_Configure.Contains(g_Keys.files.dht) ? g_Configure.GetString(g_Keys.files.dht) : "");
	if (path.size() > 0)
		myfile.open(path, std::ios::binary|std::ios::ate);
	if (myfile.is_open())
	{
		msgpack::unpacker pac;
		auto size = myfile.tellg();
		myfile.seekg (0, std::ios::beg);
		pac.reserve_buffer(size);
		myfile.read (pac.buffer(), size);
		pac.buffer_consumed(size);
		// Import nodes
		msgpack::object_handle oh;
		while (pac.next(oh)) {
			auto imported_nodes = oh.get().as<std::vector<dht::NodeExport>>();
			std::cout << "Importing " << imported_nodes.size() << " ham-dht nodes from " << path << std::endl;
			node.bootstrap(imported_nodes);
		}
		myfile.close();
	}
	else
	{
		const auto bsnode = g_Configure.GetString(g_Keys.names.bootstrap);
		if (bsnode.size())
		{
			std::cout << "Bootstrapping from " << bsnode << std::endl;
			node.bootstrap(bsnode, "17171");
		}
		else
		{
			std::cout << "WARNING: The DHT bootstrap node has not been defined!" << std::endl;
		}
	}
#endif

	// let's go!
	keep_running = true;

	// init gate keeper. It can only return true!
	g_GateKeeper.Init();

	// init dmrid directory. No need to check the return value.
	g_LDid.LookupInit();

	// init dmrid directory. No need to check the return value.
	g_LNid.LookupInit();

	// init wiresx node directory. Likewise with the return vale.
	g_LYtr.LookupInit();

	// create protocols
	if (! m_Protocols.Init())
	{
		m_Protocols.Close();
		return true;
	}

	// start one thread per reflector module
	for (auto c : m_Modules)
	{
		auto stream = std::make_shared<CPacketStream>(c, (tcmods.npos == tcmods.find(c)) ? false : true);
		if (stream)
		{
			m_Stream[c] = stream;
		}
		else
		{
			std::cerr << "Could not make a CPacketStream for module '" << c << "'" << std::endl;
			return true;
		}
		try
		{
			m_RouterFuture[c] = std::async(std::launch::async, &CReflector::RouterThread, this, c);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Cannot start module '" << c << "' thread: " << e.what() << '\n';
			keep_running = false;
			return true;
		}
	}

	// start the reporting thread
	try
	{
		m_XmlReportFuture = std::async(std::launch::async, &CReflector::StateReportThread, this);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Cannot start the dashboard data report thread: " << e.what() << '\n';
	}

#ifndef NO_DHT
	PutDHTConfig();
#endif

	return false;
}

void CReflector::Stop(void)
{
	// stop & delete all threads
	keep_running = false;

	// stop & delete report threads
	if ( m_XmlReportFuture.valid() )
	{
		m_XmlReportFuture.get();
	}

	// stop & delete all router thread
	for (auto c : m_Modules)
	{
		if (m_RouterFuture[c].valid())
			m_RouterFuture[c].get();
	}

	// close protocols
	m_Protocols.Close();

	// close gatekeeper
	g_GateKeeper.Close();

	// close databases
	g_LDid.LookupClose();
	g_LNid.LookupClose();
	g_LYtr.LookupClose();

#ifndef NO_DHT
	// save the state of the DHT network
	const std::string path(g_Configure.Contains(g_Keys.files.dht) ? g_Configure.GetString(g_Keys.files.dht) : "");
	if (path.size() > 0)
	{
		auto exnodes = node.exportNodes();
		if (exnodes.size())
		{
			// Export nodes to binary file
			std::ofstream myfile(path, std::ios::binary | std::ios::trunc);
			if (myfile.is_open())
			{
				std::cout << "Saving " << exnodes.size() << " nodes to " << path << std::endl;
				msgpack::pack(myfile, exnodes);
				myfile.close();
			}
			else
				std::cerr << "Trouble opening " << path << std::endl;
		}
		else
			std::cout << "There are no DHT network nodes to save!" << std::endl;
	}

	// kill the DHT
	node.cancelPut(refhash, toUType(EUrfdValueID::Config));
	node.cancelPut(refhash, toUType(EUrfdValueID::Peers));
	node.cancelPut(refhash, toUType(EUrfdValueID::Clients));
	node.cancelPut(refhash, toUType(EUrfdValueID::Users));
	node.shutdown({}, true);
	node.join();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
// stream opening & closing

// clients MUST have bee locked by the caller so we can freely access it within the function
std::shared_ptr<CPacketStream> CReflector::OpenStream(std::unique_ptr<CDvHeaderPacket> &DvHeader, std::shared_ptr<CClient>client)
{
	// check sid is not zero
	if ( 0U == DvHeader->GetStreamId() )
	{
		std::cerr << "StreamId for client " << client->GetCallsign() << " is zero!" << std::endl;
		return nullptr;
	}

	// check if client is valid candidate
	if ( ! m_Clients.IsClient(client) || client->IsAMaster() )
	{
		return nullptr;
	}

	// check if no stream with same streamid already open
	// to prevent loops
	if ( IsStreamOpen(DvHeader) )
	{
		std::cerr << "Detected stream loop on module " << DvHeader->GetRpt2Module() << " for client " << client->GetCallsign() << " with sid " << DvHeader->GetStreamId() << std::endl;
		return nullptr;
	}

	// set the packet module
	DvHeader->SetPacketModule(client->GetReflectorModule());
	// get the module's queue
	char module = DvHeader->GetRpt2Module();
	auto stream = GetStream(module);
	if ( stream == nullptr )
	{
		std::cerr << "Can't find module '" << module << "' for Client " << client->GetCallsign() << std::endl;
		return nullptr;
	}

	// is it available ?
	if ( stream->OpenPacketStream(*DvHeader, client) )
	{
		// stream open, mark client as master
		// so that it can't be deleted
		client->SetMasterOfModule(module);

		// update last heard time
		client->Heard();

		// report
		std::cout << std::showbase << std::hex;
		std::cout << "Opening stream on module " << module << " for client " << client->GetCallsign() << " with sid " << ntohs(DvHeader->GetStreamId()) << " by user " << DvHeader->GetMyCallsign() << std::endl;
		std::cout << std::noshowbase << std::dec;

		// and push header packet
		stream->Push(std::move(DvHeader));

		// notify
		//OnStreamOpen(stream->GetUserCallsign());

	}
	return stream;
}

void CReflector::CloseStream(std::shared_ptr<CPacketStream> stream)
{
	if ( stream != nullptr )
	{
		// wait queue is empty. this waits forever
		bool bEmpty = stream->IsEmpty();
		while (! bEmpty)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			bEmpty = stream->IsEmpty();
		}

		GetClients();	// lock clients

		// get and check the master
		std::shared_ptr<CClient>client = stream->GetOwnerClient();
		if ( client != nullptr )
		{
			// client no longer a master
			client->NotAMaster();

			// notify
			//OnStreamClose(stream->GetUserCallsign());

			std::cout << "Closing stream of module " << GetStreamModule(stream) << std::endl;
		}

		// release clients
		ReleaseClients();

		// and stop the queue
		stream->ClosePacketStream();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// router threads

void CReflector::RouterThread(const char ThisModule)
{
	auto pitem = m_Stream.find(ThisModule);
	if (m_Stream.end() == pitem)
	{
		std::cerr << "Module '" << ThisModule << " CPacketStream doesn't exist! aborting RouterThread()" << std::endl;
		return;
	}
	const auto streamIn = pitem->second;
	while (keep_running)
	{
		// wait until something shows up
		auto packet = streamIn->PopWait();

		if (packet->GetPacketModule() != ThisModule)
		{
			// if we're here, something is wrong internally
			// For header packets, the module is set in CReflector::OpenStream, and
			// for Frame packets, the module is set in CProtocol::OnDvFramePacketIn
			std::cerr << "RouterThread[" << ThisModule << "]: " << (packet->IsDvHeader()?"Header":"Frame") << "Packet has wrong module: '" << packet->GetPacketModule() << "'. Reset!" << std::endl;
			packet->SetPacketModule(ThisModule);
		}

		// iterate on all protocols
		m_Protocols.Lock();
		for ( auto it=m_Protocols.begin(); it!=m_Protocols.end(); it++ )
		{
			auto copy = packet->Copy();

			// if packet is header, update RPT2 according to protocol
			if ( copy->IsDvHeader() )
			{
				// make the protocol-patched reflector callsign
				CCallsign csRPT = (*it)->GetReflectorCallsign();
				csRPT.SetCSModule(ThisModule);
				// and put it in the copy
				(static_cast<CDvHeaderPacket *>(copy.get()))->SetRpt2Callsign(csRPT);
			}

			(*it)->Push(std::move(copy));
		}
		m_Protocols.Unlock();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// report threads

#define XML_UPDATE_PERIOD 10

void CReflector::StateReportThread()
{
	std::string xmlpath, jsonpath;
#ifndef NO_DHT
	peers_changed = /*clients_changed = users_changed =*/ true;
#endif
	if (g_Configure.Contains(g_Keys.files.xml))
		xmlpath.assign(g_Configure.GetString(g_Keys.files.xml));
	if (g_Configure.Contains(g_Keys.files.json))
		jsonpath.assign(g_Configure.GetString(g_Keys.files.json));

	if (xmlpath.empty() && jsonpath.empty())
		return;	// nothing to do

	while (keep_running)
	{
		// report to xml file
		if (! xmlpath.empty())
		{
			std::ofstream xmlFile;
			xmlFile.open(xmlpath, std::ios::out | std::ios::trunc);
			if ( xmlFile.is_open() )
			{
				// write xml file
				WriteXmlFile(xmlFile);

				// and close file
				xmlFile.close();
			}
			else
			{
				std::cout << "Failed to open " << xmlpath  << std::endl;
			}
		}

		// json report
		if (!  jsonpath.empty())
		{
			nlohmann::json jreport;
			JsonReport(jreport);
			std::ofstream jsonFile;
			jsonFile.open(jsonpath, std::ios::out | std::ios::trunc);
			if (jsonFile.is_open())
			{
				jsonFile << jreport.dump();
				jsonFile.close();
			}
		}

		// and wait a bit and do something useful at the same time
		for (int i=0; i< XML_UPDATE_PERIOD && keep_running; i++)
		{
#ifndef NO_DHT
			// update the dht data, if needed
			if (peers_changed)
			{
				PutDHTPeers();
				peers_changed = false;
			}
			// if (clients_changed)
			// {
			// 	PutDHTClients();
			// 	clients_changed = false;
			// }
			// if (users_changed)
			// {
			// 	PutDHTUsers();
			// 	users_changed = false;
			// }
#endif
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// notifications

void CReflector::OnPeersChanged(void)
{
#ifndef NO_DHT
	peers_changed = true;
#endif
}

// void CReflector::OnClientsChanged(void)
// {
// #ifndef NO_DHT
// 	clients_changed = true;
// #endif
// }

// void CReflector::OnUsersChanged(void)
// {
// #ifndef NO_DHT
// 	users_changed = true;
// #endif
// }

////////////////////////////////////////////////////////////////////////////////////////
// modules & queues

std::shared_ptr<CPacketStream> CReflector::GetStream(char module)
{
	auto it=m_Stream.find(module);
	if (it!=m_Stream.end())
		return it->second;

	return nullptr;
}

bool CReflector::IsStreamOpen(const std::unique_ptr<CDvHeaderPacket> &DvHeader)
{
	for (auto it=m_Stream.begin(); it!=m_Stream.end(); it++)
	{
		if ( (it->second->GetStreamId() == DvHeader->GetStreamId()) && (it->second->IsOpen()) )
			return true;
	}
	return false;
}

char CReflector::GetStreamModule(std::shared_ptr<CPacketStream> stream)
{
	for (auto it=m_Stream.begin(); it!=m_Stream.end(); it++)
	{
		if ( it->second == stream )
			return it->first;
	}
	return ' ';
}

////////////////////////////////////////////////////////////////////////////////////////
// report helpers

void CReflector::JsonReport(nlohmann::json &report)
{
	for (auto &item : g_Configure.GetData().items())
	{
		if (isupper(item.key().at(0)))
			report["Configure"][item.key()] = item.value();
	}

	report["Peers"] = nlohmann::json::array();
	auto peers = GetPeers();
	for (auto pit=peers->cbegin(); pit!=peers->cend(); pit++)
		(*pit)->JsonReport(report);
	ReleasePeers();

	report["Clients"] = nlohmann::json::array();
	auto clients = GetClients();
	for (auto cit=clients->cbegin(); cit!=clients->cend(); cit++)
		(*cit)->JsonReport(report);
	ReleaseClients();

	report["Users"] = nlohmann::json::array();
	auto users = GetUsers();
	for (auto uid=users->begin(); uid!=users->end(); uid++)
		(*uid).JsonReport(report);
	ReleaseUsers();
}

void CReflector::WriteXmlFile(std::ofstream &xmlFile)
{
	// write header
	xmlFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;

	// software version
	xmlFile << "<Version>" << g_Version << "</Version>" << std::endl;

	CCallsign cs = m_Callsign;
	cs.PatchCallsign(0, "XLX", 3);

	// linked peers
	xmlFile << "<" << cs << "linked peers>" << std::endl;
	// lock
	CPeers *peers = GetPeers();
	// iterate on peers
	for ( auto pit=peers->cbegin(); pit!=peers->cend(); pit++ )
	{
		(*pit)->WriteXml(xmlFile);
	}
	// unlock
	ReleasePeers();
	xmlFile << "</" << cs << "linked peers>" << std::endl;

	// linked nodes
	xmlFile << "<" << cs << "linked nodes>" << std::endl;
	// lock
	CClients *clients = GetClients();
	// iterate on clients
	for ( auto cit=clients->cbegin(); cit!=clients->cend(); cit++ )
	{
		if ( (*cit)->IsNode() )
		{
			(*cit)->WriteXml(xmlFile);
		}
	}
	// unlock
	ReleaseClients();
	xmlFile << "</" << cs << "linked nodes>" << std::endl;

	// last heard users
	xmlFile << "<" << cs << "heard users>" << std::endl;
	// lock
	CUsers *users = GetUsers();
	// iterate on users
	for ( auto it=users->begin(); it!=users->end(); it++ )
	{
		it->WriteXml(xmlFile);
	}
	// unlock
	ReleaseUsers();
	xmlFile << "</" << cs << "heard users>" << std::endl;
}

#ifndef NO_DHT
// DHT put() and get()
void CReflector::PutDHTPeers()
{
	const std::string cs(g_Configure.GetString(g_Keys.names.callsign));
	// load it up
	SUrfdPeers1 p;
	time(&p.timestamp);
	p.sequence = peers_put_count++;
	auto peers = GetPeers();
	for (auto pit=peers->cbegin(); pit!=peers->cend(); pit++)
	{
		p.list.emplace_back((*pit)->GetCallsign().GetCS(), (*pit)->GetReflectorModules(), (*pit)->GetConnectTime());
	}
	ReleasePeers();

	auto nv = std::make_shared<dht::Value>(p);
	nv->user_type.assign(URFD_PEERS_1);
	nv->id = toUType(EUrfdValueID::Peers);

	node.putSigned(
		refhash,
		nv,
#ifdef DEBUG
		[](bool success){ std::cout << "PutDHTPeers() " << (success ? "successful" : "unsuccessful") << std::endl; },
#else
		[](bool success){ if (! success) std::cout << "PutDHTPeers() unsuccessful" << std::endl; },
#endif
		true	// permanent!
	);
}

// void CReflector::PutDHTClients()
// {
// 	const std::string cs(g_Configure.GetString(g_Keys.names.callsign));
// 	SUrfdClients1 c;
// 	time(&c.timestamp);
// 	c.sequence = clients_put_count++;
// 	auto clients = GetClients();
// 	for (auto cit=clients->cbegin(); cit!=clients->cend(); cit++)
// 	{
// 		c.list.emplace_back((*cit)->GetCallsign().GetCS(), std::string((*cit)->GetIp().GetAddress()), (*cit)->GetReflectorModule(), (*cit)->GetConnectTime(), (*cit)->GetLastHeardTime());
// 	}
// 	ReleaseClients();

// 	auto nv = std::make_shared<dht::Value>(c);
// 	nv->user_type.assign(URFD_CLIENTS_1);
// 	nv->id = toUType(EUrfdValueID::Clients);

// 	node.putSigned(
// 		refhash,
// 		nv,
// #ifdef DEBUG
// 		[](bool success){ std::cout << "PutDHTClients() " << (success ? "successful" : "unsuccessful") << std::endl; },
// #else
// 		[](bool success){ if (! success) std::cout << "PutDHTClients() unsuccessful" << std::endl; },
// #endif
// 		false	// not permanent!
// 	);
// }

// void CReflector::PutDHTUsers()
// {
// 	const std::string cs(g_Configure.GetString(g_Keys.names.callsign));
// 	SUrfdUsers1 u;
// 	time(&u.timestamp);
// 	u.sequence = users_put_count++;
// 	auto users = GetUsers();
// 	for (auto uit=users->cbegin(); uit!=users->cend(); uit++)
// 	{
// 		u.list.emplace_back((*uit).GetCallsign(), std::string((*uit).GetViaNode()), (*uit).GetOnModule(), (*uit).GetViaPeer(), (*uit).GetLastHeardTime());
// 	}
// 	ReleaseUsers();

// 	auto nv = std::make_shared<dht::Value>(u);
// 	nv->user_type.assign(URFD_USERS_1);
// 	nv->id = toUType(EUrfdValueID::Users);

// 	node.putSigned(
// 		refhash,
// 		nv,
// #ifdef DEBUG
// 		[](bool success){ std::cout << "PutDHTUsers() " << (success ? "successful" : "unsuccessful") << std::endl; },
// #else
// 		[](bool success){ if (! success) std::cout << "PutDHTUsers() unsuccessful" << std::endl; },
// #endif
// 		false	// not permanent
// 	);
// }

void CReflector::PutDHTConfig()
{
	const std::string cs(g_Configure.GetString(g_Keys.names.callsign));
	SUrfdConfig1 cfg;
	time(&cfg.timestamp);
	cfg.callsign.assign(cs);
	cfg.ipv4addr.assign(g_Configure.GetString(g_Keys.ip.ipv4address));
	cfg.ipv6addr.assign(g_Configure.GetString(g_Keys.ip.ipv6address));
	cfg.modules.assign(g_Configure.GetString(g_Keys.modules.modules));
	cfg.transcodedmods.assign(g_Configure.GetString(g_Keys.tc.tcmodules));
	cfg.url.assign(g_Configure.GetString(g_Keys.names.url));
	cfg.email.assign(g_Configure.GetString(g_Keys.names.email));
	cfg.country.assign(g_Configure.GetString(g_Keys.names.country));
	cfg.sponsor.assign(g_Configure.GetString(g_Keys.names.sponsor));
	std::ostringstream ss;
	ss << g_Version;
	cfg.version.assign(ss.str());
	cfg.almod[toUType(EUrfdAlMod::nxdn)]   = g_Configure.GetAutolinkModule(g_Keys.nxdn.autolinkmod);
	cfg.almod[toUType(EUrfdAlMod::p25)]    = g_Configure.GetAutolinkModule(g_Keys.p25.autolinkmod);
	cfg.almod[toUType(EUrfdAlMod::ysf)]    = g_Configure.GetAutolinkModule(g_Keys.ysf.autolinkmod);
	cfg.ysffreq[toUType(EUrfdTxRx::rx)]    = g_Configure.GetUnsigned(g_Keys.ysf.defaultrxfreq);
	cfg.ysffreq[toUType(EUrfdTxRx::tx)]    = g_Configure.GetUnsigned(g_Keys.ysf.defaulttxfreq);
	cfg.refid[toUType(EUrfdRefId::nxdn)]   = g_Configure.GetUnsigned(g_Keys.nxdn.reflectorid);
	cfg.refid[toUType(EUrfdRefId::p25)]    = g_Configure.GetUnsigned(g_Keys.p25.reflectorid);
	cfg.port[toUType(EUrfdPorts::dcs)]     = (uint16_t)g_Configure.GetUnsigned(g_Keys.dcs.port);
	cfg.port[toUType(EUrfdPorts::dextra)]  = (uint16_t)g_Configure.GetUnsigned(g_Keys.dextra.port);
	cfg.port[toUType(EUrfdPorts::dmrplus)] = 0;
	cfg.port[toUType(EUrfdPorts::dplus)]   = (uint16_t)g_Configure.GetUnsigned(g_Keys.dplus.port);
	cfg.port[toUType(EUrfdPorts::m17)]     = (uint16_t)g_Configure.GetUnsigned(g_Keys.m17.port);
	cfg.port[toUType(EUrfdPorts::mmdvm)]   = (uint16_t)g_Configure.GetUnsigned(g_Keys.mmdvm.port);
	cfg.port[toUType(EUrfdPorts::nxdn)]    = (uint16_t)g_Configure.GetUnsigned(g_Keys.nxdn.port);
	cfg.port[toUType(EUrfdPorts::p25)]     = (uint16_t)g_Configure.GetUnsigned(g_Keys.p25.port);
	cfg.port[toUType(EUrfdPorts::urf)]     = (uint16_t)g_Configure.GetUnsigned(g_Keys.urf.port);
	cfg.port[toUType(EUrfdPorts::ysf)]     = (uint16_t)g_Configure.GetUnsigned(g_Keys.ysf.port);
	cfg.g3enabled = false;
	for (const auto m : cfg.modules)
		cfg.description[m] = g_Configure.GetString(g_Keys.modules.descriptor[m-'A']);

	auto nv = std::make_shared<dht::Value>(cfg);
	nv->user_type.assign(URFD_CONFIG_1);
	nv->id = toUType(EUrfdValueID::Config);

	node.putSigned(
		refhash,
		nv,
#ifdef DEBUG
		[](bool success){ std::cout << "PutDHTConfig() " << (success ? "successful" : "unsuccessful") << std::endl; },
#else
		[](bool success){ if(! success) std::cout << "PutDHTConfig() unsuccessful" << std::endl; },
#endif
		true
	);
}

void CReflector::GetDHTConfig(const std::string &cs)
{
	static SUrfdConfig1 cfg;
	cfg.timestamp = 0;	// every time this is called, zero the timestamp

	std::cout << "Getting " << cs << " connection info..." << std::endl;

	// we only want the configuration section of the reflector's document
	dht::Where w;
	w.id(toUType(EUrfdValueID::Config));

	node.get(
		dht::InfoHash::get(cs),
		[](const std::shared_ptr<dht::Value> &v) {
			if (0 == v->user_type.compare(URFD_CONFIG_1))
			{
				auto rdat = dht::Value::unpack<SUrfdConfig1>(*v);
				if (rdat.timestamp > cfg.timestamp)
				{
					// the time stamp is the newest so far, so put it in the static cfg struct
					cfg = dht::Value::unpack<SUrfdConfig1>(*v);
				}
			}
			else
			{
				std::cerr << "Get() returned unknown user_type: '" << v->user_type << "'" << std::endl;
			}
			return true;	// check all the values returned
		},
		[](bool success) {
			if (success)
			{
				if (cfg.timestamp)
				{
					// if the get() call was successful and there is a nonzero timestamp, then do the update
					g_GateKeeper.GetInterlinkMap()->Update(cfg.callsign, cfg.modules, cfg.ipv4addr, cfg.ipv6addr, cfg.port[toUType(EUrfdPorts::urf)], cfg.transcodedmods);
					g_GateKeeper.ReleaseInterlinkMap();
				}
				else
				{
					std::cerr << "node.Get() was successful, but the timestamp was zero" << std::endl;
				}
			}
			else
			{
				std::cout << "Get() was unsuccessful" << std::endl;
			}
		},
		{}, // empty filter
		w	// just the configuration section
	);
}

#endif
