#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <exception>
#include <stdlib.h>
#include <string.h>
#include "Parser.h"

using namespace std;
using namespace OxygenMark;

static void search(Document& doc, int currentNodeId, stringstream& ss) {
    Node& currentNode = doc.nodes[currentNodeId];

    for(auto& item : currentNode.properties) {
        if(item.first == "@invisible") {
            if(item.second.type == fromString && item.second.ds == "true") return;
            if(item.second.type == fromParam) {
                auto itr = doc.params.find(item.second.ds);
                if(itr != doc.params.end()) {
                    if(itr -> second == "true") return;
                }
            }
        }
    }

    ss << "<" << currentNode.key;
    for(auto& item : currentNode.properties) {
        if(item.second.type == fromString) ss << " " << item.first << "=\"" << item.second.ds << "\"";
        else if(item.second.type == fromParam) {
            auto itr = doc.params.find(item.second.ds);
            if(itr == doc.params.end()) continue;
            ss << " " << item.first << "=\"" << itr -> second << "\"";
        }
    }
    ss << ">";

    if(currentNode.content.type == fromString) {
        ss << currentNode.content.ds;
    } else if(currentNode.content.type == fromParam) {
        auto itr = doc.params.find(currentNode.content.ds);
        if(itr != doc.params.end()) ss << itr -> second;
    }

    for(auto& i : currentNode.children) {
        search(doc, i, ss);
    }
    ss << "</" << currentNode.key << ">";
}

extern "C" Document * loadDocument(const char *filename) {
    try {
        Document *newDoc = new Document(filename);
        return newDoc;
    } catch(runtime_error e) {
        return NULL;
    }
    return NULL;
}

extern "C" void destroyDocument(Document *doc) {
    if(doc == NULL) return;
    delete doc;
}

extern "C" void setDocumentParam(Document *doc, const char *key, const char *value) {
    if(doc == NULL) return;
    doc -> params[(string) key] = (string) value;
}

extern "C" char * renderToHtml(Document *doc) {
    if(doc == NULL) return NULL;

    stringstream ss;
    search(*doc, 0, ss);
    string result = ss.str();

    char *result_c = new char [result.size() + 1];
    result_c[result.size()] = 0;

    strcpy(result_c, result.c_str());

    return result_c;
}
