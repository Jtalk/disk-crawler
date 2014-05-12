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

#include "SignatureWalker.h"

#include "BaseDecoder.h"
#include "FSFileStream.h"
#include "Log.h"

#include "ZipDecoder.h"

#include "utility.h"

#include <algorithm>

#include <cstdio>

const SignatureWalker::signatures_t SignatureWalker::signatures(SignatureWalker::make_signatures());

SignatureWalker::SignatureWalker(const std::string &device_name):
	device_name(device_name)
{
	this->device = fopen(this->device_name.c_str(), "rb");
}

SignatureWalker::~SignatureWalker()
{
	if (this->device != nullptr) {
		fclose(this->device);
	}
}

SignatureWalker::signatures_t SignatureWalker::make_signatures()
{
	signatures_t new_signatures((size_t)MAX_SIGNATURE);

	new_signatures[ZIP] = byte_array_t {0x50, 0x4B, 0x03, 0x04};
	new_signatures[RAR] = byte_array_t {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07};

	return new_signatures;
}

static bool length_comparator(const byte_array_t &a, const byte_array_t &b)
{
        return a.length() < b.length();
}

SignatureWalker::possible_matches_t SignatureWalker::find_by_signatures() const
{
        possible_matches_t matches;
        
        static const size_t BUFFER_OVERLAP = std::max_element(signatures.cbegin(), signatures.cend(), length_comparator)->length();
        static const size_t BUFFER_SIZE = 100000000;
	
        Buffer buffer(BUFFER_SIZE);

        while (!feof(this->device) && !ferror(this->device) && ftell(this->device) != -1) {
                size_t pos = ftell(this->device);
                
		buffer.reset(BUFFER_SIZE);
		auto read_bytes = fread(buffer.begin(), 1, BUFFER_SIZE, this->device);
		buffer.shrink(read_bytes);
		
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

BaseDecoder* SignatureWalker::decode(FSFileStream* stream, SignatureType signature)
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

SignatureWalker::results_t SignatureWalker::find(FSFileStream *stream, SignatureType type, const byte_array_t &to_find)
{
	auto decoder = this->decode(stream, type);

	results_t results;
	BaseDecoder::streampos offset = 0;
	bool has_match = false;
	offsets_t *current_result = nullptr;
	while(true) {
		auto found = utility::find(*decoder, to_find, offset);

		if (found == BaseDecoder::npos) {
			break;
		}
		
		if (current_result == nullptr) {
			auto iter = results.emplace(results.end(), decoder, offsets_t());
			current_result = &iter->second;
		}
		
		current_result->push_back(found);
		offset = (found + 1);
		has_match = true;
	}
	
	if (!has_match) {
		delete decoder;
	}

	return std::move(results);
}

SignatureWalker::results_t SignatureWalker::find(const byte_array_t& to_find)
{
	auto signature_matches = this->find_by_signatures();

	if (signature_matches.empty())
		logger()->debug("No signatures detected");
	else
		logger()->debug("%u signatures found", signature_matches.size());

	results_t found;

	for (auto & match : signature_matches) {
		logger()->debug("Tracing back %u signature with %u offset", match.signature, match.offset);
		
		auto file_stream = this->traceback(match.offset);

		if (file_stream == nullptr) {
			logger()->debug("Invalid file stream traceback for this signature");
			continue;
		}

		auto found_by_signature = this->find(file_stream, match.signature, to_find);
		logger()->debug("Found %u items by signature %u", found_by_signature.size(), match.signature);
		found.splice(found.end(), found_by_signature);
	}

	return found;
}

bool SignatureWalker::operator !() const
{
	return !this->device;
}
