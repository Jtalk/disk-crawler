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

#include "FSWalker.h"

#include "BaseDecoder.h"
#include "FSFileStream.h"
#include "Log.h"

#include "ZipDecoder.h"

#include "utility.h"

#include <cstdio>

extern Log *logger;

const FSWalker::signatures_t FSWalker::signatures(FSWalker::make_signatures());

FSWalker::FSWalker(const std::string &device_name):
	device_name(device_name)
{
	this->device = fopen(this->device_name.c_str(), "rb");
}

FSWalker::~FSWalker()
{
	if (this->device != nullptr) {
		fclose(this->device);
	}
}

FSWalker::signatures_t FSWalker::make_signatures()
{
	signatures_t new_signatures((size_t)MAX_SIGNATURE);

	new_signatures[ZIP] = byte_array_t {0x50, 0x4B, 0x03, 0x04/**/, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0xcc, 0x99, 0x59, 0x44};
	new_signatures[RAR] = byte_array_t {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07};

	return new_signatures;
}

BaseDecoder* FSWalker::decode(FSFileStream* stream, SignatureType signature)
{
	BaseDecoder::stream_t to_decode(stream);

	switch (signature) {
	case ZIP:
		return new ZipDecoder(to_decode);
	case RAR:
		return new ZipDecoder(to_decode);

	case MAX_SIGNATURE:
		break;
	}

	DEBUG_ASSERT(false, "Invalid signature ID %u in FSWalker::decode", signature);

	return nullptr;
}

FSWalker::results_t FSWalker::find(FSFileStream *stream, SignatureType type, const byte_array_t &to_find)
{
	auto decoder = this->decode(stream, type);

	results_t results;
	BaseDecoder::streampos offset = 0;
	bool has_match = false;
	while(true) {
		auto found = utility::find(*decoder, to_find, offset);

		if (found == BaseDecoder::npos) {
			break;
		}
		
		results.emplace_front(decoder, found);
		offset += (found + 1);
		has_match = true;
	}
	
	if (!has_match) {
		delete decoder;
	}

	return std::move(results);
}

FSWalker::results_t FSWalker::find(const byte_array_t& to_find)
{
	auto signature_matches = this->find_by_signatures();

	if (signature_matches.empty())
		logger->debug("No signatures detected");
	else
		logger->debug("%u signatures found", signature_matches.size());

	results_t found;

	for (auto & match : signature_matches) {
		logger->debug("Tracing back %u signature with %u offset", match.signature, match.offset);
		
		auto file_stream = this->traceback(match.offset);

		if (file_stream == nullptr) {
			logger->debug("Invalid file stream traceback for this signature");
			continue;
		}

		auto found_by_signature = this->find(file_stream, match.signature, to_find);
		logger->debug("Found %u items by signature %u", found_by_signature.size(), match.signature);
		found.splice(found.end(), found_by_signature);
	}

	return found;
}

bool FSWalker::operator !() const
{
	return !this->device;
}

