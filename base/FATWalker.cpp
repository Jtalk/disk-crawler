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

#include "FATFileStream.h"

#include "Buffer.h"

#include "utility.h"

FATWalker::FATWalker(const std::string& device_name):
        SignatureWalker(device_name)
{}

FATWalker::~FATWalker()
{}

FSFileStream* FATWalker::traceback(size_t absolute_offset) const
{
        auto stream = new FATFileStream(this->device_name, FATFileStream::streampos(absolute_offset));
        if (stream->correct())
                return stream;
        delete stream;
        return nullptr;                
}
