/*
    Disk Crawler library.
    Copyright (C) 2013  Jtalk <me@jtalk.me>
 
    This program is free software: you can redistribute it and/or modify    
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public Licens
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#pragma once

#include "Buffer.h"

class OverlapBuffer : private Buffer
{
	size_t offset;
	
	using Buffer::capture;
	
public:
	OverlapBuffer(const OverlapBuffer&) = delete;

	explicit OverlapBuffer(size_t size);
	virtual ~OverlapBuffer();

	void capture(const Buffer &other, size_t from_start_offset, size_t stream_offset);
	size_t extract(Buffer &target, size_t size, size_t stream_offset);
	
	void clear() {
		this->Buffer::clear();
		this->offset = 0;
	}
	
	bool suitable(size_t stream_offset) const {
		return stream_offset < this->offset + this->size() and stream_offset >= this->offset;
	}
	
	using Buffer::size;
	using Buffer::empty;
};