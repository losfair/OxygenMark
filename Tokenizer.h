#ifndef _OXYGENMARK_TOKENIZER_H_
#define _OXYGENMARK_TOKENIZER_H_

#include <string>
#include <list>

namespace OxygenMark {
    enum TokenType {
        UNKNOWN_TOKEN_TYPE,
        INDENT_TYPE,
        STRING_TYPE,
        PARAM_TYPE,
        DELIMITER_TYPE,
        IDENTIFIER_TYPE
    };

    enum Delimiter {
        KEY_DATASRC_DELIMITER, // =>
        PROP_DATASRC_DELIMITER // =
    };

    class Token {
        public:
            int indentValue;
            std::string stringValue;
            Delimiter delimiterValue;
            TokenType type;
            Token();
            static Token createFromIndent(int);
            static Token createFromString(std::string);
            static Token createFromDelimiter(Delimiter);
            static Token createFromParam(std::string);
            static Token createFromIdentifier(std::string);
    };

    class Row {
        public:
            std::list<Token> tokens;
    };

    class Tokenizer {
        public:
            std::list<Row> rows;
            Tokenizer(std::string& doc);
    };
}

#endif