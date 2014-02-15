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

#include "FSFileStream.h"

#include <memory>

class BaseDecoder
{
	BaseDecoder() = delete;
	BaseDecoder(const BaseDecoder&) = delete;

public:
	typedef FSFileStream::streampos streampos;
	typedef std::shared_ptr<FSFileStream> stream_t;
		
	static const streampos npos = FSFileStream::npos;

protected:
	stream_t stream;

public:
	BaseDecoder(const stream_t &stream);
	virtual ~BaseDecoder();

	virtual streampos read(uint8_t *buffer, streampos size) = 0;

	virtual void seekg(streampos offset) = 0;
	virtual streampos tellg() const = 0;
	virtual bool eof() const = 0;
	bool operator !() const;
};
