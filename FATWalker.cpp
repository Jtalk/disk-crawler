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

#include "base/Buffer.h"

#include "base/utility.h"

#include <algorithm>

FATWalker::FATWalker(const std::string& device_name):
        FSWalker(device_name)
{}

FATWalker::~FATWalker()
{}

static bool length_comparator(const byte_array_t &a, const byte_array_t &b)
{
        return a.length() < b.length();
}

FATWalker::possible_matches_t FATWalker::find_by_signatures() const
{
        possible_matches_t matches;
        
        static const size_t BUFFER_OVERLAP = std::max_element(signatures.cbegin(), signatures.cend(), length_comparator)->length();
        static const size_t BUFFER_SIZE = 100000000;
	
        Buffer buffer(BUFFER_SIZE);

        while (!feof(this->device) && !ferror(this->device) && ftell(this->device) != -1) {
                size_t pos = ftell(this->device);
                
		buffer.resize(BUFFER_SIZE);
		auto read_bytes = fread(buffer.begin(), 1, BUFFER_SIZE, this->device);
		buffer.resize(read_bytes);
		
                for (size_t signature_type = 0; signature_type < MAX_SIGNATURE; signature_type++) {
			const auto &signature = signatures[signature_type];
			size_t in_buffer_offset = 0;
			do {
				size_t found_pos = utility::str_find(buffer, signature);
				
				if (found_pos == Buffer::npos) {
					break;
				}
				
				in_buffer_offset += found_pos;
				buffer.move_front(found_pos + signature.length(), buffer.size() - found_pos - signature.length());
				matches.push_front({pos + in_buffer_offset, (SignatureType)signature_type});
			} while (true);
			
			buffer.reset_offset();
                }
                
                if (feof(this->device) || ferror(this->device))
			break;
                
		fseek(this->device, -BUFFER_OVERLAP, SEEK_CUR);
                
                usleep(500);
        }
        
        return matches;
}

FSFileStream* FATWalker::traceback(size_t absolute_offset) const
{
        auto stream = new FATFileStream(this->device, FATFileStream::streampos(absolute_offset));
        if (stream->correct())
                return stream;
        delete stream;
        return nullptr;                
}
