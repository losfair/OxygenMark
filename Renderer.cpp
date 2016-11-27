#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <exception>
#include <map>
#include <stdlib.h>
#include <string.h>
#include "Parser.h"

using namespace std;
using namespace OxygenMark;

map<string, bool> singleTags;
bool moduleInitialized = false;

static void tryModuleInit() {
    if(moduleInitialized) return;
    moduleInitialized = true;
    singleTags["br"] = true;
    singleTags["hr"] = true;
    singleTags["img"] = true;
    singleTags["input"] = true;
    singleTags["param"] = true;
    singleTags["meta"] = true;
    singleTags["link"] = true;
}

class RenderedDocument {
    public:
        string content;

        RenderedDocument& push(const string& s) {
            content += s;
            return *this;
        }
        RenderedDocument& push(const char *s_c) {
            string s(s_c);
            return push(s);
        }
        string& str() {
            return content;
        }
};

static void search(Document& doc, int currentNodeId, RenderedDocument& rl) {
    bool showTags = true;
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
        } else if(item.first == "@if") {
            if(item.second.type == fromParam) {
                auto itr = doc.params.find(item.second.ds);
                if(itr == doc.params.end()) return;
                if(itr -> second != "true") return;
            }
        }
    }

    if(currentNode.key == "" || currentNode.key == "_") showTags = false;

    bool isSingleTag = false;

    if(singleTags.find(currentNode.key) != singleTags.end()) isSingleTag = true;

    if(showTags) {
        rl.push("<").push(currentNode.key);
        for(auto& item : currentNode.properties) {
            if(!item.first.empty() && item.first[0] == '@') continue;
            if(item.second.type == fromString) {
                rl.push(" ").push(item.first).push("=\"").push(item.second.ds).push("\"");
            }
            else if(item.second.type == fromParam) {
                auto itr = doc.params.find(item.second.ds);
                if(itr == doc.params.end()) continue;
                rl.push(" ").push(item.first).push("=\"").push(itr -> second).push("\"");
            }
        }
        if(isSingleTag) rl.push(" />");
        else rl.push(">");
    }

    if(!isSingleTag) {
        if(currentNode.content.type == fromString) {
            rl.push(currentNode.content.ds);
        } else if(currentNode.content.type == fromParam) {
            auto itr = doc.params.find(currentNode.content.ds);
            if(itr != doc.params.end()) rl.push(itr -> second);
        }

        for(auto& i : currentNode.children) {
            search(doc, i, rl);
        }
    }

    if(!isSingleTag && showTags) rl.push("</").push(currentNode.key).push(">");
}

extern "C" Document * loadDocument(const char *filename) {
    tryModuleInit();

    try {
        Document *newDoc = new Document(filename);
        return newDoc;
    } catch(runtime_error e) {
        return NULL;
    }
    return NULL;
}

extern "C" Document * loadDocumentFromSource(const char *src_c) {
    tryModuleInit();

    try {
        string src(src_c);
        Document *newDoc = new Document(src);
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

extern "C" void clearDocumentParams(Document *doc) {
    if(doc == NULL) return;
    doc -> params.clear();
}

extern "C" char * renderToHtml(Document *doc, bool isWholePage) {
    if(doc == NULL) return NULL;

    RenderedDocument rl;
    if(isWholePage) rl.push("<!DOCTYPE html><html>");

    search(*doc, 0, rl);

    if(isWholePage) rl.push("</html>");

    string result = rl.str();

    char *result_c = new char [result.size() + 1];
    result_c[result.size()] = 0;

    strcpy(result_c, result.c_str());

    return result_c;
}
