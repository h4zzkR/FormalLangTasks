#include "LRParser.h"
#include <gtest/gtest.h>

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

    struct BasicGrammar3 {
        Grammar G{"S"};
        BasicGrammar3() {
            G.add("S -> S S");
            G.add("S -> T c");
            G.add("S -> x");
            G.add("T -> T T");
            G.add("T -> k");
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

TEST(SimpleTests, TestBucket) {
    LRParser parser;
    parser.fit(aug::BasicGrammar().G);

    EXPECT_TRUE(parser.predict("d r o p"));
    EXPECT_FALSE(parser.predict("d r o p p"));
    EXPECT_TRUE(parser.predict("d o p"));

    LRParser parser2;
    parser2.fit(aug::BasicGrammar2().G);
    EXPECT_TRUE(parser2.predict("c d d"));
    EXPECT_FALSE(parser2.predict("d d c"));

    LRParser parser3;
    parser3.fit(aug::BasicGrammar3().G);
    EXPECT_TRUE(parser3.predict("k c x"));
    EXPECT_FALSE(parser3.predict("k c c"));
    EXPECT_TRUE(parser3.predict("k c x"));
    EXPECT_THROW(parser3.predict("k k k c"), parts::ConflictGrammar);
}

TEST(GrammarHotSwitch, TestBucket) {
    aug::BasicGrammar g1;
    LRParser parser;
    parser.fit(g1.G);
    EXPECT_TRUE(parser.predict("d r o p"));

    aug::BasicGrammar2 g2;
    parser.fit(g2.G);
    EXPECT_TRUE(parser.predict("c d d"));
    EXPECT_FALSE(parser.predict("d d c"));
}

TEST(ConflictGrammarTests, TestBucket) {
    aug::ConflictGrammar g;
    LRParser parser;
    parser.fit(g.G);

    EXPECT_THROW(parser.predict("c d a"), parts::ConflictGrammar);
}

TEST(ArithmeticsTests, TestBucket) {
    aug::ArithmeticsGrammar g;
    LRParser parser;
    parser.fit(g.G);
    EXPECT_TRUE(parser.predict("a + b"));
    EXPECT_TRUE(parser.predict("a + b + b + a + b + a + b"));
    EXPECT_TRUE(parser.predict("( a )"));
    EXPECT_TRUE(parser.predict("( a + b )"));
    EXPECT_TRUE(parser.predict("( ( ( ( a ) ) ) )"));
    EXPECT_TRUE(parser.predict("( a + b ) + ( b + a )"));
    EXPECT_TRUE(parser.predict("( a + b ) + ( b + ( a + b ) + a )"));
    EXPECT_FALSE(parser.predict("( a + b ) + ( b + ( a + b ) + a ) ) ("));
    EXPECT_FALSE(parser.predict(") ( ) ( ) ("));
    EXPECT_FALSE(parser.predict("( ( ( a ) ) ) + ( b ) + ( a b )"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

