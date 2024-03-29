/*
    Disk Crawler library.
    Copyright (C) 2013-2014 Roman Nazarenko <me@jtalk.me>

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

#include "utility.h"

#include "Log.h"
#include "FSWalker.h"
#include "SignatureWalker.h"
#include "ExtFileStream.h"
#include "FATFileStream.h"

#include "/usr/local/include/iconv.h"

#include <utility>

#include <clocale>
#include <cctype>

using namespace std;

namespace utility {

template<class T>
void merge(T &dest, const T &src) {
	auto insert = back_inserter<T>(dest);
	for (auto &i : src) {
		*(insert++) = i;
	}
}
	
size_t str_find(const Buffer &string, const byte_array_t &substr) {
	byte_array_t array(string.cbegin(), string.size());

	// TODO: Bayer-Moore

	auto result = array.find(substr);
	if (result == byte_array_t::npos)
		return Buffer::npos;
	else
		return result;
}

bool dump(ByteReader &reader, const std::string &filename) {
	using namespace std;

	fstream file(filename, ios_base::binary | ios_base::out | ios_base::trunc);

	if (not file.is_open()) {
		logger()->warning("File %s cannot be opened for write", filename.c_str());
		return false;
	}

	reader.seekg(0);

	Buffer buffer(BUFFER_SIZE);
	auto read = reader.read(buffer, BUFFER_SIZE);

	while (read > 0 and read != ByteReader::npos) {
		const char *buffer_raw = reinterpret_cast<const char*>(buffer.cbegin());
		file.write(buffer_raw, buffer.size());
		buffer.clear();
		read = reader.read(buffer, BUFFER_SIZE);
	}

	return true;
}

SignatureWalker *walker(const std::string &fs, std::string &device_name, size_t size, const progress_callback_t &callback) {
	if (fs.substr(0, 3) == "ext") {
		return new FSWalker<ExtFileStream>(device_name, size, callback);
	}
	if (fs == "vfat" or fs.substr(0, 3) == "fat") {
		return new FSWalker<FATFileStream>(device_name, size, callback);
	}
	return nullptr;
}

size_t overlap_size(const search_terms_t &to_find) {
	size_t size = 0;
	for (const auto &term : to_find) {
		size = std::max(size, term.pattern.size());
	}
	return size;
}

found_offsets_t find(ByteReader &stream, const search_terms_t &to_find, size_t size, const progress_callback_t &callback)
{
	if (not stream) {
		return {};
	}

	long int buffers_overlap = overlap_size(to_find);

	Buffer buffer(BUFFER_SIZE);
	stream.seekg(0);
	
	DEBUG_ASSERT(0 == stream.tellg(), "Unable to set stream offset %u in utility::find", 0);

	found_offsets_t results;
	while (not stream.eof() and stream.tellg() != ByteReader::npos) {
		buffer.reset(BUFFER_SIZE);
		auto pos = stream.tellg();
		auto read_bytes = stream.read(buffer, BUFFER_SIZE);
		buffer.shrink(read_bytes);
		
		int64_t pattern_n = 0;
		for (const auto &pattern : to_find) {
			size_t in_buffer_offset = 0;
			do {
				size_t found_pos = utility::str_find(buffer, pattern.pattern);
				
				if (found_pos == Buffer::npos) {
					break;
				}
								
				in_buffer_offset += found_pos;
				
				if (callback) {
					callback((pos + in_buffer_offset) * 100 / size);
				}
				
				buffer.move_front(found_pos + pattern.pattern.length());
				results.push_back({pattern_n, pos + in_buffer_offset});
				in_buffer_offset += pattern.pattern.length();
			} while (true);
			
			buffer.reset_offset();
			
			if (callback) {
				callback((pos + read_bytes) * 100 / size);
			}
			
			++pattern_n;
                }
                
                if (stream.eof()) {
			break;
                }

		stream.seekg(stream.tellg() - buffers_overlap);

		usleep(PAUSE_DURATIION_MSEC);
	}

	return results;
}

struct EncConverter {
	iconv_t converter;
	const string *target_enc;
};
typedef std::list<EncConverter> converters_t;

static bool enc_is_utf(const string &name) {
	return name.substr(0, 3) == "UTF" and name.size() > 3;
}

static bool enc_is_koi(const string &name) {
	return name.substr(0, 3) == "KOI" and name.size() > 4;
}

static string enc_filter_utf(const string &name) {
	if (name[3] == '-') {
		return name;
	} else {
		return name.substr(0, 3) + "-" + name.substr(3);
	}
}

static string enc_filter_koi(const string &name) {
	if (name[4] == '-') {
		return name;
	} else {
		return name.substr(0, 4) + "-" + name.substr(4);
	}
}

static string enc_filter(const string &raw) {
	string output;
	for (auto &value : raw) {
		if (value == '.') {
			output.clear();
		} else if (isalnum(value) or value == '-') {
			output.push_back(toupper(value));
		}
	}
	if (enc_is_utf(output)) {
		output = enc_filter_utf(output);
	}
	if (enc_is_koi(output)) {
		output = enc_filter_koi(output);
	}
	return output;
}

static bool enc_make_converter(iconv_t &result, const string &from, const string &to) {
	result = iconv_open(to.c_str(), from.c_str());
	logger()->debug("Creating converter from %s to %s", from.c_str(), to.c_str());
	if (result == iconv_t(-1)) {
		if (errno == EINVAL) {
			logger()->warning("Conversion from %s to %s is not supported by this platform\'s iconv library", from.c_str(), to.c_str());
		} else {
			logger()->warning("Unknown error %d while converting encoding from %s to %s", errno, from.c_str(), to.c_str());
		}
		return false;
	} else {
		return true;
	}	
}

static converters_t enc_make_converters(const string &from, encodings_t &to) {
	converters_t converters;
	for (auto &encoding : to) {
		EncConverter c;
		auto filtered = enc_filter(encoding);
		if (filtered == from) {
			continue;
		}
		
		auto made = enc_make_converter(c.converter, from, filtered);
		
		if (made) {
			encoding.swap(filtered);
			c.target_enc = &encoding;
			logger()->debug("Adding converter for %s", filtered.c_str());
			converters.push_back(c);
		}
	}
	return converters;
}

static byte_array_t enc_convert(const EncConverter &converter, const byte_array_t &source) {
	size_t source_size = source.size();
	size_t dest_size = source.size() * 5; // For UTF-32 to be stored completely
	byte_array_t result(dest_size, 0);
	
	char *in = (char*)source.c_str();
	char *out = (char*)&result[0];
	// We can now use basic_string as contiguous storage array as it's required by C++ 2011 standard on which this library
	// is highly rely. We can also convert to char* freely as C++ 2003+ standard declares char as one-byte type.
	auto status = iconv(converter.converter, &in, &source_size, &out, &dest_size);
	
	if (status == size_t(-1)) {
		switch (errno) {
		case E2BIG:
			logger()->warning("Not enough buffer size to convert %s to %s encoding, please, report this to developers", 
				(const char*)source.c_str(), converter.target_enc->c_str());
			break;
		case EILSEQ:
			logger()->warning("Invalid multibyte sequence %s inputed, please, report this to developers", (const char*)source.c_str());
			break;
		default:
			logger()->warning("Unknown error %u while converting %s to encoding %s", errno, (const char*)source.c_str(), converter.target_enc->c_str());
			break;
		}
		return {};
	} else {
		result.resize(out - (char*)&result[0]);
		return result;
	}
}

static void enc_clear_converters(converters_t &converters) {
	for (auto &converter : converters) {
		iconv_close(converter.converter);
	}
}

static string enc_get_local() {
	string current_locale = setlocale(LC_ALL, nullptr);
	return enc_filter(current_locale);
}

void encode(Options &opts) {
	auto current_enc = enc_get_local();
	
	for (auto &i : opts.to_find) {
		i.encoding = current_enc;
	}
	
	if (opts.encodings.empty()) {
		return;
	}
	
	if (current_enc.empty()) {
		logger()->warning("Invalid system locale %s, please, report this to developers", setlocale(LC_ALL, nullptr));
		return;
	}
	
	auto converters = enc_make_converters(current_enc, opts.encodings);
	opts.to_find.reserve(opts.to_find.size() * (converters.size() + 1));
	
	search_terms_t old = std::move(opts.to_find);
	opts.to_find.clear();
	for (const auto &term : old) {
		size_t i = 0;
		for (const auto &converter : converters) {
			if (*converter.target_enc == current_enc) {
				continue;
			}
			auto result = enc_convert(converter, term.pattern);
			if (result.empty()) {
				logger()->warning("Invalid conversion for string %s from %s to %s", (char*)term.pattern.c_str(), current_enc.c_str(), converter.target_enc->c_str());
			} else {
				opts.to_find.push_back({result, *converter.target_enc});
			}
			++i;
		}
	}
	merge(opts.to_find, old);
	
	enc_clear_converters(converters);
}

static void san_same_enc(Buffer &buffer, uint8_t placeholder) {
	Buffer tmp(buffer.size() + 1);
	size_t last = 0;
	
	for (size_t i = 0; i < buffer.size(); i++) {
		int character = buffer.cbegin()[i];
		
		if (character == 0) {
			break;
		}
		
		if (isprint(character) or isspace(character) or iscntrl(character)) {
			tmp.begin()[last++] = (uint8_t)character;
		} else if (placeholder) {
			tmp.begin()[last++] = (uint8_t)placeholder;
		}
	}
	
	tmp.begin()[last] = 0;
	tmp.shrink(last + 1);
	buffer.exchange(tmp);	
}

bool san_try_encode(const EncConverter &c, Buffer &b) {
	static const int ENABLE = 1;
	iconvctl(c.converter, ICONV_SET_DISCARD_ILSEQ, (void*)&ENABLE);
	
	size_t source_size = b.size();
	size_t dest_size = b.size() * 5; // For UTF-32 to be stored completely
	Buffer result(dest_size);
	
	char *in = (char*)b.cbegin();
	char *out = (char*)result.cbegin();	
	// We can now use basic_string as contiguous storage array as it's required by C++ 2011 standard on which this library
	// is highly rely. We can also convert to char* freely as C++ 2003+ standard declares char as one-byte type.
	auto status = iconv(c.converter, &in, &source_size, &out, &dest_size);
	
	if (status == size_t(-1)) {
		switch (errno) {
		case E2BIG:
			logger()->warning("Not enough buffer size to convert %s to %s encoding, please, report this to developers", 
				(const char*)b.cbegin(), c.target_enc->c_str());
			break;
		case EILSEQ:
			logger()->warning("Invalid multibyte sequence %s inputed, please, report this to developers", (const char*)b.cbegin());
			break;
		default:
			logger()->warning("Unknown error %u while converting %s to encoding %s", errno, (const char*)b.cbegin(), c.target_enc->c_str());
			break;
		}
		return false;
	} else {
		result.shrink(out - (char*)result.cbegin());
		b.exchange(result);
		return true;
	}
}

static bool san_diff_enc(Buffer &buffer, const string &system_enc, const string &encoding) {
	EncConverter c;
	auto made = enc_make_converter(c.converter, encoding, system_enc);
	if (not made) {
		return false;
	}
	c.target_enc = &system_enc;
	bool success = san_try_encode(c, buffer);
	converters_t converters = {c};
	enc_clear_converters(converters);
	return success;
}

bool sanitize(Buffer &buffer, const string &encoding, uint8_t placeholder) {
	auto system_enc = enc_get_local();
	if (system_enc == encoding) {
		logger()->debug("Sanitizing system charset encoded buffer");
		san_same_enc(buffer, placeholder);
		return true;
	} else {
		logger()->debug("Sanitizing other charset encoded buffer");
		return san_diff_enc(buffer, system_enc, encoding);
	}	
}

}
