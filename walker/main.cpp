#include "DiskWalker.h"

#include "types.h"

#include <iostream>

#include <cinttypes>

const char *const USAGE = "Usage: walker DEVICE [PHRASE]..";

using namespace std;

int main(int argc, char **argv) {
        if (argc < 3) {
                cout << USAGE << endl;
                return 0;
        }
        
        auto phrase(reinterpret_cast<byte_array_t::pointer>(argv[2]));
        string filename(argv[1]);
        
        DiskWalker walker(filename);
        
        if (!walker) {
                cout << "Access denied" << endl;
                return -1;
        }
        
        if (walker.find(phrase) != byte_array_t::npos)
                cout << "Found" << endl;
        else
                cout << "Not found" << endl;
}
