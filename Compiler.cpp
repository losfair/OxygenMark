#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include "Parser.h"

using namespace std;
using namespace SimpleMark;

void compileAndDump(string origData, const char *filename) {
    Document doc(origData);
    doc.dumpToFile(filename);
    cout << "Done. " << endl;
}

int main(int argc, char *argv[]) {
    if(argc != 3) return 1;

    ifstream inFile(argv[1], ios::in | ios::binary);
    if(!inFile.is_open()) return 2;

    char fileData[16385];
    inFile.read(fileData, 16384);

    try {
        compileAndDump((string) fileData, argv[2]);
    } catch(runtime_error e) {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}