#include "include/parser.h"

int main() {
//    std::string in = "ab + c.aba. * .bac. + . + * b 2";
//    Parser::StringInput inp;
//    in >> inp;
//    Parser p(inp);
//    std::cout << p.parse();
//    std::string tmp;
//    tmp << p.parse();
//    std::cout << '\n' << tmp << '\n';

    Parser::StdinInput inp2;
    Parser::StringInput inp = std::string("sdsdds");
    std::cin >> inp2;
    Parser p2(inp2);
    std::cout << p2.parse();

    inp << "ab+c.aba.*.bac.+.+* b 2";
    Parser p(inp);
    std::string tmp = p.parse();
    std::cout << tmp;
}