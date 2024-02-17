//  Copyright © 2019 Jean-Luc Deltombe (LX3JL). All rights reserved.
//  Copyright (C) 2016,2017 by Jonathan Naylor G4KLX
//  Copyright (C) 2018 by Andy Uribe CA6JAU
//  Copyright (C) 2018 by Manuel Sanchez EA7EE

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

#include <cstdint>

////////////////////////////////////////////////////////////////////////////////////////
// class

class CYsfUtils
{
public:
	// operation
	static void DecodeVD2Vchs(uint8_t *, uint8_t **);
	static void EncodeVD2Vch(uint8_t *, uint8_t *);
};
