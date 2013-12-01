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


#include "FSWalker.h"

#include "FSFileStream.h"

#include "utility.h"

const FSWalker::signatures_t FSWalker::signatures{
        
};

FSWalker::FSWalker(const std::string &device_name):
        device(device_name, std::ios_base::binary)
{}

FSWalker::~FSWalker()
{}

FSWalker::results_t FSWalker::find(const byte_array_t& to_find)
{
        auto possible_matches = this->find_by_signatures();
        results_t found;
        
        for (auto &match : possible_matches) {
                FSFileStream *file_stream = this->traceback(match);
                
                if (file_stream == nullptr) {
                        continue;
                }
                
                size_t pos = utility::find(*file_stream, to_find);
                
                if (pos != byte_array_t::npos) {
                        found.push_back({file_stream, pos});
                }
                else {
                        delete file_stream;
                }
        }
        
        return found;
}

bool FSWalker::operator !() const
{
        return !this->device;
}

