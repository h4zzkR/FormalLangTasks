//
// Created by h4zzkr on 14.12.2021.
//

#ifndef YAPARSER_PARSER_H
#define YAPARSER_PARSER_H

#include "lr1/LRGrammar.h"

class YAParser {
public:
    YAParser() = default;
    virtual void fit(const Grammar& g) = 0;
    virtual bool predict(const std::string& word) = 0;
};

#endif //YAPARSER_PARSER_H
