#include "parser.h"
#include <gtest/gtest.h>

TEST(TestCases1, CorrectWork) {
    Parser::StringInput inp = std::string("ab + c.aba. * .bac. + . + * b 2");
    Parser rp;
    rp.ps.prepareInputString("ab + c.aba. * .bac. + . + * b 2");
    EXPECT_TRUE(rp.ps.regular == "ab+c.aba.*.bac.+.+*");
    EXPECT_TRUE(rp.ps.letter == 'b');
    EXPECT_TRUE(rp.ps.len == 2);
    rp.ps.reset();
    std::string output = rp.parse(inp);
    EXPECT_EQ(output, "4");
    EXPECT_TRUE(rp.ps.stack.empty() == true);
    EXPECT_TRUE(rp.ps.regular.empty() == true);
    
    inp = "acb..bab.c. * .ab.ba. + . + *a. b 3";
    EXPECT_EQ((std::string)rp.parse(inp), "7");
}

TEST(TestCases2, CorrectWork) {
    Parser rp;
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("ab + c.aba. * . + b 2")), "5");
}

TEST(TestCases3, CorrectWork) {
    Parser rp;
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("ab + c.aba. * . + d 2")), "0"); // todo INF
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("bbb.. * b 3")), "3");
}

TEST(TestCases4, CorrectWork) {
    Parser rp;
    // ((abc)* + aaa + ca)*
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("abc.. * aaa.. + ca. + * b 1")), "3");
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("abc.. * aaa.. + ca. + * b 2")), "6");
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("abc.. * aaa.. + ca. + * b 3")), "9");
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("abc.. * aaa.. + ca. + * b 0")), "0");
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("abc.. aaa.. + ca. + b 0")), "2");
}

TEST(TestCases5, CorrectWork) {
    Parser rp;
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("ab. bc. + bbb.. + b + acc.. + ab. + bbb.. + b 1")), "1");
    EXPECT_EQ((std::string)rp.parse(Parser::StringInput("bccb... bbbcccbbb........ + acb.. + bcc.. + c 2 ")), "3");
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

