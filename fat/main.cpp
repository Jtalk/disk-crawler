#include "FATWalker.h"

#include <iostream>

#include <cinttypes>

const char *const USAGE = "Usage: fat DEVICE";

using namespace std;

int main(int argc, char **argv) {
        if (argc < 2) {
                cout << USAGE << endl;
                return 0;
        }
        
        string filename(argv[1]);
        
        FATWalker walker(filename);
        
        if (!walker) {
                cout << "Access denied" << endl;
                return -1;
        }
        
}