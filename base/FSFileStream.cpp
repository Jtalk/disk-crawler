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

#include "FSFileStream.h"

FSFileStream::FSFileStream(const std::string &device_name, streampos absolute_offset): abs_offset(absolute_offset)
{
	this->stream = fopen(device_name.c_str(), "rb");
}

FSFileStream::~FSFileStream()
{
	if (this->stream != nullptr) {
		fclose(this->stream);
	}
}

bool FSFileStream::operator!() const
{
        return !this->correct();
}