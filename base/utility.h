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
#include "ByteReader.h"
#include "Log.h"

#include "types.h"

#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

#include <cstdlib>

static const int PAUSE_DURATIION_MSEC = 1;

extern Log *logger;

#define RELEASE_ASSERT(EXPR, FORMAT, ...) \
	do { \
		if (!(EXPR)) { \
			logger->error(FORMAT, __VA_ARGS__); \
		} \
	} while(0)

#ifndef DEBUG
#define DEBUG_ASSERT(EXPR, FORMAT, ...)
#else
#define DEBUG_ASSERT(EXPR, FORMAT, ...) RELEASE_ASSERT(EXPR, FORMAT, __VA_ARGS__)
#endif

namespace utility
{

static const size_t BUFFER_SIZE = 100000000;

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
	if (!stream)
		return Stream::npos;

	long int buffers_overlap = to_find.size();

	Buffer buffer(BUFFER_SIZE);
	stream.seekg(0);

	while (!stream.eof() && stream.tellg() != Stream::npos) {
		auto pos = stream.tellg();

		buffer.resize(BUFFER_SIZE);
		auto read = stream.read(buffer, buffer.size());

		if (read == Stream::npos) {
			return Stream::npos;
		}

		buffer.resize(read);

		auto found_pos = str_find(buffer, to_find);

		if (found_pos != Stream::npos) {
			return pos + found_pos;
		}

		if (stream.eof()) {
			break;
		}

		stream.seekg(stream.tellg() - buffers_overlap);

		usleep(PAUSE_DURATIION_MSEC);
	}

	return Stream::npos;
}

bool dump(ByteReader &reader, const std::string &filename)
{
	using namespace std;
	
	ofstream file(filename, ios_base::binary | ios_base::out | ios_base::in);
	
	if (not file.is_open()) {
		return false;
	}
	
	Buffer buffer(BUFFER_SIZE);
	
	while (reader.read(buffer, buffer.size()) > 0) {
		file.write(buffer.cbegin(), buffer.size());
		buffer.resize(BUFFER_SIZE);
	}
	
	return true;
}

}
