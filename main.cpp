#include "FATWalker.h"

#include "base/BaseDecoder.h"
#include "base/Log.h"

#include <iostream>

#include <cinttypes>
#include <cstring>

const char *const USAGE = "Usage: crawler DEVICE";

using namespace std;

Log *logger;

enum Status {
	SUCCESS = 0,
	ACCESS_DENIED = -1,
	NOT_FOUND = -2,
	NOT_ENOUGH_ARGUMENTS = -3,
};

void output(const FSWalker::results_t &results, size_t chunk_size)
{
	for (auto & result : results) {

		auto &reader = result.first;
		auto pos = result.second;

		reader->reset();
		reader->seekg(pos);
		Buffer buffer(chunk_size);
		reader->read(buffer, chunk_size);
		buffer.begin()[chunk_size] = 0;

		logger->info("Found: %s", (char*)buffer.begin());
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		cout << USAGE << endl;
		return NOT_ENOUGH_ARGUMENTS;
	}

	logger = new Log();
	
	string filename(argv[1]);

	FATWalker walker(filename);

	if (!walker) {
		logger->warning("Access denied");
		return ACCESS_DENIED;
	}

	auto && found = walker.find("whilst"_us);

	if (found.empty()) {
		logger->warning("Not found");
		return NOT_FOUND;
	}

	logger->debug("Found %u matches!", found.size());

	output(found, strlen((const char*)"whilst"_us));

	delete logger;
	
	return SUCCESS;
}
