#include "FATWalker.h"

#include "base/BaseDecoder.h"

#include "test/Tests.h"

#include <iostream>

#include <cinttypes>
#include <cstring>

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
	for (auto & result : results) {

		auto &reader = result.first;
		auto pos = result.second;

		reader->seekg(pos);
		Buffer buffer(chunk_size);
		reader->read(buffer, chunk_size);
		buffer.begin()[chunk_size] = 0;
		
		cout << (char*)buffer.begin() << endl;
	}
}

int main(int argc, char **argv)
{
	run_tests();
	
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

	auto found = walker.find("meanwhile"_us);

	if (found.empty()) {
		cout << "Not found" << endl;
		return NOT_FOUND;
	}
	
	cout << "Found " << found.size() << " matches!" << endl;

	output(found, strlen((const char*)"meanwhileeee"_us));

	return SUCCESS;
}
