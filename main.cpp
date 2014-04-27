#include "FATWalker.h"

#include "base/BaseDecoder.h"
#include "base/Log.h"

#include <iostream>

#include <cinttypes>
#include <cstring>

const char *const USAGE = "Usage: crawler DEVICE TO_FIND";

using namespace std;

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
		uint32_t counter = 0;
		
		for (auto pos : result.second) {
			reader->reset();
			reader->seekg(pos);
			Buffer buffer(chunk_size);
			reader->read(buffer, chunk_size);
			buffer.begin()[chunk_size] = 0;

			logger()->info("Found: %s at position %u", (char*)buffer.begin(), pos);
			
			++counter;
		}
		
		logger()->info("Found %u matches at current decoder", counter);
	}
}

FSWalker::results_t walk(const string &filename, const byte_array_t &to_find)
{
	FATWalker walker(filename);

	if (!walker) {
		logger()->warning("Access denied");
		exit(ACCESS_DENIED);
	}
	
	auto found = walker.find(to_find);
	
	return found;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		cout << USAGE << endl;
		return NOT_ENOUGH_ARGUMENTS;
	}

	Log *logger_object = new Log();
	
	string filename(argv[1]);
	
	auto length = strlen(argv[2]);
	byte_array_t to_find((uint8_t*)argv[2], length);
	
	auto && found = walk(filename, to_find);

	if (found.empty()) {
		logger()->warning("Not found");
		return NOT_FOUND;
	}

	logger()->debug("Found %u matched documents!", found.size());

	output(found, length);

	for (auto &result : found)
		delete result.first;
	
	delete logger_object;
	
	return SUCCESS;
}
