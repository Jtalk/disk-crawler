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

#include "fspick.h"

#include "ExtFileStream.h"
#include "FATFileStream.h"
#include "FATWalker.h"

template<class FileStream>
bool is(const std::string &device_name) {
	FileStream stream(device_name, 0);
	return stream.info().correct;
}

SignatureWalker::walkers_t fspick(const std::string &device_name) {
	SignatureWalker::walkers_t result;
	
	if (is<FATFileStream>(device_name)) {
		result.push_back(new FATWalker(device_name));
	}
	
	if (is<ExtFileStream>(device_name)) {
		//result.push_back(new ExtWalker(device_name));
	}
	
	return result;
}
