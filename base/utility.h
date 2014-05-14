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

#include <functional>
#include <fstream>
#include <iostream>
#include <string>
#include <limits>

#include <cmath>
#include <cstdlib>

static const int PAUSE_DURATIION_MSEC = 1;

#define RELEASE_ASSERT(EXPR, FORMAT, ...) \
	do { \
		if (!(EXPR)) { \
			logger()->error(FORMAT, __VA_ARGS__); \
		} \
	} while(0)

#ifndef DEBUG
#define DEBUG_ASSERT(EXPR, FORMAT, ...)
#else
#define DEBUG_ASSERT(EXPR, FORMAT, ...) RELEASE_ASSERT(EXPR, FORMAT, __VA_ARGS__)
#endif

namespace utility
{

static const size_t BUFFER_SIZE = 10000;
static const size_t MAX_DEVICE_SIZE = std::numeric_limits<size_t>::max();

typedef std::function<void(int)> progress_callback_t;

size_t str_find(const Buffer &string, const byte_array_t &substr);
bool dump(ByteReader &reader, const std::string &filename);
void sanitize(Buffer &buffer);

template<typename Target>
inline Target to(const byte_array_t &bytes)
{
	return *reinterpret_cast<const Target*>(&bytes[0]);
}

template<class Stream>
size_t find(Stream &stream, const byte_array_t &to_find, typename Stream::streampos offset = 0, size_t total_size = MAX_DEVICE_SIZE, const progress_callback_t &callback = progress_callback_t())
{
	if (!stream) {
		return Stream::npos;
	}

	long int buffers_overlap = to_find.size();

	Buffer buffer(BUFFER_SIZE);
	stream.seekg(offset);
	
	DEBUG_ASSERT(offset == stream.tellg(), "Unable to set stream offset %u in utility::find", offset);

	while (!stream.eof() && stream.tellg() != Stream::npos) {
		auto pos = stream.tellg();

		buffer.reset();
		auto read = stream.read(buffer, BUFFER_SIZE);

		if (read == Stream::npos) {
			return Stream::npos;
		}

		buffer.shrink(read);
		
		auto found_pos = str_find(buffer, to_find);

		if (callback) {
			callback(std::min<int>(99, floor(float(pos + BUFFER_SIZE) / total_size * 100)));
		}
		
		if (found_pos != Buffer::npos) {
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

}
