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

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "FSFileStream.h"

#include <cmath>

FSFileStream::FSFileStream(stream_t &stream, size_t absolute_offset, const FileSystem &fs):
        stream(&stream),
        fs(&fs),
        chunk_num(0),
        chunk_pos(0),
        file_begin(0)
{
        this->file_begin = absolute_offset;
}

FSFileStream::~FSFileStream()
{}

size_t FSFileStream::tellg()
{
        return this->chunk_pos * fs->chunk_size() + this->chunk_pos;
}

size_t FSFileStream::seekg(size_t offset)
{
        this->chunk_pos = std::remquo(offset, fs->chunk_size(), &this->chunk_num);        
}

size_t FSFileStream::read(uint8_t* buffer, size_t size)
{
      size_t read = 0;
      size_t chunk_reminded = fs->chunk_size() - this->chunk_pos;
      size_t stream_offset = fs->offset()
}
