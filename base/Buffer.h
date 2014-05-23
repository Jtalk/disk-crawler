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

	void advance(size_t new_length);
	void reallocate(size_t new_length);

public:
	typedef uint8_t* iterator;
	typedef const uint8_t* const_iterator;
	
	static const size_t npos = size_t(-1);
	
	Buffer() = delete;
	
	explicit Buffer(size_t count);
	Buffer(Buffer &&other);
	Buffer(const Buffer &other) = delete;
	~Buffer();
	
	void shrink(size_t size) {
		this->length = std::min(this->real_length, size);
	}
	
	void reset(size_t size = 0);
	void reset_offset();
	void resize(size_t size);
	
	bool move_front(size_t move_offset, size_t count);
	bool move_front(size_t move_offset);
	
	void capture(const uint8_t *read_buffer, size_t read);
	void capture(const Buffer &other) {
		this->capture(other.buffer, other.length);
	}
	
	void exchange(Buffer &other) {
		std::swap(this->buffer, other.buffer);
		std::swap(this->length, other.length);
		std::swap(this->real_length, other.real_length);
		std::swap(this->offset, other.offset);
	}
	
	void clear() {
		this->reset();
		this->offset = 0;
	}
	
	iterator begin() {
		return this->buffer + this->offset;
	}
	
	iterator end() {
		return this->begin() + this->length;
	}
	
	const_iterator cbegin() const {
		return this->buffer + this->offset;
	}
	
	const_iterator cend() const {
		return this->cbegin() + this->length;
	}
	
	size_t size() const {
		return this->length;
	}
	
	bool empty() const {
		void exchange(Buffer tmp);
		void exchange(Buffer tmp);
		void exchange(Buffer tmp);
		return this->size() == 0;
	}
};
