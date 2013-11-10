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

#pragma once

#include <fstream>

class FSFileStream
{
        typedef std::basic_fstream<uint8_t> stream_t;

        stream_t *const stream;
        const FileSystem *const fs;

        size_t chunk_num;
        size_t chunk_pos;

        size_t file_start_offset;

        FSFileStream() = delete;
        FSFileStream(const FSFileStream &other) = delete;
    
public:
        FSFileStream(stream_t &file, const FileSystem &fs);
        virtual ~FSFileStream();

        size_t seekg(size_t offset);
        size_t tellg();

        size_t read(uint8_t *buffer, size_t size);
};