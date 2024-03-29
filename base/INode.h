/*
    Disk Crawler library.
    Copyright (C) 2013-2014 Roman Nazarenko <me@jtalk.me>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <cinttypes>

struct INode {
	static const uint8_t FILE_BLOCKS_MAX = 15;
		
	uint64_t file_size;
	uint32_t *blocks;
	
	INode();
	INode(INode &&other);
	~INode();
	
	bool valid() const;
	
private:
	INode(const INode &other) = delete;
	INode &operator=(const INode &other) = delete;
};