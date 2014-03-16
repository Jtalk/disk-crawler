/*
 *  Disk Crawler library.
 *  Copyright (C) 2013  Jtalk <me@jtalk.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utility.h"

#include "Log.h"

extern Log *logger;

namespace utility {
	
size_t str_find(const Buffer &string, const byte_array_t &substr)
{
	byte_array_t array(string.cbegin(), string.size());

	// TODO: Bayer-Moore

	return array.find(substr);
}

bool dump(ByteReader &reader, const std::string &filename)
{
	using namespace std;
	
	fstream file(filename, ios_base::binary | ios_base::out | ios_base::trunc);
	
	if (not file.is_open()) {
		logger->warning("File %s cannot be opened for write", filename.c_str());
		return false;
	}
	
	reader.seekg(0);
	
	Buffer buffer(BUFFER_SIZE);
	auto read = reader.read(buffer, buffer.size());
	
	while (read > 0 and read != ByteReader::npos) {
		const char *buffer_raw = reinterpret_cast<const char*>(buffer.cbegin());
		file.write(buffer_raw, buffer.size());
		buffer.resize(BUFFER_SIZE);
		read = reader.read(buffer, buffer.size());
	}
	
	return true;
}

}