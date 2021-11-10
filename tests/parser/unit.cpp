#include "parser.h"
#include <gtest/gtest.h>

TEST(TestCases1, CorrectWork) {
    RegularParser rp;
    rp.prepareInputString("ab + c.aba. * .bac. + . + * b 2");
    EXPECT_TRUE(rp.regular == "ab+c.aba.*.bac.+.+*");
    EXPECT_TRUE(rp.letter == 'b');
    EXPECT_TRUE(rp.len == 2);
    rp.reset();
    auto output = rp.parse("ab + c.aba. * .bac. + . + * b 2");
    EXPECT_EQ(output, "4");
    EXPECT_TRUE(rp.stack.empty() == true);
    EXPECT_TRUE(rp.regular.empty() == true);
    EXPECT_EQ(rp.parse("acb..bab.c. * .ab.ba. + . + *a. b 3"), "7");
}

TEST(TestCases2, CorrectWork) {
    RegularParser rp;
    EXPECT_EQ(rp.parse("ab + c.aba. * . + b 2"), "5");
}

TEST(TestCases3, CorrectWork) {
    RegularParser rp;
    EXPECT_EQ(rp.parse("ab + c.aba. * . + d 2"), "0"); // todo INF
    EXPECT_EQ(rp.parse("bbb.. * b 3"), "3");
}

TEST(TestCases4, CorrectWork) {
    RegularParser rp;
    // ((abc)* + aaa + ca)*
    EXPECT_EQ(rp.parse("abc.. * aaa.. + ca. + * b 1"), "3");
    EXPECT_EQ(rp.parse("abc.. * aaa.. + ca. + * b 2"), "6");
    EXPECT_EQ(rp.parse("abc.. * aaa.. + ca. + * b 3"), "9");
    EXPECT_EQ(rp.parse("abc.. * aaa.. + ca. + * b 0"), "0");
    EXPECT_EQ(rp.parse("abc.. aaa.. + ca. + b 0"), "2");
}

TEST(TestCases5, CorrectWork) {
    RegularParser rp;
    EXPECT_EQ(rp.parse("ab. bc. + bbb.. + b + acc.. + ab. + bbb.. + b 1"), "1");
    EXPECT_EQ(rp.parse("bccb... bbbcccbbb........ + acb.. + bcc.. + c 2 "), "3");
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

