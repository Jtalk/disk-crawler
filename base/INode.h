/*
    Disk Crawler Library.
    Copyright (C) 2013  Jtalk <me@jtalk.me>

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
	
	enum AllocType {
		ALLOC_EMPTY,
		ALLOC_FULL,
	};
	
	uint64_t file_size;
	uint32_t *blocks;
	
	/*
	 * AllocType is kinda workarond disabling auto heap buffer allocation (which is slow)
	 * in case of move construction preparations while empty object is created before move
	 * assignment operator is called. Compiler will default-construct an object which
	 * will not allocate any heap space.
	 */
	INode(AllocType type = ALLOC_EMPTY);
	INode(INode &&other);
	~INode();
	
private:
	INode(const INode &other) = delete;
	INode &operator=(const INode &other) = delete;
};