//
//  ccallsignlist.cpp
//  m17ref
//
//  Created by Jean-Luc Deltombe (LX3JL) on 30/12/2015.
//  Copyright © 2015 Jean-Luc Deltombe (LX3JL). All rights reserved.
//  Copyright © 2020 Thomas A. Early, N7TAE
//
// ----------------------------------------------------------------------------
//    This file is part of m17ref.
//
//    m17ref is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    m17ref is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    with this software.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "BlackWhiteSet.h"

////////////////////////////////////////////////////////////////////////////////////////
// file io

bool CBlackWhiteSet::LoadFromFile(const std::string &filename)
{
	bool ok = false;
	char sz[256];

	// and load
	std::ifstream file(filename);
	if ( file.is_open() )
	{
		Lock();

		// empty list
		m_Callsigns.clear();
		// fill with file content
		while ( file.getline(sz, sizeof(sz)).good()  )
		{
			// remove leading & trailing spaces
			char *szt = TrimWhiteSpaces(sz);

			// crack it
			if ( (strlen(szt) > 0) && (szt[0] != '#') )
			{
				// 1st token is callsign
				if ( (szt = strtok(szt, " ,\t")) != nullptr )
				{
					std::string cs(ToUpper(szt));
					if (m_Callsigns.end() == m_Callsigns.find(cs))
					{
						m_Callsigns.insert(cs);
					}
					else
					{
						std::cerr << "Duplicate ," << cs << " in " << filename << " will be ignored." << std::endl;
					}
				}
			}
		}
		// close file
		file.close();

		// keep file path
		m_Filename = filename;

		// update time
		GetLastModTime(&m_LastModTime);

		// and done
		Unlock();
		ok = true;
		std::cout << "Gatekeeper loaded " << m_Callsigns.size() << " lines from " << filename <<  std::endl;
	}
	else
	{
		std::cout << "Gatekeeper cannot find " << filename <<  std::endl;
	}

	return ok;
}

bool CBlackWhiteSet::ReloadFromFile(void)
{
	bool ok = false;

	if ( ! m_Filename.empty() )
	{
		ok = LoadFromFile(m_Filename);
	}
	return ok;
}

bool CBlackWhiteSet::NeedReload(void)
{
	bool needReload = false;

	time_t time;
	if ( GetLastModTime(&time) )
	{
		needReload = time != m_LastModTime;
	}
	return needReload;
}

////////////////////////////////////////////////////////////////////////////////////////
// compare

bool CBlackWhiteSet::IsMatched(const std::string &cs) const
{
	for ( const auto &item : m_Callsigns )
	{
		auto pos = item.find('*');
		switch (pos)
		{
			case 0:
				return true;
			case std::string::npos:
				if (0 == item.compare(cs))
					return true;
				break;
			default:
				if (0 == item.compare(0, pos, cs, 0, pos))
					return true;
				break;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////
// helpers

char *CBlackWhiteSet::TrimWhiteSpaces(char *str)
{
	char *end;

	// Trim leading space & tabs
	while((*str == ' ') || (*str == '\t')) str++;

	// All spaces?
	if(*str == 0)
		return str;

	// Trim trailing space, tab or lf
	end = str + ::strlen(str) - 1;
	while((end > str) && ((*end == ' ') || (*end == '\t') || (*end == '\r'))) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}

bool CBlackWhiteSet::GetLastModTime(time_t *time)
{
	bool ok = false;

	if ( !m_Filename.empty() )
	{
		struct stat fileStat;
		if( ::stat(m_Filename.c_str(), &fileStat) != -1 )
		{
			*time = fileStat.st_mtime;
			ok = true;
		}
	}
	return ok;
}

char *CBlackWhiteSet::ToUpper(char *str)
{
	constexpr auto diff = 'a' - 'A';
	for (char *p=str; *p; p++)
	{
		if (*p >= 'a' && *p <= 'z')
			*p -= diff;
	}
	return str;
}
