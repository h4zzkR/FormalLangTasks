#include "include/LRParser.h"
#include <iostream>

int main() {
//    Grammar G("S");
//    G.add("S -> d R");
//    G.add("R -> o R");
//    G.add("R -> r R");
//    G.add("R -> p");
//
//    LRParser parser;
//    parser.fit(G);
//
//    bool out;
//    out = parser.predict("d r o p p");
//    out = parser.predict("d o p");

    Grammar G("S");
    G.add("S -> C C");
    G.add("C -> c C");
    G.add("C -> d");
    G.add("C -> D");
    G.add("D -> D");
    G.add("D -> a");

    LRParser parser;
    parser.fit(G);

    std::cout << parser.predict("c d a");
}