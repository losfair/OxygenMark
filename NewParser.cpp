#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <exception>
#include <map>
#include "Tokenizer.h"
#include "Parser.h"

static const unsigned int magic1 = 0x1fd8a931;
static const unsigned int magic2 = 0xc92753be;
static const unsigned int magic3 = 0xd83a7c05;
static const unsigned int magic4 = 0xa79d12cf;

namespace OxygenMark {
    static int parseRow(Row& row, Node *nodes, int currentNodeId, int parentNodeId, std::map<int, int>& nodeIndentCache) {
        Node& currentNode = nodes[currentNodeId];
        std::string currentPropertyName, currentPropertyValue, currentString;

        std::list<Token>::iterator itr = row.tokens.begin();
        if(itr == row.tokens.end()) {
            throw std::runtime_error("Parsing failed: Unexpected end of row");
        }

        if(itr -> type != INDENT_TYPE) {
            throw std::runtime_error("Parsing failed: Indent expected");
        }

        currentNode.indent = itr -> indentValue;

        if(currentNodeId > 0 && nodes[currentNodeId - 1].indent < currentNode.indent) {
            parentNodeId = currentNodeId - 1;
        } else if(currentNodeId > 0 && nodes[currentNodeId - 1].indent > currentNode.indent) {
            auto mapItr = nodeIndentCache.find(currentNode.indent);
            if(mapItr == nodeIndentCache.end()) throw std::runtime_error("Parsing failed: Illegal indent");
            parentNodeId = nodes[mapItr -> second].parent;
        }

        nodes[parentNodeId].children.push_back(currentNodeId);
        nodes[currentNodeId].parent = parentNodeId;
        nodeIndentCache[currentNode.indent] = currentNodeId;

        itr++;
        if(itr == row.tokens.end()) {
            throw std::runtime_error("Parsing failed: Unexpected end of row");
        }

        if(itr -> type != IDENTIFIER_TYPE) {
            throw std::runtime_error("Parsing failed: Identifier expected");
        }

        currentNode.key = itr -> stringValue;

        while(1) {
            itr++;
            if(itr == row.tokens.end()) {
                break;
            }

            if(itr -> type == DELIMITER_TYPE && itr -> delimiterValue == KEY_DATASRC_DELIMITER) {
                itr++;
                if(itr == row.tokens.end()) {
                    throw std::runtime_error("Parsing failed: Unexpected end of row");
                }
                if(itr -> type == PARAM_TYPE) {
                    currentNode.content = DataSource(fromParam, itr -> stringValue);
                } else if(itr -> type == STRING_TYPE) {
                    currentNode.content = DataSource(fromString, itr -> stringValue);
                } else {
                    throw std::runtime_error("Parsing failed: Param or string expected");
                }
                break;
            }

            if(itr -> type == IDENTIFIER_TYPE && itr -> stringValue[0] == '#') {
                currentPropertyValue = itr -> stringValue.substr(1);
                currentNode.properties["id"] = DataSource(fromString, currentPropertyValue);
                continue;
            }

            if(itr -> type == IDENTIFIER_TYPE && itr -> stringValue[0] == '.') {
                currentPropertyValue = itr -> stringValue.substr(1);
                auto propItr = currentNode.properties.find("class");
                if(propItr == currentNode.properties.end()) {
                    currentNode.properties["class"] = DataSource(fromString, currentPropertyValue);
                } else {
                    if(propItr -> second.type != fromString) {
                        throw std::runtime_error("A non-string type has been assigned to the 'class' property.");
                    }
                    propItr -> second.ds += " ";
                    propItr -> second.ds += currentPropertyValue;
                }
                continue;
            }

            if(itr -> type == IDENTIFIER_TYPE) {
                currentPropertyName = itr -> stringValue;
                itr++;
                if(itr == row.tokens.end()) {
                    throw std::runtime_error("Parsing failed: Unexpected end of row");
                }
                if(itr -> type != DELIMITER_TYPE || itr -> delimiterValue != PROP_DATASRC_DELIMITER) {
                    throw std::runtime_error("Parsing failed: Unexpected token. A '=' delimiter is expected.");
                }
                itr++;
                if(itr -> type == PARAM_TYPE) {
                    currentNode.properties[currentPropertyName] = DataSource(fromParam, itr -> stringValue);
                } else if(itr -> type == STRING_TYPE) {
                    currentNode.properties[currentPropertyName] = DataSource(fromString, itr -> stringValue);
                } else {
                    throw std::runtime_error("Parsing failed: Unexpected rvalue type");
                }
            }
        }

        return parentNodeId;
    }

    DataSource::DataSource() {
        type = unknownDataSourceType;
    }

    DataSource::DataSource(DataSourceType newType, std::string& newValue) {
        type = newType;
        ds = newValue;
    }

    Node::Node() {
        indent = -1;
        parent = -1;
    }

    Document::Document(std::string& doc) {
        nodes = NULL;

        int pos = 1, parentNodeId = 0;

        Tokenizer docTokenizer(doc);

        rowCount = docTokenizer.rows.size();
        if(rowCount == 0) throw std::runtime_error("No rows");

        nodes = new Node [rowCount + 1];

        nodes[0].key = "";

        std::map<int, int> nodeIndentCache;

        for(auto& row : docTokenizer.rows) {
            parentNodeId = parseRow(row, nodes, pos, parentNodeId, nodeIndentCache);
            pos++;
        }
    }

