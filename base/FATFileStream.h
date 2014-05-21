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

#pragma once

#include "FSFileStream.h"

#include <deque>

class FATFileStream : public FSFileStream
{
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
		
		bool correct;
        };
	
	static const size_t FATS_MAX = 4;
                
        DeviceInfo device;
	
	std::deque<streampos> clusters;
	streampos current_pos;
	size_t file_size;
        
        bool is_correct;
	bool is_eof;
        
        FATFileStream() = delete;
        FATFileStream(const FATFileStream& other) = delete;
        
	streampos data_from_n(uint32_t number);
	streampos fat_from_n(uint32_t number);
	uint32_t n_from_data(streampos offset);
	uint32_t n_from_fat(streampos offset);
	
        void init();
	void init_clusters(streampos file_offset);
        streampos find_next_cluster(streampos source_cluster);
        void update_cluster(streampos file_pos);
        
        size_t read_fat(uint32_t number);
        
        size_t fat_type() const;
        size_t eoc() const;
        
public:
        FATFileStream(const std::string &device_name, streampos absolute_offset = 0);
        virtual ~FATFileStream();
        
        virtual streampos read(Buffer &buffer, streampos size) override;
        
        virtual void seekg(streampos offset) override;
        virtual streampos tellg() const override;
        virtual bool eof() const override;
	
	const DeviceInfo& info() const;
        
        virtual bool correct() const override;
};
