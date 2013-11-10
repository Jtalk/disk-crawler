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

#include "types.h"

#include <forward_list>
#include <fstream>
#include <string>
#include <utility>

class FSFileStream;

class FSWalker
{
public:
        typedef std::pair<FSFileStream*, size_t> result_t;
        typedef std::list<result_t> results_t;
        
private:
        FSWalker() = delete;
        FSWalker(const FSWalker& other) = delete;
        virtual FSWalker& operator=(const FSWalker& other) = delete;        
        
protected:
        typedef std::forward_list<size_t> possible_matches_t;
        
        typedef std::list<byte_array_t> signatures_t;
        static const signatures_t signatures;
        
        mutable std::basic_ifstream<uint8_t> device;
        
        virtual FSFileStream *traceback(size_t absolute_offset) const = 0;
        virtual possible_matches_t find_by_signatures() const = 0;
        
public:
        FSWalker(const std::string &device_name);
        virtual ~FSWalker();
        
        results_t find(const byte_array_t &to_find);
        
        bool operator ! () const;
};
