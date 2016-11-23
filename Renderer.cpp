#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include "Parser.h"

using namespace std;
using namespace SimpleMark;

void search(Document& doc, int currentNodeId) {
    Node& currentNode = doc.nodes[currentNodeId];

    cout << "<" << currentNode.key;
    for(auto& item : currentNode.properties) {
        if(item.second.type == fromString) cout << " " << item.first << "=\"" << item.second.ds << "\"";
    }
    cout << ">";

    if(currentNode.content.type == fromString) {
        cout << currentNode.content.ds;
    }
    for(auto& i : currentNode.children) {
        search(doc, i);
    }
    cout << "</" << currentNode.key << ">";
}

void renderToHtml(const char *filename) {
    Document doc(filename);
    search(doc, 0);
    cout << endl;
}

int main(int argc, char *argv[]) {
    if(argc != 2) return 1;

    try {
        renderToHtml(argv[1]);
    } catch(runtime_error e) {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}