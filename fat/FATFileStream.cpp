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


#include "FATFileStream.h"

#include <cmath>

enum FATOffsets : size_t {
        SECTOR_SIZE = 0x0B,
        CLUSTER_SIZE = 0x0D,
        RESERVED_SECTORS = 0x0E,
        NUMBER_OF_FATS = 0x10,
        NUMBER_OF_ROOT_ENTRIES = 0x11,
        SECTORS_PER_FAT = 0x16,
        SECTORS_PER_FAT32 = 0x24
};

enum FATDirectoryOffsets : size_t {
        SIZE_OFFSET = 0x1C
};

FATFileStream::FATFileStream(stream_t &stream, size_t absolute_offset)
        : FSFileStream(stream), is_correct(true)
{
        this->init();
        
        if (!this->correct())
                return;
        
        this->stream->seekg(absolute_offset);
        this->is_correct = (absolute_offset >= this->data_offset) && this->stream->good();
        
        if (!this->correct())
                return;
        
        this->current_cluster_fat_index = ;
        
        this->file_cluster_num = (absolute_offset - this->data_offset) / (this->cluster_size);
        this->file_cluster_pos = 0;
        this->file_entry = this->make_file_entry();
}

FATFileStream::~FATFileStream()
{}

void FATFileStream::init()
{
        auto sector_size = this->get<uint16_t>(SECTOR_SIZE);
        this->cluster_size = sector_size * this->get<uint8_t>(CLUSTER_SIZE);
        
        this->fat_offset = sector_size * this->get<uint16_t>(RESERVED_SECTORS);
        
        auto number_of_fats = this->get<uint8_t>(NUMBER_OF_FATS);
        auto number_of_root_entries = this->get<uint16_t>(NUMBER_OF_ROOT_ENTRIES);
        
        this->fat_size = this->get<uint16_t>(SECTORS_PER_FAT) * sector_size;
        if (this->fat_size == 0)
                this->fat_size = this->get<uint32_t>(SECTORS_PER_FAT32) * sector_size;
                
        size_t fats_end = this->fat_offset + number_of_fats * this->fat_size;
        
        size_t root_directory_entries_size = number_of_root_entries * 32 / sector_size;
        
        this->data_offset = fats_end + root_directory_entries_size;
        
        this->is_correct = this->stream->good();
}

void FATFileStream::update_cluster()
{
        
}

FATFileStream::FileEntry FATFileStream::make_file_entry()
{
        FileEntry entry;
        
        size_t offset = (?) + SIZE_OFFSET;
        entry.size = this->get<uint32_t>(offset);
        
        return entry;
}

size_t FATFileStream::read(uint8_t* buffer, size_t size)
{
        size_t read = 0;
        while(size > 0) {
                size_t rest = this->cluster_size - this->file_cluster_pos;
                size_t read = this->stream->read(buffer, rest);
                
                if (!this->stream->good())
                        return read;
                
                size -= read;
                buffer += read;
                this->file_cluster_pos += read;
                
                if (this->file_cluster_pos >= this->cluster_size)
                        this->update_cluster();
        }
}

size_t FATFileStream::tellg() const
{
        return this->file_cluster_num * this->cluster_size + this->file_cluster_pos;
}

size_t FATFileStream::seekg(size_t offset)
{
        this->file_cluster_pos = std::remquo(offset, this->cluster_size, &this->file_cluster_num);
}

bool FATFileStream::correct() const
{
        return this->is_correct;
}