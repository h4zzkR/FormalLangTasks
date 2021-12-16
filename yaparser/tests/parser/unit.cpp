#include "LRParser.h"
#include <gtest/gtest.h>

TEST(SimpleTests, TestBucket) {
//    Grammar G("START");
//
//    G.add("START -> ADD");
//    G.add("ADD -> ADD + FACTOR");
//    G.add("ADD -> FACTOR");
//    G.add("FACTOR -> TERM");
//    G.add("FACTOR -> FACTOR * TERM");
//    G.add("TERM -> ( ADD )");
//    G.add("TERM -> a");
//    G.add("TERM -> b");

    Grammar G("S");
    G.add("S -> d R");
    G.add("R -> o R");
    G.add("R -> r R");
    G.add("R -> p");

    LRParser parser;
    parser.fit(G);

    EXPECT_TRUE(parser.predict("d r o p"));
    EXPECT_FALSE(parser.predict("d r o p p"));
    EXPECT_TRUE(parser.predict("d o p"));
}

TEST(ConflictGrammarTests, TestBucket) {
    Grammar G("S");
    G.add("S -> C C");
    G.add("C -> c C");
    G.add("C -> d");
    G.add("C -> D");
    G.add("D -> D");
    G.add("D -> a");

    LRParser parser;
    parser.fit(G);

    EXPECT_THROW(parser.predict("c d a"), parts::ConflictGrammar);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

