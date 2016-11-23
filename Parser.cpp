#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <exception>
#include <map>
#include "Parser.h"

static const unsigned int magic1 = 0x1fd8a931;
static const unsigned int magic2 = 0xc92753be;
static const unsigned int magic3 = 0xd83a7c05;
static const unsigned int magic4 = 0xa79d12cf;

static const char DEFAULT_INDENT_CHAR = '\t';
static const char *DEFAULT_IGNORE_CHARS = "\t\r ";
static std::string KEY_DATASRC_DELIMITER = "=>";

static int countIndent(std::string& row, char indentChar) {
    int i = 0;
    int rowLength = row.size();

    size_t pos = row.find_first_not_of(indentChar);
    if(pos != std::string::npos) return pos;

    return rowLength;
}

static int countIndent(std::string& row) {
    return countIndent(row, DEFAULT_INDENT_CHAR);
}

static void splitToRows(std::string doc, std::vector<std::string>& result) {
    size_t prevPos = 0;
    size_t currentPos = 0;
    std::string currentRow;

    currentPos = doc.find('\n');
    while(currentPos != std::string::npos) {
        currentRow = doc.substr(prevPos, currentPos - prevPos);
        if(currentRow.find_first_not_of(DEFAULT_IGNORE_CHARS) != std::string::npos) result.push_back(currentRow);
        prevPos = currentPos + 1;
        currentPos = doc.find('\n', currentPos + 1);
    }
}

namespace OxygenMark {
    static int parseRow(std::string& rawRow, Node *nodes, int currentNodeId, int parentNodeId, std::map<int, int>& nodeIndentCache) {
        Node& currentNode = nodes[currentNodeId];
        std::stringstream ss;
        std::string currentToken;
        std::string currentPropertyName, currentPropertyValue, currentString;
        int a;
        size_t pos, beginPos, endPos;
        bool shouldContinue, isFirst;
        int currentIndent = countIndent(rawRow);
        std::map<int, int>::iterator mapItr;

        std::string row = rawRow.substr(currentIndent);
        currentNode.indent = currentIndent;

        if(currentNodeId > 0 && nodes[currentNodeId - 1].indent < currentNode.indent) {
            parentNodeId = currentNodeId - 1;
        } else if(currentNodeId > 0 && nodes[currentNodeId - 1].indent > currentNode.indent) {
            mapItr = nodeIndentCache.find(currentNode.indent);
            if(mapItr == nodeIndentCache.end()) throw std::runtime_error("Illegal syntax");
            parentNodeId = nodes[mapItr -> second].parent;
        }

        nodes[parentNodeId].children.push_back(currentNodeId);
        nodes[currentNodeId].parent = parentNodeId;
        nodeIndentCache[currentNode.indent] = currentNodeId;

        ss.str(row);
        ss >> currentNode.key;

        while(!ss.eof()) {
            ss >> currentToken;
            if(currentToken.empty()) break;

            if(currentToken == "=>") {
                ss >> currentToken;
                if(currentToken.empty()) throw std::runtime_error("Parse error");
                if(currentToken == "param") {
                    ss >> currentToken;
                    if(currentToken.empty()) throw std::runtime_error("Parse error");
                    currentNode.content = DataSource(fromParam, currentToken);
                } else if(currentToken == "string") {
                    std::getline(ss, currentToken);
                    if(currentToken.empty()) throw std::runtime_error("Parse error");

                    beginPos = currentToken.find('\"');
                    if(beginPos == std::string::npos) throw std::runtime_error("Parse error");

                    beginPos++;

                    endPos = currentToken.find('\"', beginPos);
                    if(endPos == std::string::npos) throw std::runtime_error("Parse error");

                    currentToken = currentToken.substr(beginPos, endPos - beginPos);

                    currentNode.content = DataSource(fromString, currentToken);
                } else if(currentToken == "file") {
                    ss >> currentToken;
                    if(currentToken.empty()) throw std::runtime_error("Parse error");
                    currentNode.content = DataSource(fromFile, currentToken);
                } else {
                    throw std::runtime_error("Parse error");
                }
                break;
            }

            pos = currentToken.find("=");
            if(pos == std::string::npos) throw std::runtime_error("Parse error");

            currentPropertyName = currentToken.substr(0, pos);
            currentPropertyValue = currentToken.substr(pos + 1);

            if(currentPropertyName.empty() || currentPropertyValue.empty()) throw std::runtime_error("Parse error");

            if(currentPropertyValue == "param") {
                ss >> currentToken;
                currentNode.properties[currentPropertyName] = DataSource(fromParam, currentToken);
            } else if(currentPropertyValue == "file") {
                ss >> currentToken;
                currentNode.properties[currentPropertyName] = DataSource(fromFile, currentToken);
            } else if(currentPropertyValue == "string") {
                currentString = "";
                ss >> currentToken;
                if(currentToken.empty() || currentToken[0] != '\"') throw std::runtime_error("Parse error");
                currentToken = currentToken.substr(1);
                shouldContinue = true;
                isFirst = true;
                while(shouldContinue) {
                    if(currentToken.empty()) throw std::runtime_error("Parse error");
                    if(currentToken[currentToken.size() - 1] == '\"') {
                        currentToken = currentToken.substr(0, currentToken.size() - 1);
                        shouldContinue = false;
                    }
                    if(!isFirst) currentString += " ";
                    currentString += currentToken;
                    isFirst = false;
                    if(shouldContinue) ss >> currentToken;
                }
                currentNode.properties[currentPropertyName] = DataSource(fromString, currentString);
            } else {
                throw std::runtime_error("Parse error");
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

    Token::Token() {
        type = unknownTokenType;
    }

    Document::Document(std::string& doc) {
        int pos = 1, parentNodeId = 0;

        std::vector<std::string> rows;
        splitToRows(doc, rows);

        rowCount = rows.size();
        if(rowCount == 0) throw std::runtime_error("No rows");

        nodes = new Node [rowCount + 1];

        nodes[0].key = "html";

        std::map<int, int> nodeIndentCache;

        for(auto& row : rows) {
            parentNodeId = parseRow(row, nodes, pos, parentNodeId, nodeIndentCache);
            pos++;
        }
    }

    Document::Document(const char *filename) {
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

        if(nodeCount <= 0) throw std::runtime_error("Incorrect node count");
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
