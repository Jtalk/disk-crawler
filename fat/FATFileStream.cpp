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
#include <cstdlib>

enum FATOffsets : size_t {
        SECTOR_SIZE = 0x0B,
        CLUSTER_SIZE = 0x0D,
        RESERVED_SECTORS = 0x0E,
        NUMBER_OF_FATS = 0x10,
        NUMBER_OF_ROOT_ENTRIES = 0x11,
        LOGICAL_SECTORS_COUNT = 0x13,
        SECTORS_PER_FAT = 0x16,
        LOGICAL_SECTORS_32 = 0x20,
        SECTORS_PER_FAT32 = 0x24
};

enum FATDirectoryOffsets : size_t {
        SIZE_OFFSET = 0x1C
};

FATFileStream::FATFileStream(FILE *stream, streampos absolute_offset)
        : FSFileStream(stream), is_correct(true)
{
        this->init();
        
        if (!this->correct())
                return;
        
        fseek(this->stream, absolute_offset, SEEK_SET);
        this->is_correct = (absolute_offset >= this->device.data_offset) && !ferror(this->stream);
        
        if (!this->correct())
                return;
        
        int64_t data_offset = absolute_offset - this->device.data_offset;
        
        this->info.file_cluster_num = data_offset / this->device.cluster_size;
        this->info.file_cluster_pos = 0;
	
        this->info.current_cluster_offset = this->device.cluster_size * this->info.file_cluster_num;
}

FATFileStream::~FATFileStream()
{}

size_t FATFileStream::fat_type() const
{
        auto count = this->device.data_clusters_count;
        if (count > 4085 and count <= 65524)
                return 16;
        
        if (count > 65524)
                return 32;
        
        abort();        
}

size_t FATFileStream::eoc() const
{
        switch(this->device.fat_entry_size) {
                case 16: return 0xFFF8;
                case 32: return 0xFFFFFF8;
                default: abort();
        }
}

void FATFileStream::init()
{
        auto sector_size = this->get<uint16_t>(SECTOR_SIZE);
        this->device.cluster_size = sector_size * this->get<uint8_t>(CLUSTER_SIZE);
        
        this->device.fat_offset = sector_size * this->get<uint16_t>(RESERVED_SECTORS);
        
        auto number_of_fats = this->get<uint8_t>(NUMBER_OF_FATS);
        auto number_of_root_entries = this->get<uint16_t>(NUMBER_OF_ROOT_ENTRIES);
        
        this->device.fat_size = this->get<uint16_t>(SECTORS_PER_FAT) * sector_size;
        if (this->device.fat_size == 0)
                this->device.fat_size = this->get<uint32_t>(SECTORS_PER_FAT32) * sector_size;
                
        streampos fats_end = this->device.fat_offset + streampos(number_of_fats * this->device.fat_size);
        
        size_t root_directory_entries_size = number_of_root_entries * 32 / sector_size;
        
        this->device.data_offset = fats_end + streampos(root_directory_entries_size);
        
        size_t total_sectors = this->get<uint16_t>(LOGICAL_SECTORS_COUNT);
        if (total_sectors == 0)
                total_sectors = this->get<uint32_t>(LOGICAL_SECTORS_32);
        
        this->device.data_clusters_count = (total_sectors - this->device.data_offset / sector_size) * sector_size / this->device.cluster_size;
        
        this->device.fat_entry_size = this->fat_type();
        this->device.eoc = this->eoc();
        
        this->is_correct = !ferror(this->stream);
}

size_t FATFileStream::read_fat(streampos fat_entry_offset)
{
        switch(this->device.fat_entry_size) {
                case 16: return this->get<uint16_t>(fat_entry_offset);
                case 32: return this->get<uint32_t>(fat_entry_offset);
                default: abort();
        }
}

FATFileStream::streampos FATFileStream::find_next_cluster()
{
        size_t number = (this->info.current_cluster_offset - this->device.data_offset) / this->device.cluster_size;
        streampos fat_entry_offset = this->device.fat_offset + streampos(this->device.fat_entry_size * number);
        auto entry = this->read_fat(fat_entry_offset);
        if (entry == 0)
                return this->info.current_cluster_offset += this->device.cluster_size;
        else if (entry >= this->device.eoc)
                return streampos(-1);
        else
                return this->device.data_offset + streampos(this->device.cluster_size * entry);
}

void FATFileStream::update_cluster()
{
        int cluster_increment;
        this->info.file_cluster_pos = std::remquo((double)this->info.file_cluster_pos, (double)this->device.cluster_size, &cluster_increment);
        for (int i = 0; i < cluster_increment; i++) {
                auto next_offset = this->find_next_cluster();
                bool is_eof = next_offset == streampos(-1);
                if (is_eof)
                        return;
                this->info.current_cluster_offset = next_offset;                
        }
        this->info.file_cluster_num += cluster_increment;
        fseek(this->stream, this->info.current_cluster_offset + this->info.file_cluster_pos, SEEK_SET);
}

FATFileStream::streampos FATFileStream::read(uint8_t *buffer, streampos size)
{
        size_t read = 0;
        while(size > 0) {
                size_t rest = this->device.cluster_size - this->info.file_cluster_pos;
                size_t read = fread(buffer, 1, rest, this->stream);
                
                if (this->eof())
                        break;
                
                size -= read;
                buffer += read;
                this->info.file_cluster_pos += read;
                
                this->update_cluster();
                
                if (this->eof())
                        break;
        }
        return read;
}

FATFileStream::streampos FATFileStream::tellg() const
{
        return this->info.file_cluster_num * this->device.cluster_size + this->info.file_cluster_pos;
}

bool FATFileStream::eof() const
{
        return feof(this->stream);
}

void FATFileStream::seekg(streampos offset)
{
        int quo;
        this->info.file_cluster_pos = std::remquo((double)offset, (double)this->device.cluster_size, &quo);
        this->info.file_cluster_num = quo;
        this->update_cluster();
}

bool FATFileStream::correct() const
{
        return this->is_correct;
}