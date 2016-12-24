#include "Tokenizer.h"
#include <string>
#include <list>
#include <exception>
#include <stdexcept>

static const char DEFAULT_INDENT_CHAR = '\t';
static const char *DEFAULT_IGNORE_CHARS = "\t\r ";
static std::string KEY_DATASRC_DELIMITER = "=>";

static int countIndent(std::string& row, char indentChar) {
    int rowLength = row.size();

    size_t pos = row.find_first_not_of(indentChar);
    if(pos != std::string::npos) return pos;

    return rowLength;
}

static int countIndent(std::string& row) {
    return countIndent(row, DEFAULT_INDENT_CHAR);
}

static void splitToRows(std::string doc, std::list<std::string>& result) {
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
    currentRow = doc.substr(prevPos);
    if(currentRow.find_first_not_of(DEFAULT_IGNORE_CHARS) != std::string::npos) result.push_back(currentRow);
}

static inline bool charIsInRange(char ch, char start, char end) {
    if(ch >= start && ch <= end) return true;
    return false;
}

static inline bool isValidIdentifierChar(char ch) {
    if(charIsInRange(ch, 'a', 'z')
    || charIsInRange(ch, 'A', 'Z')
    || charIsInRange(ch, '0', '9')
    || ch == '-'
    || ch == '_'
    || ch == ':'
    || ch == '.'
    || ch == '#'
    || ch == '@') {
        return true;
    }
    return false;
}

static inline bool isSpaceLikeChar(char ch) {
    if(ch == ' '
    || ch == '\t'
    || ch == '\r') {
        return true;
    }
    return false;
}

namespace OxygenMark {
    Token::Token() {
        type = UNKNOWN_TOKEN_TYPE;
    }

    Token Token::createFromIndent(int indent) {
        Token newToken;
        newToken.type = INDENT_TYPE;
        newToken.indentValue = indent;
        return newToken;
    }

    Token Token::createFromString(std::string s) {
        Token newToken;
        newToken.type = STRING_TYPE;
        newToken.stringValue = s;
        return newToken;
    }

    Token Token::createFromDelimiter(Delimiter d) {
        Token newToken;
        newToken.type = DELIMITER_TYPE;
        newToken.delimiterValue = d;
        return newToken;
    }

    Token Token::createFromParam(std::string p) {
        Token newToken;
        newToken.type = PARAM_TYPE;
        newToken.stringValue = p;
        return newToken;
    }

    Token Token::createFromIdentifier(std::string s) {
        Token newToken;
        newToken.type = IDENTIFIER_TYPE;
        newToken.stringValue = s;
        return newToken;
    }

    Tokenizer::Tokenizer(std::string& doc) {
        std::list<std::string> rawRows;
        splitToRows(doc, rawRows);

        for(auto& currentRawRow : rawRows) {
            Row newRow;

            int currentIndent = countIndent(currentRawRow);
            newRow.tokens.push_back(Token::createFromIndent(currentIndent));

            currentRawRow = currentRawRow.substr(currentIndent);
            const char *currentRawRowC = currentRawRow.c_str();
            int currentRawRowSize = currentRawRow.size();
            int currentPos = 0;
            char charBuf[2] = {0, 0};

            while(currentPos < currentRawRowSize) {
                if(isValidIdentifierChar(currentRawRowC[currentPos])) { // identifier
                    std::string currentIdentifier;
                    while(currentPos < currentRawRowSize && isValidIdentifierChar(currentRawRowC[currentPos])) {
                        charBuf[0] = currentRawRowC[currentPos];
                        currentIdentifier += charBuf;
                        currentPos++;
                    }
                    newRow.tokens.push_back(Token::createFromIdentifier(currentIdentifier));
                } else if(currentRawRowC[currentPos] == '\"') { // string
                    std::string currentString;
                    currentPos++;
                    while(currentPos < currentRawRowSize && currentRawRowC[currentPos] != '\"') {
                        charBuf[0] = currentRawRowC[currentPos];
                        currentString += charBuf;
                        currentPos++;
                    }
                    if(currentPos > currentRawRowSize) {
                        throw std::runtime_error("Tokenizing failed: Cannot find the end of string");
                    }
                    currentPos++;
                    newRow.tokens.push_back(Token::createFromString(currentString));
                } else if(currentRawRowC[currentPos] == '[') { // param
                    std::string currentParam;
                    currentPos++;
                    while(currentPos < currentRawRowSize && currentRawRowC[currentPos] != ']') {
                        charBuf[0] = currentRawRowC[currentPos];
                        currentParam += charBuf;
                        currentPos++;
                    }
                    if(currentPos > currentRawRowSize) {
                        throw std::runtime_error("Tokenizing failed: Cannot find the end of param");
                    }
                    currentPos++;
                    newRow.tokens.push_back(Token::createFromParam(currentParam));
                } else if(isSpaceLikeChar(currentRawRow[currentPos])) { // space
                    currentPos++;
                    continue;
                } else if(currentRawRowC[currentPos] == '=') {
                    currentPos++;
                    if(currentPos == currentRawRowSize) {
                        throw std::runtime_error("Tokenizing failed: rvalue expected");
                    }
                    if(currentRawRowC[currentPos] == '>') {
                        currentPos++;
                        newRow.tokens.push_back(Token::createFromDelimiter(KEY_DATASRC_DELIMITER));
                    } else {
                        newRow.tokens.push_back(Token::createFromDelimiter(PROP_DATASRC_DELIMITER));
                    }
                } else { // unknown
                    charBuf[0] = currentRawRowC[currentPos];
                    throw std::runtime_error((std::string) "Tokenizing failed: Unknown token: " + (std::string) charBuf);
                }
            }

            rows.push_back(newRow);
        }
    }
}