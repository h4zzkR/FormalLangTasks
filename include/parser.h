//
// Created by h4zzkr on 17.10.2021.
//

#ifndef REGEXP_PARSER_PARSER_H
#define REGEXP_PARSER_PARSER_H

#include <string>
#include <stack>
#include <vector>
#include <iostream>

struct RegularParser {
    std::string regular;
    int len{};
    char letter{};

    struct DpHandler {
        bool has_right_cnt = false; // есть ли в регулярке слово с i буквами letter
        int  min_len_of_correct = 0; // минимальная длина такого слова

        DpHandler(bool has_right_cnt = false, int ml = 0): has_right_cnt(has_right_cnt),
                                                           min_len_of_correct(ml) {}
    };

    std::stack<std::vector<DpHandler>> stack;

    void stringInput(const std::string& s = "") {
        int i = 0;
        for (letter = s[i]; !std::isdigit(letter); ++i, letter = s[i]) {
            if (letter != ' ')
                regular += letter;
        }

        letter = regular.back();
        regular.pop_back();
        std::string number;

        for (char digit = s[i]; i < s.size(); ++i, digit = s[i]) {
            number += digit;
        }
        len = std::stoi(number);
    }

    void cinInput() {
        std::string inp;
        std::cin >> inp;
        stringInput(inp);
    }

    std::pair<std::vector<DpHandler>, std::vector<DpHandler>> top() {
        if (stack.size() < 2)
            throw std::out_of_range("Stack size does not meet the requirements");
        auto one = stack.top(); stack.pop();
        auto two = stack.top(); stack.pop();
        return {one, two};
    }

    void push_letter(char c) {
        std::vector<DpHandler> handler(len + 1, DpHandler(false, 0));
        if (c == letter) {
            handler[0] = DpHandler(false, -1);
            handler[1] = DpHandler(true, 1);
        } else {
            handler[0] = DpHandler(true, 1);
            handler[1] = DpHandler(false, 0);
        }
        stack.push(handler);
    }

    void sum(std::vector<DpHandler>&one, std::vector<DpHandler>& two, std::vector<DpHandler>& handler) {
        int a = one[0].min_len_of_correct, b = two[0].min_len_of_correct;
        handler[0].has_right_cnt = true;
        if (a == -1 && b == -1) {
            handler[0].has_right_cnt = false;
            handler[0].min_len_of_correct = 0;
        } else if (a == -1)
            handler[0].min_len_of_correct = b;
        else if (b == -1)
            handler[0].min_len_of_correct = a;
        else {
            if (one[0].has_right_cnt && !two[0].has_right_cnt) // длина одного из - 0
                handler[0].min_len_of_correct = a;
            else if (!one[0].has_right_cnt && two[0].has_right_cnt) // длина одного из - 0
                handler[0].min_len_of_correct = b;
            else
                handler[0].min_len_of_correct = std::min(a,b);
        }

        for (int i = 1; i <= len; ++i) {
            if (one[i].has_right_cnt && two[i].has_right_cnt) {
                handler[i].min_len_of_correct = std::min(one[i].min_len_of_correct, two[i].min_len_of_correct);
            } else if (one[i].has_right_cnt && !two[i].has_right_cnt) { // внезапно нашли подходящее слово
                handler[i].min_len_of_correct = one[i].min_len_of_correct;
            } else if (!one[i].has_right_cnt && two[i].has_right_cnt) { // внезапно нашли подходящее слово
                handler[i].min_len_of_correct = two[i].min_len_of_correct;
            }
            handler[i].has_right_cnt = one[i].has_right_cnt | two[i].has_right_cnt;
        }
    }

    void sum_stack() {
        std::vector<DpHandler> handler(len+1, DpHandler(false, 0));
        auto [one, two] = top();
        sum(one, two, handler);
        stack.push(handler);
    }

    void concat(std::vector<DpHandler>&one, std::vector<DpHandler>& two, std::vector<DpHandler>& handler) const {

        // BASE
        for (int i = 0; i < 2; ++i) {
            std::vector<DpHandler> &frst = (i == 0) ? one : two;
            std::vector<DpHandler> &scnd = (i == 1) ? one : two;

            int part = (frst[0].min_len_of_correct == -1) ? 1
                                                          : frst[0].min_len_of_correct; // first is for one letter (special case)
            for (int j = 1; j <= len; ++j) {
                int cnt_in_concat = j * scnd[j].has_right_cnt;
                if (cnt_in_concat != 0) {
                    handler[cnt_in_concat].has_right_cnt = true;
                    handler[cnt_in_concat].min_len_of_correct = part + scnd[j].min_len_of_correct;
                }
            }
        }

        int a = one[0].min_len_of_correct, b = two[0].min_len_of_correct;
        if (a != -1 && b != -1 && one[0].has_right_cnt && two[0].has_right_cnt) {
            handler[0].has_right_cnt = true;
            handler[0].min_len_of_correct = a + b;
        } else {
            handler[0].has_right_cnt = false;
            handler[0].min_len_of_correct = 0;
        }

        // END OF BASE

        for (int i = 1; i <= len; ++i) {
            for (int j = 1; j <= len; ++j) {
                int cnt_in_concat = i * one[i].has_right_cnt + j * two[j].has_right_cnt;
                if (cnt_in_concat != 0 && cnt_in_concat <= len && one[i].has_right_cnt && two[j].has_right_cnt) {
                    handler[cnt_in_concat].has_right_cnt = true;
                    handler[cnt_in_concat].min_len_of_correct = one[i].min_len_of_correct + two[j].min_len_of_correct;
                }
            }
        }
    }

    void concat_stack() {
        std::vector<DpHandler> handler(len+1, DpHandler(false, 0));
        auto [one, two] = top();
        concat(one, two, handler);
        stack.push(handler);
    }

    void star_stack() {
		if (stack.empty())
		   throw std::out_of_range("Stack size does not meet the requirements");
        auto one = stack.top(); stack.pop();
        std::vector<DpHandler> handler = one;
        for (int i = 1; i <= len; ++i) {
            std::vector<DpHandler> current_handler(len+1, DpHandler(false, 0));
            concat(handler, one, current_handler);
            sum(handler, current_handler, handler);
        }
        handler[0].min_len_of_correct = 0; // epsilon word
        stack.push(handler);
    } // (abc)*

    void parse() {
        for (char c : regular) {
            if (c >= 'a' && c <= 'c') {
                push_letter(c);
            } else if (c == '+') {
                sum_stack(); // here todo
            } else if (c == '.') {
                concat_stack();
            } else if (c == '*') {
                star_stack();
            }
        }
    } // abc.. * aaa.. + ca. + * b 1

    int output() {
        std::stack<std::vector<DpHandler>> prey;
        std::swap(prey, stack);
        int out = prey.top()[len].min_len_of_correct;
        regular.clear();
        return out;
    }

    std::string getAnswer() {
        int out = output();
        return std::to_string(out);
//        return (out == 0) ? "INF" : std::to_string(out);
    }

    void coutAnswer() {
        std::cout << getAnswer();
    }

    std::string pipeline(const std::string& inp = "") {
        stringInput(inp);
        parse();
        return getAnswer();
    }
};

#endif //REGEXP_PARSER_PARSER_H
