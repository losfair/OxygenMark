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

static inline bool isIdentifierChar(char ch) {
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
        std::list<std::string> rows;
        splitToRows(doc, rows);

        for(auto& row : rows) {
            row += " ";

            Row rowInfo;
            int indent = countIndent(row);
            rowInfo.tokens.push_back(Token::createFromIndent(indent));

            row = row.substr(indent);

            if(!row.size() || row[0] == '#') {
                continue;
            }

            bool inIdentifier = false;
            bool inString = false;
            bool isEscaped = false;
            bool isParam = false;
            bool maybeAssignment = false;
            std::string token;

            for(char ch : row) {
                if(inString) {
                    char endOfString = isParam ? ']' : '\"';
                    if(!isEscaped && ch == endOfString) {
                        inString = false;
                        rowInfo.tokens.push_back(Token::createFromString(token));
                    } else {
                        if(ch == '\\' && !isEscaped) {
                            isEscaped = true;
                        } else {
                            isEscaped = false;
                            token += ch;
                        }
                    }
                    continue;
                }

                if(maybeAssignment) {
                    maybeAssignment = false;
                    if(ch == '>') {
                        rowInfo.tokens.push_back(Token::createFromDelimiter(KEY_DATASRC_DELIMITER));
                        continue;
                    } else {
                        rowInfo.tokens.push_back(Token::createFromDelimiter(PROP_DATASRC_DELIMITER));
                    }
                }

                if(ch == '\"' || ch == '[') {
                    inString = true;
                    isParam = (ch == '[');
                    token = "";
                    continue;
                }

                if(isIdentifierChar(ch)) {
                    if(!inIdentifier) {
                        token = "";
                        inIdentifier = true;
                    }
                    token += ch;
                    continue;
                }
                if(inIdentifier) {
                    inIdentifier = false;
                    rowInfo.tokens.push_back(Token::createFromIdentifier(token));
                }
                if(ch == '=') {
                    maybeAssignment = true;
                    continue;
                }

                if(isSpaceLikeChar(ch)) {
                    continue;
                }

                throw std::runtime_error((std::string) "Unknown token: " + ch);
            }

            this -> rows.push_back(rowInfo);
        }
    }
}
