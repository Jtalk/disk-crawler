/*
    Disk Crawler Library.
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

#include "types.h"

#include <unistd.h>

#include <forward_list>
#include <fstream>
#include <list>
#include <string>
#include <vector>
#include <utility>

class BaseDecoder;
class FSFileStream;

class FSWalker
{
public:
	typedef std::pair<BaseDecoder*, size_t> result_t;
	typedef std::list<result_t> results_t;

protected:
	enum SignatureType {
		ZIP = 0,

		MAX_SIGNATURE
	};

	struct PossibleMatch {
		size_t offset;
		SignatureType signature;
	};

	typedef std::forward_list<PossibleMatch> possible_matches_t;
	typedef std::vector<byte_array_t> signatures_t;
	typedef FILE* device_t;

	static const signatures_t signatures;

	mutable device_t device;

	virtual FSFileStream *traceback(size_t absolute_offset) const = 0;
	virtual possible_matches_t find_by_signatures() const = 0;

private:
	FSWalker() = delete;
	FSWalker(const FSWalker& other) = delete;
	virtual FSWalker& operator=(const FSWalker& other) = delete;

	results_t && find(FSFileStream *stream, SignatureType type, const byte_array_t &to_find);
	BaseDecoder* decode(FSFileStream* stream, SignatureType signature);

public:
	static signatures_t make_signatures();

	FSWalker(const std::string &device_name);
	virtual ~FSWalker();

	results_t find(const byte_array_t &to_find);

	bool operator !() const;
};

inline constexpr const uint8_t* operator "" _us(const char *s, size_t)
{
	return (const uint8_t*) s;
}
