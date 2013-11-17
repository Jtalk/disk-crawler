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

#include "FSFileStream.h"

#include "utility.h"

#include <algorithm>

FATWalker::FATWalker(const string& device_name):
        FSWalker(device_name)
{}

FATWalker::~FATWalker()
{}

static bool length_comparator(const byte_array_t &a, const byte_array_t &b)
{
        return a.length() < b.length();
}

possible_matches_t FATWalker::find_by_signatures() const
{
        this->device.seekg(this->data_offset());
        possible_matches_t matches;
        
        static constexpr size_t BUFFER_SIZE = 100000;
        byte_array_t buffer(BUFFER_SIZE, 0);

        size_t buffers_overlap = std::max_element(signatures.cbegin(), signatures.cend(), length_comparator)->length();
        
        while (!this->device.eof() && this->device.tellg() != -1) {
                size_t pos = this->device.tellg();
                
                this->device.read(&buffer[0], 100000);
                
                for (auto &signature : signatures) {
                        size_t found_pos = buffer.find(signature);
                        if (found_pos != byte_array_t::npos)
                                matches.push_front(pos + found_pos);
                }
                
                this->device.seekg(this->device.tellg() - buffers_overlap);
                
                usleep(1000);
        }
        
        return matches;
}

FSFileStream* FATWalker::traceback(size_t absolute_offset) const
{
        auto stream = new FATFileStream(this->device, absolute_offset);
        if (stream->correct())
                return stream;
        delete stream;
        return nullptr;                
}
