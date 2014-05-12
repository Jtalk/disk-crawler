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

#include "SignatureWalker.h"

template<class FileStreamer>
class FSWalker : public SignatureWalker
{        
protected:
        virtual FSFileStream* traceback(size_t absolute_offset) const {
		auto stream = new FileStreamer(this->device_name, (typename FileStreamer::streampos)absolute_offset);
		if (stream->correct())
			return stream;
		delete stream;
		return nullptr;                
	}
                
public:
        FSWalker(const std::string &device_name):
		SignatureWalker(device_name)
	{}
	
        virtual ~FSWalker()
	{}
};