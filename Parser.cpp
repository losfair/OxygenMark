#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <exception>
#include <map>
#include "Parser.h"

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

namespace SimpleMark {
    static int parseRow(std::string& rawRow, Node *nodes, int currentNodeId, int parentNodeId, std::map<int, int>& nodeIndentCache) {
        Node& currentNode = nodes[currentNodeId];
        std::stringstream ss;
        std::string currentToken;
        int a;
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

        nodes[0].key = "root";

        std::map<int, int> nodeIndentCache;

        for(auto& row : rows) {
            parentNodeId = parseRow(row, nodes, pos, parentNodeId, nodeIndentCache);
            pos++;
        }
    }

    Document::~Document() {
        if(nodes) delete[] nodes;
    }
}
