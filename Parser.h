#ifndef _OXYGENMARK_PARSER_H_
#define _OXYGENMARK_PARSER_H_

#include <string>
#include <map>
#include <list>

namespace OxygenMark {
    enum DataSourceType {
        unknownDataSourceType,
        fromString,
        fromFile,
        fromParam
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
            std::map<std::string, std::string> params;

            Document(std::string& doc);
            Document(const char *filename);
            ~Document();

            void dumpToFile(const char *filename);
    };
}

#endif