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

enum OpType {
    OP_TYPE_UNKNOWN,
    OP_TYPE_APPEND_STRING,
    OP_TYPE_APPEND_PARAM,
    OP_TYPE_APPEND_PROPERTY_FROM_STRING,
    OP_TYPE_APPEND_PROPERTY_FROM_PARAM
};

static string escapeString(const string& str) {
    string result;
    for(char ch : str) {
        if(ch == '\r') continue;
        if(ch == '\n') {
            result += "\\n";
            continue;
        }
        if(ch == '\"'
        || ch == '\''
        || ch == '\\') {
            result += '\\';
        }
        result += ch;
    }
    return result;
}

struct RenderFlow_Operation {
    OpType type;
    string value1, value2;
    RenderFlow_Operation() {
        type = OP_TYPE_UNKNOWN;
    }
    RenderFlow_Operation(OpType _type, const string& _value1) {
        type = _type;
        value1 = _value1;
    }
    RenderFlow_Operation(OpType _type, const string& _value1, const string& _value2) {
        type = _type;
        value1 = _value1;
        value2 = _value2;
    }
};

class RenderFlow {
    public:
        list<RenderFlow_Operation> ops;

        void appendString(const string& s) {
            ops.push_back(RenderFlow_Operation(OP_TYPE_APPEND_STRING, s));
        }

        void appendParam(const string& k) {
            ops.push_back(RenderFlow_Operation(OP_TYPE_APPEND_PARAM, k));
        }

        void appendPropertyFromString(const string& propertyName, const string& s) {
            ops.push_back(RenderFlow_Operation(OP_TYPE_APPEND_PROPERTY_FROM_STRING, propertyName, s));
        }

        void appendPropertyFromParam(const string& propertyName, const string& k) {
            ops.push_back(RenderFlow_Operation(OP_TYPE_APPEND_PROPERTY_FROM_PARAM, propertyName, k));
        }

        void optimize() {
            list<RenderFlow_Operation> optimizedOps;
            bool wasAppendString = false;
            string prevString;

            ops.push_back(RenderFlow_Operation());

            for(auto& op : ops) {
                if(op.type == OP_TYPE_APPEND_PROPERTY_FROM_STRING) {
                    string buf;
                    buf = " ";
                    buf += op.value1;
                    buf += "=\"";
                    buf += escapeString(op.value2);
                    buf += "\"";
                    optimizedOps.push_back(RenderFlow_Operation(OP_TYPE_APPEND_STRING, buf));
                } else {
                    optimizedOps.push_back(op);
                }
            }

            ops = optimizedOps;
            optimizedOps.clear();
            
            for(auto& op : ops) {
                if(wasAppendString && op.type != OP_TYPE_APPEND_STRING) {
                    wasAppendString = false;
                    optimizedOps.push_back(RenderFlow_Operation(OP_TYPE_APPEND_STRING, prevString));
                    prevString = "";
                }
                if(op.type == OP_TYPE_APPEND_STRING) {
                    wasAppendString = true;
                    prevString += op.value1;
                } else {
                    optimizedOps.push_back(op);
                }
            }

            ops = optimizedOps;
            optimizedOps.clear();

            for(auto& op : ops) {
                if(op.type == OP_TYPE_UNKNOWN) {
                } else {
                    optimizedOps.push_back(op);
                }
            }

            ops = optimizedOps;
        }

        string generateHTML() {
            string ret;
            for(auto& op : ops) {
                switch(op.type) {
                    case OP_TYPE_APPEND_STRING:
                        ret += op.value1;
                        break;
                    case OP_TYPE_APPEND_PROPERTY_FROM_STRING:
                        ret += " ";
                        ret += op.value1;
                        ret += "=\"";
                        ret += escapeString(op.value2);
                        ret += "\"";
                        break;
                    case OP_TYPE_APPEND_PARAM:
                    case OP_TYPE_APPEND_PROPERTY_FROM_PARAM:
                    default:
                        break;
                }
            }
            return ret;
        }

