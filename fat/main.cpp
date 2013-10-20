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
        
        cout << "Cluster size is " << (size_t)walker.cluster_size() << endl;
        
        cout << "Sector size is " << (size_t)walker.sector_size() << endl;
        cout << "Sectors per cluster " << (size_t)walker.sectors_per_cluster() << endl;
        
        cout << "Number of tables " << (size_t)walker.tables_count() << endl;
}