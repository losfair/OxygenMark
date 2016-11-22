#include <iostream>
#include <fstream>
#include <string>
#include "Parser.h"

using namespace std;
using namespace SimpleMark;

void search(Document& doc, int currentNodeId) {
    Node& currentNode = doc.nodes[currentNodeId];
    cout << "<" << currentNode.key << ">";
    if(currentNode.content.type == fromString) {
        cout << currentNode.content.ds;
    }
    for(auto& i : currentNode.children) {
        search(doc, i);
    }
    cout << "</" << currentNode.key << ">";
}

int main(int argc, char *argv[]) {
    if(argc != 2) return 1;

    ifstream inFile(argv[1], ios::in | ios::binary);
    if(!inFile.is_open()) return 2;

    char fileData[16385];
    inFile.read(fileData, 16384);

    string fileDataStr = fileData;

    Document doc(fileDataStr);

    search(doc, 0);
    cout << endl;

    return 0;
}