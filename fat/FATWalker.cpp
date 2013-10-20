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

using namespace std;

template<typename Target>
Target to(const FATWalker::bytes_t &bytes) {
        Target value = *reinterpret_cast<const Target*>(&bytes[0]);
        return value;
}

FATWalker::FATWalker(const string& device_name):
        device(device_name, ios_base::in | ios_base::binary)
{}

FATWalker::~FATWalker()
{}

FATWalker::bytes_t FATWalker::read(FATWalker::Offset offset, size_t bytes_to_read) const {
        bytes_t buffer(bytes_to_read, 0);
        
        this->device.seekg(static_cast<size_t>(offset));
        this->device.read(reinterpret_cast<ifstream::char_type*>(&buffer[0]), bytes_to_read);
        
        return buffer;
}

uint16_t FATWalker::sector_size() const {
        auto bytes = this->read(SECTOR_SIZE, 2);
        return to<uint16_t>(bytes);
}

uint8_t FATWalker::sectors_per_cluster() const {
        auto bytes = this->read(SECTORS_PER_CLUSTER, 1);
        return to<uint8_t>(bytes);;
}

size_t FATWalker::cluster_size() const {
        return this->sector_size() * this->sectors_per_cluster();
}

uint8_t FATWalker::tables_count() const {
        auto bytes = this->read(NUMBER_OF_TABLES, 1);
        return to<uint8_t>(bytes);;        
}
