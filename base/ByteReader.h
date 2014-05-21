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

#include "Buffer.h"
#include "Log.h"

#include <cinttypes>
#include <cstddef>

class ByteReader
{
public:
	typedef size_t streampos;
	
	static const streampos npos = streampos(-1);

	ByteReader() = default;
	ByteReader(const ByteReader& other) = delete;
	virtual ~ByteReader() 
	{}

	virtual streampos read(Buffer &buffer, streampos size) = 0;
	
        virtual void seekg(streampos offset) = 0;
        virtual streampos tellg() const = 0;
        virtual bool eof() const = 0;
        
	virtual bool operator !() const = 0;
	
	virtual void reset() {
		this->seekg(0);
	}
	
	virtual streampos start_offset() const {
		logger()->error("Start offset is got from base class while must be implemented in derived");
		return 0;
	}
	
	struct offset_hash {
		size_t operator () (const ByteReader *reader) const {
			return std::hash<streampos>()(reader->start_offset());
		}
	};
	
	struct offset_eq {
		bool operator () (const ByteReader *fst, const ByteReader *snd) const {
			return fst->start_offset() == snd->start_offset();
		}
	};
};