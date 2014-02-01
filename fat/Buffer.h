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

#include <cstdlib>
#include <cstdint>

class Buffer
{
	uint8_t *buffer;
	size_t length;
	size_t real_length;

	void reallocate(size_t new_length){
		free(this->buffer);
		this->buffer = (uint8_t*)malloc(new_length);
	}

public:
	typedef uint8_t* iterator;
	typedef const uint8_t* const_iterator;
	
	static const size_t npos = size_t(-1);
	
	Buffer(size_t count) {
		this->buffer = (uint8_t*)malloc(count);
		this->real_length = this->length = count;
	}
	
	~Buffer() {
		free(this->buffer);
	}

	void resize(size_t size) {
		this->length = size;
		
		if (this->real_length < size) {
			this->reallocate(size);
		}			
	}
	
	iterator begin() {
		return this->buffer;
	}
	
	iterator end() {
		return this->buffer + this->length;
	}
	
	const_iterator cbegin() const {
		return this->buffer;
	}
	
	const_iterator cend() const {
		return this->buffer + this->length;
	}
	
	size_t size() const {
		return this->length;
	}
};
