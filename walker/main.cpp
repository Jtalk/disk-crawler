#include "DiskWalker.h"

#include <iostream>

#include <cinttypes>

const char *const USAGE = "Usage: walker DEVICE [PHRASE]..";

using namespace std;

int main(int argc, char **argv) {
        if (argc < 3) {
                cout << USAGE << endl;
                return 0;
        }
        
        DiskWalker::byte_array_t phrase(reinterpret_cast<DiskWalker::byte_array_t::pointer>(argv[2]));
        string filename(argv[1]);
        
        DiskWalker walker(filename);
        
        if (!walker) {
                cout << "Access denied" << endl;
                return -1;
        }
        
        if (walker.find(phrase) != DiskWalker::byte_array_t::npos)
                cout << "Found" << endl;
        else
                cout << "Not found" << endl;
}
