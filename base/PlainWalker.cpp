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

PlainWalker::PlainWalker(const std::string &device_name):
	FSWalker(device_name)
{}

FSWalker::results_t PlainWalker::find(const byte_array_t &to_find) {
	auto decoder = new PlainFileStream(this->device_name);
	
	results_t results;
	results.push_back({decoder, {}});
	
	auto &offsets = results.front().second;
	
	while (not decoder->eof()) {
		auto pos = utility::find(*decoder, to_find);
		if (pos == PlainFileStream::npos) {
			break;
		} else {
			offsets.push_back(pos);
		} 
	}
	
	if (offsets.empty()) {
		delete decoder;
		results.clear();
	} 
	
	return results;
}
