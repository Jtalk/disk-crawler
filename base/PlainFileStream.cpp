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

#include "PlainFileStream.h"

#include <cstdio>

PlainFileStream::PlainFileStream(const std::string& device_name):
	FSFileStream(device_name, 0)
{}

bool PlainFileStream::correct() const {
	return not this->eof();
}

bool PlainFileStream::eof() const {
	return feof(this->stream) or ferror(this->stream);
}

ByteReader::streampos PlainFileStream::tellg() const {
	return ftell(this->stream);
}

void PlainFileStream::seekg(ByteReader::streampos offset) {
	fseek(this->stream, offset, SEEK_SET);
}

ByteReader::streampos PlainFileStream::read(Buffer &buffer, ByteReader::streampos size) {
	buffer.reset(size);
	auto read_bytes = fread(buffer.begin(), 1, size, this->stream);
	buffer.shrink(read_bytes);
	return read_bytes;
}
