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

#include "PlainDecoder.h"
#include "ZipDecoder.h"

#include "utility.h"

#include <algorithm>

#include <cstdio>

const SignatureWalker::signatures_t SignatureWalker::signatures(SignatureWalker::make_signatures());

static constexpr float STRETCH_PERCENT_FACTOR = 0.95;

SignatureWalker::SignatureWalker(const std::string &device_name, size_t size, const utility::progress_callback_t &callback):
	device_name(device_name), device_size(size), progress_callback(callback)
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

	new_signatures[ZIP0] = byte_array_t {0x50, 0x4B, 0x03, 0x04};
	new_signatures[ZIP1] = byte_array_t {0x50, 0x4B, 0x05, 0x06};
	new_signatures[ZIP2] = byte_array_t {0x50, 0x4B, 0x07, 0x08};
	new_signatures[RAR] = byte_array_t {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07};
	//new_signatures[PLAIN] = byte_array_t {0x2f, 0x2a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x44};

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
        static const size_t BUFFER_SIZE = 100000;
	
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
				
				if (this->progress_callback) {
					this->progress_callback((pos + in_buffer_offset) * 100 * STRETCH_PERCENT_FACTOR / this->device_size);
				}
				
				buffer.move_front(found_pos + signature.length(), buffer.size() - found_pos - signature.length());
				matches.push_front({pos + in_buffer_offset, (SignatureType)signature_type});
			} while (true);
			
			buffer.reset_offset();
			
			if (this->progress_callback) {
				this->progress_callback((pos + read_bytes) * 100 * STRETCH_PERCENT_FACTOR / this->device_size);
			}
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
	case ZIP0:
		return new ZipDecoder(to_decode);
	case ZIP1:
		return new ZipDecoder(to_decode);
	case ZIP2:
		return new ZipDecoder(to_decode);
	case RAR:
		return new ZipDecoder(to_decode);
// 	case PLAIN:
// 		return new PlainDecoder(to_decode);

	case MAX_SIGNATURE:
		break;
	}

	DEBUG_ASSERT(false, "Invalid signature ID %u in FSWalker::decode", signature);

	return nullptr;
}

static void weighted_callback(const utility::progress_callback_t &callback, int raw_percents) {
	int weighted = 100 * STRETCH_PERCENT_FACTOR + raw_percents * (1 - STRETCH_PERCENT_FACTOR);
	return callback(weighted);
}

SignatureWalker::results_t SignatureWalker::find(FSFileStream *stream, SignatureType type, const search_terms_t &to_find)
{
	using std::bind;
	using namespace std::placeholders;
	
	auto decoder = this->decode(stream, type);

	logger()->debug("Parsing decoder for type %u", type);

	results_t results;
	BaseDecoder::streampos offset = 0;
	bool has_match = false;
	offsets_t *current_result = nullptr;
	
	utility::progress_callback_t weighted_callback_binded; 
	if (this->progress_callback) {
		weighted_callback_binded = bind(weighted_callback, this->progress_callback, _1);
	}
	
	while(true) {
		utility::SearchResult found = utility::find(*decoder, to_find, offset, this->device_size, weighted_callback_binded);

		if (found.pattern_n == -1) {
			break;
		}
		
		if (current_result == nullptr) {
			auto iter = results.emplace(results.end(), decoder, offsets_t());
			current_result = &iter->second;
		}
		
		current_result->push_back(found);
		offset = (found.offset + 1);
		has_match = true;
	}
	
	if (!has_match) {
		delete decoder;
	}

	return std::move(results);
}

SignatureWalker::results_t SignatureWalker::find(const search_terms_t& to_find)
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