        string generateScript() {
            string ret;
            ret = "function(p){var r=\"\";if(!p)p={};";
            for(auto& op : ops) {
                switch(op.type) {
                    case OP_TYPE_APPEND_STRING:
                        ret += "r+=\"";
                        ret += escapeString(op.value1);
                        ret += "\";";
                        break;
                    case OP_TYPE_APPEND_PARAM:
                        ret += "var v=p[\"";
                        ret += escapeString(op.value1);
                        ret += "\"];";
                        ret += "if(v)r+=v;";
                        break;
                    case OP_TYPE_APPEND_PROPERTY_FROM_STRING:
                        ret += "r+=\" ";
                        ret += escapeString(op.value1);
                        ret += "=\\\"";
                        ret += escapeString(op.value2);
                        ret += "\\\"\";";
                        break;
                    case OP_TYPE_APPEND_PROPERTY_FROM_PARAM:
                        ret += "var v=p[\"";
                        ret += escapeString(op.value2);
                        ret += "\"];";
                        ret += "if(v)r+=\" ";
                        ret += escapeString(op.value1);
                        ret += "=\\\"\";r+=v;r+=\"\\\"\";";
                        break;
                    default:
                        break;
                }
            }
            ret += "return r;}";
            return ret;
        }
};

std::string lastError;
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

/*static string escapeString(const char *str) {
    string s(str);
    return escapeString(s);
}*/

static void walkToFlow(Document& doc, int currentNodeId, RenderFlow& rf) {
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
        rf.appendString("<");
        rf.appendString(currentNode.key);

        for(auto& item : currentNode.properties) {
            if(!item.first.empty() && item.first[0] == '@') continue;
            if(item.second.type == fromString) {
                rf.appendPropertyFromString(item.first, item.second.ds);
            } else if(item.second.type == fromParam) {
                auto itr = doc.params.find(item.second.ds);
                if(itr == doc.params.end()) {
                    rf.appendPropertyFromParam(item.first, item.second.ds);
                } else {
                    rf.appendPropertyFromString(item.first, itr -> second);
                }
            }
        }

        if(isSingleTag) rf.appendString(" />");
        else rf.appendString(">");
    }

    if(!isSingleTag) {
        if(currentNode.content.type == fromString) {
            rf.appendString(currentNode.content.ds);
        } else if(currentNode.content.type == fromParam) {
            auto itr = doc.params.find(currentNode.content.ds);
            if(itr == doc.params.end()) {
                rf.appendParam(currentNode.content.ds);
            } else {
                rf.appendString(itr -> second);
            }
        }

        for(auto& i : currentNode.children) {
            walkToFlow(doc, i, rf);
        }
    }

    if(!isSingleTag && showTags) {
        rf.appendString("</");
        rf.appendString(currentNode.key);
        rf.appendString(">");
    }
}

extern "C" Document * loadDocument(const char *filename) {
    tryModuleInit();

    try {
        Document newDoc(filename);
        Document *newDocPtr = new Document(newDoc);
        lastError = "";
        newDoc.nodes = NULL; // prevent nodes from being deleted
        return newDocPtr;
    } catch(runtime_error& e) {
        lastError = e.what();
        return NULL;
    }
    return NULL;
}

extern "C" Document * loadDocumentFromSource(const char *src_c) {
    tryModuleInit();

    try {
        string src(src_c);
        Document newDoc(src);
        Document *newDocPtr = new Document(newDoc);
        lastError = "";
        newDoc.nodes = NULL; // prevent nodes from being deleted
        return newDocPtr;
    } catch(runtime_error& e) {
        lastError = e.what();
        return NULL;
    }
    return NULL;
}

extern "C" const char * getLastError() {
    return lastError.c_str();
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

    RenderFlow rf;
    walkToFlow(*doc, 0, rf);

    string result = rf.generateHTML();

    char *result_c = new char [result.size() + 1];
    result_c[result.size()] = 0;

    strcpy(result_c, result.c_str());

    return result_c;
}

extern "C" char * generateJavascriptRenderer(Document *doc, bool isWholePage) {
    if(doc == NULL) return NULL;

    RenderFlow rf;
    walkToFlow(*doc, 0, rf);
    rf.optimize();

    string result = rf.generateScript();

    char *result_c = new char [result.size() + 1];
    result_c[result.size()] = 0;

    strcpy(result_c, result.c_str());

    return result_c;
}
