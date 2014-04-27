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


#include "OverlapBuffer.h"

#include "utility.h"

OverlapBuffer::OverlapBuffer(size_t size):
	Buffer(size), offset(0)
{}

OverlapBuffer::~OverlapBuffer()
{}

void OverlapBuffer::capture(const Buffer &other, size_t from_start_offset, size_t buffer_start_offset)
{
	DEBUG_ASSERT(other.size() >= from_start_offset, "Offset from captured buffer start is %u while size itself is %u", from_start_offset, other.size());
	
	this->reset(other.size() - from_start_offset);
	this->capture(other.cbegin() + from_start_offset, other.size() - from_start_offset);
	this->offset = buffer_start_offset;
}

size_t OverlapBuffer::extract(Buffer &target, size_t size, size_t stream_offset)
{
	if (not this->suitable(stream_offset)) {
		return 0;
	}
	
	this->move_front(stream_offset - this->offset);
	this->offset = stream_offset;
	
	auto extraction_size = std::min(size, this->size());
	target.capture(this->cbegin(), extraction_size);
	
	return extraction_size;
}
