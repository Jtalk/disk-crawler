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

#include "cmdopts.h"

#include "Config.h"
#include "Log.h"

#include <sstream>

#include <cstring>

using namespace std;

static const char ENCODINGS_DELIMITER = ',';
	
static encodings_t split(const string &str, char delim) {
	stringstream ss(str);
	string current;
	encodings_t items;
	while (getline(ss, current, delim)) {
		items.push_front(current);
	}
	return items;
}

Options cmdopts(int argc, const char **argv) {
	
	Options options;
	options.filename = argv[1];
	options.to_find.reserve(argc);
	
	for (int32_t i = 2; i < argc; i++) {
		string value(argv[i]);
		
		if (value == "-v" or value == "--verbose") {
			config()->VERBOSE = true;
			continue;
		}
		
		
		if (value == "-e") {
			if (i == argc - 1) {
				logger()->warning("Invalid option -e: no encodings list provided");
			}
			string encodings_raw(argv[++i]);
			options.encodings = split(encodings_raw, ENCODINGS_DELIMITER);
			continue;			
		}
		
		static const char ENCODINGS[] = "--encodings=";
		auto substr = value.substr(0, sizeof(ENCODINGS) - 1);
		if (substr == ENCODINGS) {
			string encodings_raw = value.substr(sizeof(ENCODINGS) - 1);
			options.encodings = split(encodings_raw, ENCODINGS_DELIMITER);
			continue;
		}
		
		options.to_find.push_back({byte_array_t((uint8_t*)argv[i], strlen(argv[i])), {}});
	}
	
	return options;
}
