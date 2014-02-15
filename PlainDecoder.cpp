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

#include "PlainDecoder.h"

PlainDecoder::PlainDecoder(const stream_t &stream): 
	BaseDecoder(stream)
{}

PlainDecoder::~PlainDecoder()
{}

PlainDecoder::streampos PlainDecoder::read(uint8_t* buffer, streampos size)
{
	return this->stream->read(buffer, size);
}

void PlainDecoder::seekg(streampos offset)
{
	this->stream->seekg(offset);
}

PlainDecoder::streampos PlainDecoder::tellg() const
{
	return this->stream->tellg();
}

bool PlainDecoder::eof() const
{
	return this->stream->eof();
}