    Document::Document(const char *filename) {
        nodes = NULL;

        std::ifstream inFile(filename, std::ios::binary);
        if(!inFile.is_open()) {
            throw std::runtime_error("Unable to open input file");
        }

        unsigned int uintReadbuf;
        int intReadbuf;
        
        inFile.read((char *) &uintReadbuf, sizeof(unsigned int));
        if(uintReadbuf != magic1) throw std::runtime_error("Bad file type");
        
        inFile.read((char *) &uintReadbuf, sizeof(unsigned int));
        if(uintReadbuf != magic2) throw std::runtime_error("Bad file type");

        int nodeCount;
        inFile.read((char *) &nodeCount, sizeof(int));

        if(nodeCount < 1) throw std::runtime_error("Incorrect node count");

        rowCount = nodeCount - 1;
        
        nodes = new Node [nodeCount];

        for(int i = 0; i < nodeCount; i++) {
            Node& currentNode = nodes[i];

            int keySize, propertyCount, contentSize, parentNodeId, childrenCount;
            char *strBuf;

            inFile.read((char *) &keySize, sizeof(int));
            if(keySize < 0 || keySize >= 256) throw std::runtime_error("Incorrect key size");

            strBuf = new char [keySize + 1];

            inFile.read(strBuf, keySize);
            strBuf[keySize] = 0;

            currentNode.key = strBuf;
            delete[] strBuf;

            inFile.read((char *) &propertyCount, sizeof(int));
            if(propertyCount < 0) throw std::runtime_error("Incorrect property count");

            for(int j = 0; j < propertyCount; j++) {
                inFile.read((char *) &intReadbuf, sizeof(int)); // current property name size
                strBuf = new char [intReadbuf + 1];
                strBuf[intReadbuf] = 0;
                inFile.read(strBuf, intReadbuf);
                std::string currentPropKey = strBuf;
                delete[] strBuf;

                DataSource currentDs;

                inFile.read((char *) &intReadbuf, sizeof(int)); // current property value type
                currentDs.type = (DataSourceType) intReadbuf;

                inFile.read((char *) &intReadbuf, sizeof(int)); // current property value size
                strBuf = new char [intReadbuf + 1];
                strBuf[intReadbuf] = 0;

                inFile.read(strBuf, intReadbuf);
                currentDs.ds = strBuf;
                delete[] strBuf;

                currentNode.properties[currentPropKey] = currentDs;
            }

            inFile.read((char *) &intReadbuf, sizeof(int)); // current content type
            currentNode.content.type = (DataSourceType) intReadbuf;

            inFile.read((char *) &contentSize, sizeof(int));
            strBuf = new char [contentSize + 1];
            strBuf[contentSize] = 0;
            
            inFile.read(strBuf, contentSize);

            currentNode.content.ds = strBuf;

            delete[] strBuf;

            inFile.read((char *) &parentNodeId, sizeof(int));
            currentNode.parent = parentNodeId;
            
            inFile.read((char *) &childrenCount, sizeof(int));
            for(int j = 0; j < childrenCount; j++) {
                inFile.read((char *) &intReadbuf, sizeof(int));
                currentNode.children.push_back(intReadbuf);
            }
        }

        inFile.close();
    }

    Document::Document(Document& from) noexcept {
        //std::cout << "[DEBUG] Copying Document object" << std::endl;
        rowCount = from.rowCount;
        nodes = from.nodes;
        params = from.params;
    }

    Document::~Document() {
        if(nodes) delete[] nodes;
    }

    void Document::dumpToFile(const char *filename) {
        std::ofstream outFile(filename, std::ios::binary);
        if(!outFile.is_open()) {
            throw std::runtime_error("Unable to open output file");
        }

        outFile.write((const char *) &magic1, sizeof(unsigned int));
        outFile.write((const char *) &magic2, sizeof(unsigned int));

        int nodeCount = rowCount + 1;
        outFile.write((const char *) &nodeCount, sizeof(int));

        for(int i = 0; i < nodeCount; i++) {
            Node& currentNode = nodes[i];

            int keySize = currentNode.key.size();
            int propertyCount = currentNode.properties.size();
            int contentSize = currentNode.content.ds.size();
            int parentNodeId = currentNode.parent;
            int childrenCount = currentNode.children.size();

            outFile.write((const char *) &keySize, sizeof(int));
            outFile.write((const char *) currentNode.key.c_str(), keySize);

            outFile.write((const char *) &propertyCount, sizeof(int));
            for(auto& prop : currentNode.properties) {
                int currentPropNameSize = prop.first.size();
                int currentPropValueType = prop.second.type;
                int currentPropValueSize = prop.second.ds.size();
                outFile.write((const char *) &currentPropNameSize, sizeof(int));
                outFile.write(prop.first.c_str(), currentPropNameSize);
                outFile.write((const char *) &currentPropValueType, sizeof(int));
                outFile.write((const char *) &currentPropValueSize, sizeof(int));
                outFile.write(prop.second.ds.c_str(), currentPropValueSize);
            }

            outFile.write((const char *) &currentNode.content.type, sizeof(int));
            outFile.write((const char *) &contentSize, sizeof(int));
            outFile.write(currentNode.content.ds.c_str(), contentSize);

            outFile.write((const char *) &parentNodeId, sizeof(int));

            outFile.write((const char *) &childrenCount, sizeof(int));
            for(auto& childId : currentNode.children) {
                outFile.write((const char *) &childId, sizeof(int));
            }
        }

        outFile.write((const char *) &magic3, sizeof(unsigned int));
        outFile.write((const char *) &magic4, sizeof(unsigned int));

        outFile.close();
    }
}
