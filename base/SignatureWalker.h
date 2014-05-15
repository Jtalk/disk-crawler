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
#include "utility.h"

#include <unistd.h>

#include <fstream>
#include <list>
#include <string>
#include <vector>
#include <utility>

class BaseDecoder;
class ByteReader;
class FSFileStream;

class SignatureWalker {
public:	
	typedef std::list<utility::SearchResult> offsets_t;
	typedef std::pair<ByteReader *, offsets_t> result_t;
	typedef std::list<result_t> results_t;
	typedef std::list<SignatureWalker *> walkers_t;

protected:
	enum SignatureType {
	        ZIP0 = 0,
	        ZIP1,
	        ZIP2,
	        RAR,
//		PLAIN,

	        MAX_SIGNATURE
	};

	struct PossibleMatch {
		size_t offset;
		SignatureType signature;
	};

	typedef std::list<PossibleMatch> possible_matches_t;
	typedef std::vector<byte_array_t> signatures_t;
	typedef FILE *device_t;

	static const signatures_t signatures;

	device_t device;
	const std::string device_name;
	size_t device_size;
	utility::progress_callback_t progress_callback;

	possible_matches_t find_by_signatures() const;

	virtual FSFileStream *traceback(size_t absolute_offset) const = 0;

private:
	SignatureWalker() = delete;
	SignatureWalker(const SignatureWalker &other) = delete;
	virtual SignatureWalker &operator=(const SignatureWalker &other) = delete;

	results_t find(FSFileStream *stream, SignatureType type, const search_terms_t &to_find);
	BaseDecoder *decode(FSFileStream *stream, SignatureType signature);

public:
	static signatures_t make_signatures();

	SignatureWalker(const std::string &device_name, size_t size = utility::MAX_DEVICE_SIZE, const utility::progress_callback_t &callback = utility::progress_callback_t());
	virtual ~SignatureWalker();

	virtual results_t find(const search_terms_t &to_find);

	bool operator !() const;
};
