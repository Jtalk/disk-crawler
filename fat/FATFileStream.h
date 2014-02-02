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
        struct FileInfo {
                size_t file_cluster_num;
                streampos file_cluster_pos;
                streampos current_cluster_offset;
        };
        
        struct DeviceInfo {
                size_t cluster_size;
                
                streampos fat_offset;
                size_t fat_size;
                
                streampos data_offset;
                size_t data_clusters_count;
                
                size_t fat_entry_size;
                size_t eoc;
		
		size_t total_sectors;
		size_t size;
        };
                
        DeviceInfo device;        
        FileInfo info;
        
        bool is_correct;
	bool is_eof;
        
        FATFileStream() = delete;
        FATFileStream(const FATFileStream& other) = delete;
        
        void init();
        
        void update_cluster();
        
        streampos find_next_cluster();
        size_t read_fat(streampos fat_entry_offset);
        
        size_t fat_type() const;
        size_t eoc() const;
        
public:
        FATFileStream(FILE *stream, streampos absolute_offset);
        virtual ~FATFileStream();
        
        virtual streampos read(uint8_t* buffer, streampos size) override;
        
        virtual void seekg(streampos offset) override;
        virtual streampos tellg() const override;
        virtual bool eof() const override;
        
        virtual bool correct() const override;
};
