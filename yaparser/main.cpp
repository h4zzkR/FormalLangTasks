#include "include/LRParser.h"
#include <iostream>

namespace aug {
    struct BasicGrammar {
        Grammar G{"S"};
        BasicGrammar() {
            G.add("S -> d R");
            G.add("R -> o R");
            G.add("R -> r R");
            G.add("R -> p");
        }
    };

    struct BasicGrammar2 {
        Grammar G{"S"};
        BasicGrammar2() {
            G.add("S -> C C");
            G.add("C -> c C");
            G.add("C -> d");
        }
    };

    struct ConflictGrammar {
        Grammar G{"S"};
        ConflictGrammar() {
            G.add("S -> C C");
            G.add("C -> c C");
            G.add("C -> d");
            G.add("C -> D");
            G.add("D -> D");
            G.add("D -> a");
        }
    };

    struct ArithmeticsGrammar {
        Grammar G{"START"};
        ArithmeticsGrammar() {
            G.add("START -> ADD");
            G.add("ADD -> ADD + FACTOR");
            G.add("ADD -> FACTOR");
            G.add("FACTOR -> TERM");
            G.add("TERM -> ( ADD )");
            G.add("TERM -> a");
            G.add("TERM -> b");
        }
    };
}

int main() {
    LRParser parser;
    parser.fit(aug::ArithmeticsGrammar().G);
    std::cout << parser.predict("( a + b )");
    std::cout << parser.predict("a + b + b + a + b + a + b");
//    std::cout << parser.predict("( a * b )");

}