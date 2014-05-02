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

#include "FSWalker.h"

class FATWalker : public FSWalker
{       
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
        
        size_t data_offset() const;
        
protected:
        virtual FSFileStream* traceback(size_t absolute_offset) const;
        virtual possible_matches_t find_by_signatures() const;
                
public:
        FATWalker(const std::string &device_name);
        virtual ~FATWalker();
};