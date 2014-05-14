/*
 * Disk Crawler Library.
 * Copyright (C) 2014  Jtalk <me@jtalk.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "PlainWalker.h"

#include "PlainFileStream.h"

#include "utility.h"

PlainWalker::PlainWalker(const std::string &device_name, size_t size, const utility::progress_callback_t &callback):
	SignatureWalker(device_name, size, callback)
{}

SignatureWalker::results_t PlainWalker::find(const byte_array_t &to_find) {
	auto decoder = new PlainFileStream(this->device_name);
	
	results_t results;
	results.push_back({decoder, {}});
	
	auto &offsets = results.front().second;
	ByteReader::streampos offset = 0;
	
	while (not decoder->eof()) {
		auto pos = utility::find(*decoder, to_find, offset, this->device_size, this->progress_callback);
		if (pos == PlainFileStream::npos) {
			if (this->progress_callback) {
				this->progress_callback(100);
			}
			break;
		} else {
			offsets.push_back(pos);
			offset = pos + 1;
		} 
	}
	
	if (offsets.empty()) {
		delete decoder;
		results.clear();
	} 
	
	return results;
}
