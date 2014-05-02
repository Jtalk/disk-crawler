#include "FATWalker.h"

#include "base/BaseDecoder.h"
#include "base/Config.h"
#include "base/Log.h"
#include "base/PlainWalker.h"

#include "base/cmdopts.h"
#include "base/fspick.h"

#include <iostream>

#include <cinttypes>
#include <cstring>

static const char *const USAGE = "Usage: %s OPTIONS DEVICE TO_FIND\n\
OPTIONS:\n\
	\t-v|--verbose: verbose output\n\
";
static const size_t POSITION_OFFSET = 10;

using namespace std;

enum Status {
        SUCCESS = 0,
        ACCESS_DENIED = -1,
        NOT_FOUND = -2,
        NOT_ENOUGH_ARGUMENTS = -3,
};

void output(const FSWalker::results_t &results, size_t chunk_size) {
	for (auto & result : results) {

		auto &reader = result.first;
		uint32_t counter = 0;

		for (auto found_pos : result.second) {
			reader->reset();

			size_t pos;
			if (found_pos > POSITION_OFFSET) {
				pos = found_pos - POSITION_OFFSET;
			} else {
				pos = 0;
			}

			reader->seekg(pos);
			Buffer buffer(chunk_size + 2 * POSITION_OFFSET + 1);
			auto read = reader->read(buffer, chunk_size + 2 * POSITION_OFFSET);
			buffer.begin()[read] = 0;

			logger()->info("\n"
			               "========================= Found: =========================\n"
			               "%s\n"
			               "==========================================================\n"
			               "Position: %u", (char*)buffer.begin(), pos);

			++counter;
		}

		logger()->info("Found %u matches at current decoder", counter);
	}
}

Status walk(const string &filename, const byte_array_t &to_find, FSWalker::results_t &results) {
	FSWalker::walkers_t walkers;

	auto plain = new PlainWalker(filename);
	if (not plain->operator!()) {
		walkers->push_back(plain);
	} else {
		delete plain;
		logger()->warning("Access Denied at %s", filename.c_str());
		return ACCESS_DENIED;
	}

	walkers.splice(fspick(filename));

	for (auto walker : walkers) {
		auto && found = walker->find(to_find);
		if (not found.empty()) {
			results.splice(found);
		} else {
			delete walker;
		}
	}

	if (results.empty()) {
		logger()->warning("Not found");
		return NOT_FOUND;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf(USAGE, argv[0]);
		return NOT_ENOUGH_ARGUMENTS;
	}

	Config config_object;
	Log logger_object;

	Options opts = cmdopts(argc, (const char**)argv);

	FSWalker::results_t found;
	auto status = walk(opts.filename, opts.to_find, found);

	if (status != SUCCESS) {
		return status;
	}

	logger()->debug("Found %u matched documents!", found.size());

	output(found, opts.to_find.length());

	for (auto & result : found)
		delete result.first;

	return SUCCESS;
}
