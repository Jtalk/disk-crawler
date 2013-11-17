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
        
        FSFileStream() = delete;
        FSFileStream(const FSFileStream &other) = delete;
        
protected:
        template<typename T>
        T get(size_t offset) {
                this->stream->seekg(offset);
                T value;
                this->stream->read(reinterpret_cast<stream_t::char_type*>(&value), sizeof(value) / sizeof(stream_t::char_type));
                return value;
        }
    
public:
        FSFileStream(stream_t &stream);
        virtual ~FSFileStream();
        
        virtual size_t read(uint8_t *buffer, size_t size) = 0;

        virtual size_t seekg(size_t offset) = 0;
        virtual size_t tellg() const = 0 ;
        
        virtual bool correct() const = 0;
};