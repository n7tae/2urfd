/*
 *   Copyright (c) 2023 by Thomas A. Early N7TAE
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

#include <string>

#pragma once

// configuration key names
struct SJsonKeys {
	struct PORTONLY { const std::string port; }
	dcs { "DCSPort" },
	dextra { "DExtraPort" },
	dplus { "DPlusPort" },
	m17 { "M17Port" },
	urf { "URFPort" };

	struct DSD { const std::string ircServer, ircLogin; }
	dsd { "ircServer", "ircLogin" };

	struct MMDVM { const std::string port, defaultid; }
	mmdvm { "MMDVMPort", "mmdvmdefaultid" };

	struct NAMES { const std::string callsign, bootstrap, url, email, country, sponsor; }
	names { "Callsign", "bootstrap", "DashboardUrl", "SysopEmail", "Country", "Sponsor" };

	struct IP { const std::string ipv4bind, ipv4address, ipv6bind, ipv6address, transcoder; }
	ip { "ipv4bind", "IPv4Address", "ipv6bind", "IPv6Address", "tcaddress" };

	struct MODULES { const std::string modules, tcmodules, descriptor[26]; }
	modules { "Modules", "TranscodedModules",
		"DescriptionA", "DescriptionB", "DescriptionC", "DescriptionD", "DescriptionE", "DescriptionF", "DescriptionG", "DescriptionH", "DescriptionI", "DescriptionJ", "DescriptionK", "DescriptionL", "DescriptionM", "DescriptionN", "DescriptionO", "DescriptionP", "DescriptionQ", "DescriptionR", "DescriptionS", "DescriptionT", "DescriptionU", "DescriptionV", "DescriptionW", "DescriptionX", "DescriptionY", "DescriptionZ" };

	struct P25 { const std::string port, autolinkmod, reflectorid; }
	p25 { "P25Port",  "P25AutolinkMod",   "P25ReflectorID" };

	struct YSF { const std::string port, autolinkmod, defaulttxfreq, defaultrxfreq;
		struct YSLREG { const std::string id, name, description; } ysfreflectordb; }
	ysf { "YSFPort", "YSFAutoLinkMod", "YSFDefaultTxFreq", "YSFDefaultRxFreq",
		{ "ysfrefdbid", "ysfrefdbname", "ysfrefdbdesc" } };

	struct DB { const std::string url, mode, refreshmin, filepath; }
	dmriddb   {  "dmrIdDbUrl",  "dmrIdDbMode",  "dmrIdDbRefresh",  "dmrIdDbFilePath" },
	ysftxrxdb {  "ysfIdDbUrl",  "ysfIdDbMode",  "ysfIdDbRefresh",  "ysfIdDbFilePath" };

	struct FILES { const std::string pid, xml, json, white, black, interlink, dhtsave; }
	files { "pidFilePath", "xmlFilePath", "jsonFilePath", "whitelistFilePath", "blacklistFilePath", "interlinkFilePath", "dhtSavePath" };
};
