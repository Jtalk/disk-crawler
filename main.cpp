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

#include "base/BaseDecoder.h"
#include "base/Config.h"
#include "base/FSWalker.h"
#include "base/Log.h"
#include "base/PlainWalker.h"

#include "base/cmdopts.h"
#include "base/fspick.h"
#include "base/utility.h"

#include <iostream>

#include <cinttypes>
#include <clocale>
#include <cstring>

static const char *const USAGE = "Usage: %s OPTIONS DEVICE TO_FIND\n\
OPTIONS:\n\
	\t-v|--verbose: verbose output\n\
	\t-e|--encodings: encodings to convert string to [system locale encoding is default]\n\
	\t\tExample: --encodings='utf8,cp1251'\n\
";
static const size_t POSITION_OFFSET = 10;
static const char ENCODING_MISMATCH[] = ">>>Preview disabled because of encoding mismatch<<<";

using namespace std;

enum Status {
        SUCCESS = 0,
        ACCESS_DENIED = -1,
        NOT_FOUND = -2,
        NOT_ENOUGH_ARGUMENTS = -3,
};

void output(const SignatureWalker::results_t &results, const search_terms_t &to_find, size_t chunk_size) {
	for (auto & result : results) {

		auto &reader = result.first;
		uint32_t counter = 0;

		for (auto found : result.second) {
			reader->reset();

			size_t pos;
			if (found.offset > POSITION_OFFSET) {
				pos = found.offset - POSITION_OFFSET;
			} else {
				pos = 0;
			}

			reader->seekg(pos);
			Buffer buffer(chunk_size + 2 * POSITION_OFFSET + 1);
			auto read = reader->read(buffer, chunk_size + 2 * POSITION_OFFSET);
			
			logger()->debug("Read %u bytes for %u pos in output", read, pos);
			
			buffer.resize(buffer.size() + 1);
			buffer.begin()[read] = 0;

			bool encoded = utility::sanitize(buffer, to_find[found.pattern_n].encoding);
			if (not encoded) {
				logger()->debug("Encodings for pattern %u mismatches: %s for current and %s for environment", 
					found.pattern_n, to_find[found.pattern_n].encoding.c_str(), to_find.crbegin()->encoding.c_str());
				buffer.clear();
				buffer.capture((uint8_t*)ENCODING_MISMATCH, sizeof(ENCODING_MISMATCH));
			}
			
			Buffer pattern(to_find[found.pattern_n].pattern.size() + 1);
			pattern.capture(to_find[found.pattern_n].pattern.c_str(), to_find[found.pattern_n].pattern.size() + 1);
			encoded = utility::sanitize(pattern, to_find[found.pattern_n].encoding);
			if (not encoded) {
				logger()->debug("Encodings for pattern %u mismatches: %s for current and %s for environment", 
					found.pattern_n, to_find[found.pattern_n].encoding.c_str(), to_find.crbegin()->encoding.c_str());
				pattern.clear();
				pattern.capture((uint8_t*)ENCODING_MISMATCH, sizeof(ENCODING_MISMATCH));				
			}

			logger()->info("\n"
			               "========================= Found: =========================\n"
			               "%s\n"
			               "==========================================================\n"
			               "Position: %u, signature %s", (const char*)buffer.cbegin(), found.offset, (const char*)pattern.cbegin());

			++counter;
		}

		logger()->info("Found %u matches at current decoder", counter);
	}
}

Status walk(Options &opts, SignatureWalker::results_t &results) {
	SignatureWalker::walkers_t walkers;

	utility::encode(opts);
	
	auto plain = new PlainWalker(opts.filename);
	if (not plain->operator!()) {
		walkers.push_back(plain);
	} else {
		delete plain;
		logger()->warning("Access Denied at %s", opts.filename.c_str());
		return ACCESS_DENIED;
	}

	walkers.splice(walkers.end(), fspick(opts.filename));

	for (auto walker : walkers) {
		auto && found = walker->find(opts.to_find);
		if (not found.empty()) {
			SignatureWalker::merge(results, found);
		} else {
			delete walker;
		}
	}

	if (results.empty()) {
		logger()->warning("Not found");
		return NOT_FOUND;
	} else {
		return SUCCESS;
	}
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, ""); // Setting global OS locale as default
	
	if (argc < 3) {
		printf(USAGE, argv[0]);
		return NOT_ENOUGH_ARGUMENTS;
	}

	Config config_object;
	Log logger_object;

	Options opts = cmdopts(argc, (const char**)argv);

	SignatureWalker::results_t found;
	auto status = walk(opts, found);

	if (status != SUCCESS) {
		return status;
	}

	logger()->debug("Found %u matched documents!", found.size());

	output(found, opts.to_find, utility::overlap_size(opts.to_find));

	for (auto & result : found)
		delete result.first;

	return SUCCESS;
}
