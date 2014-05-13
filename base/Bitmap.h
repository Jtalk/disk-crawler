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

#include "Buffer.h"

#include <cinttypes>
#include <cstddef>

class Bitmap {
	uint8_t *storage;
	size_t size_in_bits;
	
public:
	Bitmap();
	Bitmap(Bitmap &&other);
	~Bitmap();
	
	void set(const Buffer &buffer, size_t bits_size);
	
	bool test(size_t pos) const;
	size_t size() const;
	
private:
	Bitmap(const Bitmap &other) = delete;
	Bitmap &operator= (const Bitmap &other) = delete;
};