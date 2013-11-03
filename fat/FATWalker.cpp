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


#include "FATWalker.h"

#include "utility.h"

using namespace std;

template<typename Target>
Target to(const FATWalker::bytes_t &bytes) 
{
        Target value = *reinterpret_cast<const Target*>(&bytes[0]);
        return value;
}

FATWalker::FATWalker(const string& device_name):
        device(device_name, ios_base::in | ios_base::binary)
{}

FATWalker::~FATWalker()
{}

FATWalker::bytes_t FATWalker::read(FATWalker::Offset offset, size_t bytes_to_read) const 
{
        bytes_t buffer(bytes_to_read, 0);
        
        this->device.seekg(static_cast<size_t>(offset));
        this->device.read(reinterpret_cast<ifstream::char_type*>(&buffer[0]), bytes_to_read);
        
        return buffer;
}

uint16_t FATWalker::sector_size() const 
{
        auto bytes = this->read(SECTOR_SIZE, 2);
        return to<uint16_t>(bytes);
}

uint8_t FATWalker::sectors_per_cluster() const 
{
        auto bytes = this->read(SECTORS_PER_CLUSTER, 1);
        return to<uint8_t>(bytes);;
}

size_t FATWalker::cluster_size() const 
{
        return this->sector_size() * this->sectors_per_cluster();
}

uint8_t FATWalker::tables_count() const 
{
        auto bytes = this->read(NUMBER_OF_TABLES, 1);
        return to<uint8_t>(bytes);    
}

uint16_t FATWalker::root_entries_count() const 
{
        auto bytes = this->read(NUMBER_OF_ROOT_DIRECTORY_ENTRIES, 2);
        return to<uint16_t>(bytes);        
}

uint32_t FATWalker::file_size(const std::vector< uint8_t >& dir_entry)
{
        constexpr size_t ENTRY_SIZE = 32;
        
        utility::assert(dir_entry.size() < ENTRY_SIZE, "Invalid directory entry size");
        
        return to<uint32_t>(&dir_entry[FILE_SIZE]);
}

void FATWalker::find(const byte_array_t& to_find, std::list<result_t> &found)
{
        auto found = this->find_by_signatures();
        
        for (auto &file : found) {
                const bytes_t &file_data = this->traceback(file);
                size_t pos = utility::find(byte_stream_t(file_data), to_find);
                if (pos != bytes_t::npos) {
                        found.push_back({file_data, pos});
                }
        }
}


bool FATWalker::operator!() const
{
        return !this->device;
}