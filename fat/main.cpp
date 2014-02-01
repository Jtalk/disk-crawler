#include "FATWalker.h"
#include "FSFileStream.h"

#include <iostream>

#include <cinttypes>

const char *const USAGE = "Usage: fat DEVICE";

using namespace std;

enum Status {
	SUCCESS = 0,
	ACCESS_DENIED = -1,
	NOT_FOUND = -2,
	NOT_ENOUGH_ARGUMENTS = -3,
};

void output(const FSWalker::results_t &results, size_t chunk_size)
{
	uint8_t *buffer = (uint8_t*)alloca(chunk_size);

	for (auto & result : results) {

		auto &reader = result.first;
		auto pos = result.second;

		reader->seekg(pos);
		reader->read(buffer, chunk_size);
		
		cout << buffer << endl;
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		cout << USAGE << endl;
		return NOT_ENOUGH_ARGUMENTS;
	}

	string filename(argv[1]);

	FATWalker walker(filename);

	if (!walker) {
		cout << "Access denied" << endl;
		return ACCESS_DENIED;
	}

	auto found = walker.find("XYZ"_us);

	if (found.empty()) {
		cout << "Not found" << endl;
		return NOT_FOUND;
	}

	output(found, sizeof("XYZ"_us));

	return SUCCESS;
}
