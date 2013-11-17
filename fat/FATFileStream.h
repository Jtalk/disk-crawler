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

#include <fat/FSFileStream.h>

class FATFileStream : public FSFileStream
{
        size_t cluster_size;
        
        size_t fat_offset;
        size_t fat_size;
        
        size_t data_offset;
        size_t data_size;
        
        size_t cluster_num;
        size_t cluster_pos;
        
        bool is_correct;
        
        FATFileStream() = delete;
        FATFileStream(const FATFileStream& other) = delete;
        void init();

public:
        FATFileStream(stream_t &stream, size_t absolute_offset);
        virtual ~FATFileStream();
        
        virtual size_t read(uint8_t* buffer, size_t size);
        
        virtual size_t seekg(size_t offset);
        virtual size_t tellg() const;
        
        virtual bool correct() const;
};
