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

#pragma once

#include "Buffer.h"

#include "types.h"

#include <unistd.h>

#include <iostream>
#include <string>

#include <cstdlib>

static const int PAUSE_DURATIION_MSEC = 1;
static const int MICROSECOND = 1000;
static const int PAUSE_DURATION = PAUSE_DURATIION_MSEC * MICROSECOND;

namespace utility
{

#ifdef DEBUG
void assert(bool expr, const std::string &message)
{
	if (expr)
		return;

	std::cout << message << endl;

	abort();
}

void log(const std;:string &message)
{
	cout << message << endl;
}
#else
inline void assert(bool, const std::string&) {}
inline void log(const std::string&) {}
#endif


template<typename Target>
inline Target to(const byte_array_t &bytes)
{
	return *reinterpret_cast<const Target*>(&bytes[0]);
}

static size_t str_find(const Buffer &string, const byte_array_t &substr)
{
	byte_array_t array(string.cbegin(), string.size());
	
	// TODO: Bayer-Moore
	
	return array.find(substr);
}

template<class Stream>
size_t find(Stream &stream, const byte_array_t &to_find)
{
	static const size_t BUFFER_SIZE = 100000000;
	static const auto NO_POSITION = typename Stream::streampos(-1);

	if (!stream)
		return byte_array_t::npos;

	long int buffers_overlap = to_find.size();

	Buffer buffer(BUFFER_SIZE);


	while (!stream.eof() && stream.tellg() != NO_POSITION) {
		size_t pos = stream.tellg();
		
		buffer.resize(BUFFER_SIZE);
		auto read = stream.read(buffer.begin(), buffer.size());
		buffer.resize(read);
		
		size_t found_pos = str_find(buffer, to_find);

		if (found_pos != byte_array_t::npos)
			return pos + found_pos;

		stream.seekg(stream.tellg() - buffers_overlap);

		usleep(PAUSE_DURATIION_MSEC);
	}

	return byte_array_t::npos;
}

}
