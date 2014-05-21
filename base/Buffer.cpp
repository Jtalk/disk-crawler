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

#include "Buffer.h"

#include "Log.h"

void Buffer::reallocate(size_t new_length) 
{
	free(this->buffer);
	this->buffer = (uint8_t*)malloc(new_length);
	this->real_length = new_length;
	this->offset = 0;
}

void Buffer::advance(size_t new_length) 
{
	if (new_length > this->real_length) {
		this->buffer = (uint8_t*)realloc(this->buffer, new_length);
		this->real_length = new_length;
	}
}

Buffer::Buffer(size_t count) 
{
	this->buffer = (uint8_t*)malloc(count);
	this->real_length = count;
	this->length = 0;
	this->offset = 0;
}

Buffer::Buffer(Buffer && other): Buffer(1) {
	std::swap(this->buffer, other.buffer);
	std::swap(this->offset, other.offset);
	std::swap(this->real_length, other.real_length);
	std::swap(this->length, other.length);
}

Buffer::~Buffer() 
{
	free(this->buffer);
}

void Buffer::reset(size_t size) 
{
	this->length = 0;
	this->offset = 0;
	
	if (this->real_length < size) {
		this->reallocate(size);
	}
}
	
void Buffer::reset_offset()
{
	this->length += this->offset;
	this->offset = 0;
}

void Buffer::resize(size_t size) {
	this->advance(size);
	this->length = size;
}

void Buffer::capture(const uint8_t *read_buffer, size_t read) 
{
	if (this->offset != 0) {
		logger()->error("Capturing buffer with non-zero offset %u detected", this->offset);
	}
	this->advance(this->length + read);
	memcpy(this->buffer + this->length, read_buffer, read);
	this->length += read;
}

bool Buffer::move_front(size_t move_offset, size_t count) 
{
	if (move_offset + count > this->length) {
		return false;
	}
	
	this->offset += move_offset;
	this->length = count;
	return true;
}
bool Buffer::move_front(size_t move_offset)
{
	return this->move_front(move_offset, this->size() - move_offset);
}
