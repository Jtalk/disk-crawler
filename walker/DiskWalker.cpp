/*
    Disk Crawler library.*
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

#include "DiskWalker.h"

#include "utility.h"

#include <unistd.h>

using namespace std;

DiskWalker::DiskWalker(const std::string &device_name)
{
        this->file.open(device_name, ios_base::in | ios_base::binary);
}

bool DiskWalker::operator !() const
{
        return !this->file;
}

size_t DiskWalker::find(const byte_array_t& to_find)
{
        return utility::find(this->file, to_find);
}


