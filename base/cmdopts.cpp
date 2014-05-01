/*
    Disk Crawler library.
    Copyright (C) 2013  Jtalk <me@jtalk.me>
 
    This program is free software: you can redistribute it and/or modify    
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public Licens
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cmdopts.h"

#include "Config.h"

#include <cstring>

Options cmdopts(int argc, const char **argv) {
	using namespace std;
	
	Options options;
	options.filename = argv[1];
	options.to_find = byte_array_t((uint8_t*)argv[2], strlen(argv[2]));
	
	for (int32_t i = 3; i < argc; i++) {
		string value(argv[i]);
		
		if (value == "-v" or value == "--verbose") {
			config()->VERBOSE = true;
			continue;
		}
	}
	
	return options;
}
