//
// Created by h4zzkr on 13.12.2021.
//

#ifndef YAPARSER_LRGRAMMAR_H
#define YAPARSER_LRGRAMMAR_H

#pragma once
#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <deque>
#include <string>
#include <cassert>
#include <map>
#include <set>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "LRUtil.h"

class YAParser;
class Item;
class State;

class Grammar {
    static const size_t tkn_border = 2147483647; // prime

    friend YAParser;
    friend Item;
    friend State;

    struct Token {
        size_t label{};
#ifdef DEBUG
        std::string trace;
#endif
        static size_t hash(const std::string& tkn, bool isStart, bool isNterm) {
            if (isStart)
                return 0;
            size_t hsh = std::hash<std::string>{}(tkn) + 1;
            if (isNterm)
                return hsh % tkn_border;
            else
                return tkn_border + (hsh % tkn_border);
        }
        static bool isNterm(const Token& tkn) {
            return tkn.label < tkn_border;
        }
        Token() = default;
        Token(size_t label): label(label) {}
        explicit Token(const std::string& label, bool isStart = false, bool isNterm = false):
                Token(hash(label, isStart, isNterm)) {
#ifdef DEBUG
            trace = label;
#endif
        }
        bool operator==(const Token& oth) const {
            return (isNterm(*this) == isNterm(oth)) && (label == oth.label);
        }
        bool operator!=(const Token& oth) const {
            return !(*this == oth);
        }
    };

private:
    struct Rule {
        size_t rule_id = 0;
        Token prefix;
        std::vector<Token> suffix;
        Rule() = default;
        Rule(Token pref, std::vector<Token>&& suff): prefix(std::move(pref)), suffix(std::move(suff)) {}
        Rule(const std::string& pref, const std::vector<std::string>& suff, bool init = false) {
            prefix = Token(pref, init, true);
            for (auto& s : suff) {
                suffix.emplace_back(s, false,
                                    std::all_of(s.begin(), s.end(),
                                                [](unsigned char c){ return std::isupper(c); }));

            }
        }
        [[nodiscard]] size_t getSize() const { return suffix.size(); }
    };

    struct hasher {
        bool operator()(const Token& tkn) const { return tkn.label; }
    };
    struct key_equal {
        bool operator()(const Token& tkn, const Token& tkn2) const { return tkn.label == tkn2.label; }
    };

    std::vector<Rule> rules;
    std::unordered_map<Token, std::string, hasher, key_equal> terminals;
    std::unordered_map<Token, std::string, hasher, key_equal> nterminals;
    std::string startNterminal, eof = "$";
    Token eof_t, sof_t;
    std::unordered_map<Token, std::unordered_set<Token, hasher, key_equal>, hasher, key_equal> First;
    std::unordered_map<Token, std::unordered_set<Token, hasher, key_equal>, hasher, key_equal> Follow;

    static bool isNt(const Token& tkn) {
        return Token::isNterm(tkn);
    }

    static bool isStart(const Token& tkn) {
        return tkn.label == 0;
    }

    std::string tkn2str(const Token& tkn) const {
        if (isNt(tkn)) {
            auto found = nterminals.find(tkn);
            if (found != nterminals.end())
                return found->second;
            return "DROP_TABLE KVM_BRS";
        } else {
            auto found = terminals.find(tkn);
            if (found != terminals.end())
                return found->second;
            return "DROP_TABLE KVM_BRS";
        }
    }

public:
    Grammar() = default;
    Grammar(std::string start): startNterminal(std::move(start)) {
        add("START -> " + startNterminal, true);
        sof_t = Token(startNterminal, false, true);
        eof_t = Token(eof, false, false);
        terminals[eof_t] = eof;
    }
    /* Non terminals are in uppercase */
    void add(const std::string& rule, bool init = false) {
        /* S -> aba mfdfm SS df */
        std::string del = " -> ";
        size_t idx = rule.find(del);
        auto pref = rule.substr(0, idx);
        std::vector<std::string> suff = split(rule.substr(idx + del.size(), rule.size()));
        rules.emplace_back(pref, suff, init);

        rules.back().rule_id = rules.size() - 1;

        nterminals[{rules.back().prefix}] = pref;
        for (int i = 0; i < suff.size(); ++i) {
            if (Token::isNterm(rules.back().suffix[i]))
                nterminals[{rules.back().suffix[i]}] = suff[i];
            else
                terminals[{rules.back().suffix[i]}] = suff[i];
        }
    }

    Rule& operator[](size_t idx) {
        return rules[idx];
    }
    void buildFirst() {
        // transitive closure
        while (true) {
            bool changed = false;
            for (auto &rule: rules) {
                int pos = 0;
                size_t size = First[rule.prefix].size();
                if (!isNt(rule.suffix[pos]))
                    First[rule.prefix].insert(rule.suffix[pos]);
                else {
                    auto found = First[rule.suffix[pos]];
                    First[rule.prefix].insert(found.begin(), found.end());
                }

                if (size != First[rule.prefix].size())
                    changed = true;
            }
            if (!changed) break;
        }
    }
    void buildFollow() {
        Follow[0].insert(eof_t);
        while (true) {
            bool changed = false;
            for (auto &rule: rules) {
                for (int pos = 0; pos < rule.getSize(); ++pos) {
                    auto &mt = rule.suffix[pos];
                    if (!isNt(mt)) continue;
                    size_t size = Follow[mt].size();
                    if (pos == rule.getSize() - 1) {
                        auto found = Follow[rule.prefix];
                        Follow[mt.label].insert(found.begin(), found.end());
                    } else {
                        auto &nmt = rule.suffix[pos + 1];
                        if (!isNt(nmt))
                            Follow[mt].insert(nmt);
                        else {
                            auto found = First[nmt];
                            Follow[mt].insert(found.begin(), found.end());
                        }
                    }
                    if (size != Follow[mt].size())
                        changed = true;
                }
            }
            if (!changed) break;
        }
    }
};

#endif //YAPARSER_LRGRAMMAR_H
