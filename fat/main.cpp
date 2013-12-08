#include "FATWalker.h"
#include "FSFileStream.h"

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
        
        auto found = walker.find("XYZ"_us);      
        if (found.empty()) {
                cout << "Empty" << endl;
                return 0;
        }
        
        for (auto &iter : found)
        {
                uint8_t buffer[4] = {0};
                iter.first->seekg(iter.second);
                iter.first->read(buffer, 4);
                cout << buffer << endl;
        }
}