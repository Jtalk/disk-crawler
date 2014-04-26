/*
 * Disk Crawler Library.
 * Copyright (C) 2014  Jtalk <me@jtalk.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "Log.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>

class Buffer
{
	uint8_t *buffer;
	size_t length;
	size_t real_length;
	size_t offset;

	void reallocate(size_t new_length){
		free(this->buffer);
		this->buffer = (uint8_t*)malloc(new_length);
		this->real_length = new_length;
		this->offset = 0;
	}

	void advance(size_t new_length) {
		if (new_length <= this->real_length) {
			return;
		}
		auto old_buffer = this->buffer;
		this->buffer = (uint8_t*)malloc(new_length);
		memcpy(this->buffer, old_buffer, this->length);
		free(old_buffer);		
	}
	
public:
	typedef uint8_t* iterator;
	typedef const uint8_t* const_iterator;
	
	static const size_t npos = size_t(-1);
	
	Buffer() = delete;
	
	Buffer(size_t count) {
		this->buffer = (uint8_t*)malloc(count);
		this->real_length = this->length = count;
		this->offset = 0;
	}
	
	~Buffer() {
		free(this->buffer);
	}

	void reset(size_t size = 0) {
		this->length = size;
		
		if (this->real_length < this->length) {
			this->offset = 0;
			this->reallocate(size);
		}
	}
		
	void reset_offset() {
		this->length += this->offset;
		this->offset = 0;
	}
	
	void capture(const uint8_t *read_buffer, size_t read) {
		if (this->offset != 0)
			Log().error("Capturing buffer with non-zero offset %u detected", this->offset);
		this->advance(this->length + read);
		memcpy(this->buffer + this->length, read_buffer, read);
		this->length += read;
	}
	
	void capture(const Buffer &other) {
		this->capture(other.buffer, other.length);
	}
	
	bool move_front(size_t move_offset, size_t count) {
		if (move_offset + count > this->length) {
			return false;
		}
		
		this->offset += move_offset;
		this->length = count;
		return true;
	}
	
	void clear() {
		this->reset(0);
		this->offset = 0;
	}
	
	iterator begin() {
		return this->buffer + this->offset;
	}
	
	iterator end() {
		return this->buffer + this->length;
	}
	
	const_iterator cbegin() const {
		return this->buffer + offset;
	}
	
	const_iterator cend() const {
		return this->buffer + this->length;
	}
	
	size_t size() const {
		return this->length;
	}
	
	bool empty() const {
		return this->size() == 0;
	}
};
