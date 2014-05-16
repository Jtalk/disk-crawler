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
#include <vector>
#include <unordered_map>

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

#define CONTAINER(x) std::begin(x), std::end(x)

class SignatureWalker;

namespace utility
{

static const size_t BUFFER_SIZE = 10000000;
static const size_t MAX_DEVICE_SIZE = std::numeric_limits<size_t>::max();

typedef std::function<void(int)> progress_callback_t;
typedef int32_t heuristics_t[sizeof(uint8_t) + 1];

struct BmHeuristics {
	heuristics_t stop;
	heuristics_t suffix;
	bool initialized;
	BmHeuristics() : initialized(false) {}
};

struct SearchResult {
	int64_t pattern_n;
	ByteReader::streampos offset;
};

bool dump(ByteReader &reader, const std::string &filename);
void sanitize(Buffer &buffer);
SignatureWalker *walker(const std::string &fs, std::string &device_name, size_t size, const progress_callback_t &callback);
size_t overlap_size(const search_terms_t &to_find);

SearchResult rabin_karp(const Buffer &string, const search_terms_t &to_find);
SearchResult str_find(const Buffer &string, const search_terms_t &to_find);
size_t str_find(const Buffer &string, const byte_array_t &to_find, BmHeuristics &heuristics);
SearchResult find(ByteReader &stream, const search_terms_t &to_find, 
		  typename ByteReader::streampos offset = 0, 
		  size_t total_size = MAX_DEVICE_SIZE, 
		  const progress_callback_t &callback = progress_callback_t());

template<typename Target>
inline Target to(const byte_array_t &bytes)
{
	return *reinterpret_cast<const Target*>(&bytes[0]);
}

}
