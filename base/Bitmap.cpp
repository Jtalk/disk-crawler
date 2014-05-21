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

#include "Bitmap.h"

#include "utility.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

Bitmap::Bitmap():
	storage(nullptr), size_in_bits(0) 
{}

Bitmap::Bitmap(Bitmap && other): Bitmap() {
	std::swap(this->size_in_bits, other.size_in_bits);
	std::swap(this->storage, other.storage);
}

Bitmap::~Bitmap() {
	free(this->storage);
}

void Bitmap::set(const Buffer &buffer, size_t bits_size) {
	size_t size_required = ceil(bits_size / 8.f);
	DEBUG_ASSERT(buffer.size() >= size_required, "Invalid buffer size at Bitmap set: buffer size is %u and size required is %u while bits size is %u", buffer.size(), size_required, bits_size);
	this->storage = (uint8_t*)calloc(size_required, sizeof(uint8_t));
	memcpy(this->storage, buffer.cbegin(), size_required);
	this->size_in_bits = bits_size;
}

bool Bitmap::test(size_t pos) const {
	DEBUG_ASSERT(pos < this->size_in_bits, "Invalid position %u in Bitmap test, size is %u", pos, this->size_in_bits);
	return this->storage[pos / 8] & 1 << pos % 8;
	
}

size_t Bitmap::size() const {
	return this->size_in_bits;
}