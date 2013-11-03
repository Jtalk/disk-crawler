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
#include <string>

class FATWalker {       
public:
        typedef std::basic_string<uint8_t> bytes_t;
        typedef std::basic_stringstream<uint8_t> byte_stream_t;
        typedef std::pair<bytes_t, size_t> result_t;
       
private:
        enum Offset {
                SECTOR_SIZE = 0x0B,
                SECTORS_PER_CLUSTER = 0x0D, 
                NUMBER_OF_TABLES = 0x010, 
                NUMBER_OF_ROOT_DIRECTORY_ENTRIES = 0x011
        };
        
        enum DirectoryEntryOffset {
                FILE_NAME = 0x0, 
                FILE_SIZE = 0x1c
        };
                
        mutable std::ifstream device;
        
        FATWalker() = delete;
        FATWalker(const FATWalker& other) = delete;
        FATWalker& operator=(const FATWalker& other) = delete;
        
        bytes_t read(Offset offset, size_t bytes_to_read) const;

public:
        FATWalker(const std::string &device_name);
        virtual ~FATWalker();

        uint16_t sector_size() const;
        uint8_t sectors_per_cluster() const;

        size_t cluster_size() const;
        
        uint8_t tables_count() const;
        
        uint16_t root_entries_count() const;
        
        uint32_t file_size(const std::vector<uint8_t> &dir_entry);
        
        size_t find(const byte_array_t &to_find);
        
        bool operator ! () const;
};