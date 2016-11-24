#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include "Parser.h"

using namespace std;
using namespace OxygenMark;

void compileAndDump(string origData, const char *filename) {
    Document doc(origData);
    doc.dumpToFile(filename);
    cout << "Done. " << endl;
}

int main(int argc, char *argv[]) {
    if(argc != 3) return 1;

    ifstream inFile(argv[1], ios::in | ios::binary);
    if(!inFile.is_open()) return 2;

    char *fileData = new char [1048577];
    inFile.read(fileData, 1048576);

    fileData[1048576] = 0;

    try {
        compileAndDump((string) fileData, argv[2]);
    } catch(runtime_error e) {
        cout << "Error: " << e.what() << endl;
    }

    delete[] fileData;

    return 0;
}