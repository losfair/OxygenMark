#ifndef _SIMPLEMARK_PARSER_H_
#define _SIMPLEMARK_PARSER_H_

#include <string>
#include <map>
#include <list>

namespace SimpleMark {
    enum TokenType {
        unknownTokenType,
        indentType,
        stringType,
        delimiterType
    };

    enum DataSourceType {
        unknownDataSourceType,
        fromString,
        fromFile,
        fromParam
    };

    enum Delimiter {
        KEY_CONTENT_DELIMITER,
        KEY_DATASRC_DELIMITER,
        PROP_DATASRC_DELIMITER
    };

    class Token {
        public:
            int indentValue;
            std::string stringValue;
            Delimiter delimiterValue;
            TokenType type;
            Token();
    };

    class DataSource {
        public:
            DataSourceType type;
            std::string ds;
            DataSource();
            DataSource(DataSourceType newType, std::string& newValue);
    };

    class Node {
        public:
            int indent;
            std::string key;
            std::map<std::string, DataSource> properties;
            DataSource content;
            int parent;
            std::list<int> children;
            Node();
    };

    class Document {
        public:
            size_t rowCount;
            Node *nodes;

            Document(std::string& doc);
            Document(const char *filename);
            ~Document();

            void dumpToFile(const char *filename);
    };
}

#endif