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

#include "ByteReader.h"

#include <cstdio>
#include <cinttypes>

class FSFileStream : public ByteReader
{
	FSFileStream() = delete;
	FSFileStream(const FSFileStream &other) = delete;
        
protected:
	FILE *stream;
	streampos abs_offset;
        
	template<typename T>
	T get(streampos offset) const { 
		fseek(this->stream, offset, SEEK_SET);
		T value = T();
		auto read = fread(reinterpret_cast<void*>(&value), 1, sizeof(value), this->stream);
		(void)read; // ignoring bytes count
		return value;
	}
    
	virtual bool correct() const = 0;
	
public:
	FSFileStream(const std::string &device_name, streampos absolute_offset);
	virtual ~FSFileStream();
	
	virtual bool operator !() const override final;
	
	virtual streampos start_offset() const override {
		return this->abs_offset;
	}
};