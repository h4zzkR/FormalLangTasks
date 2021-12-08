//
// Created by h4zzkr on 08.12.2021.
//

#ifndef YAPARSER_LRAUTOMATA_H
#define YAPARSER_LRAUTOMATA_H

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
#include <unordered_set> // change on prod

#include <queue>

#include "util.h"

class YAParser {
protected:
    Grammar grammar;

    struct Item {
        std::unordered_set<Grammar::Token, Grammar::hasher, Grammar::key_equal> oracles;
        Grammar::Rule rule;
        size_t dotPtr = 0;

        size_t hash(bool drop_oracles = false) const {
            size_t prime = 3;
            size_t hsh = rule.prefix.label;
            size_t hsh_suff = 0, hsh_oracle = 0;

            for (const auto & i : rule.suffix) {
                hsh_suff += i.label * prime;
                prime *= prime;
            }

            hsh ^= hsh_suff;
            if (!drop_oracles) {
                prime = 3;
                for (auto &o: oracles) {
                    hsh_oracle += o.label * prime;
                    prime *= prime;
                }
            }
            hsh ^= hsh_oracle;
            hsh ^= dotPtr;
            hsh %= Grammar::tkn_border;

            return hsh;
        }

        size_t getSize() { return rule.getSize(); }
        void setRule(const Grammar::Rule& rule) {
            this->rule = rule;
        }
        void setDotPtr(size_t dotPtr) {
            this->dotPtr = dotPtr;
        }
        void upOracles(const Grammar::Token& tkn) {
            oracles.insert(tkn);
        }
        bool equal(const Item& oth) const {
            return hash(true) == oth.hash(true);
        }
        Grammar::Token getCur() const {
            if (dotPtr == rule.getSize())
                return rule.prefix;
            return rule.suffix[dotPtr];
        }
        Grammar::Token getPref() const {
            return rule.prefix;
        }
        size_t getSize() const {
            return rule.getSize();
        }
        Grammar::Token getTkn(size_t ptr) const {
            return rule.suffix[ptr];
        }
        Item(Grammar::Rule rule, decltype(oracles) oracles):
                    rule(std::move(rule)), oracles(std::move(oracles)) {}
        Item(Grammar::Rule rule, Grammar::Token tkn): rule(std::move(rule)) {
            oracles.insert(tkn);
        }
    };

    using oracles_type = std::unordered_set<Grammar::Token, Grammar::hasher, Grammar::key_equal>;

    struct item_hasher {
        size_t operator()(const Item& it) const { return it.hash(); }
    };
    struct item_equal {
        bool operator()(const Item& it1, const Item& it2) const {
            return it1.hash() == it2.hash();
        }
    };
    struct State {
        State* parent = nullptr;
        std::unordered_set<Item, item_hasher, item_equal> items;
        std::unordered_map<Grammar::Token, State*, Grammar::hasher, Grammar::key_equal> go;

        void add(const Grammar::Rule& rule,
                 const std::unordered_set<Grammar::Token, Grammar::hasher, Grammar::key_equal>& oracles) {
            items.insert(Item(rule, oracles));
        }

        template<typename T>
        void add(T&& item) {
            items.insert(std::forward<T>(item));
        }

        void closure(Grammar& grammar) {
            std::queue<std::pair<Item, bool>> Q;
            for (auto& item : items)
                Q.push({item, true});

            while (!Q.empty()) {
                auto[item, is_kernel] = std::move(Q.front()); Q.pop();
                if (!is_kernel) {
                    items.insert(item);
                }

                if (item.dotPtr != item.getSize() && Grammar::isNt(item.getCur())) {
                    oracles_type oracles;
                    if (item.dotPtr == item.getSize() - 1) {
                        auto found = grammar.Follow[item.rule.prefix];
                        oracles.insert(found.begin(), found.end());
                    } else {
                        // todo epsilon
                        if (!Grammar::isNt(item.getTkn(item.dotPtr + 1))) {
                            oracles.insert(item.getTkn(item.dotPtr + 1));
                        } else {
                            auto found = grammar.First[item.getTkn(item.dotPtr + 1)];
                            oracles.insert(found.begin(), found.end());
                        }
                    }
                    for (auto& rule : grammar.rules) {
                        if (rule.prefix != item.getCur()) continue;
                        Item nitem(rule, oracles);
                        if (items.find(nitem) == items.end())
                            Q.push({nitem, false});
                    }
                }
            }
            // в конце работы lookahead мб разбросаны по сету
            // todo transitive closure
        }

        State(State* parent = nullptr) {
            parent = parent;
        }
        State(Grammar& grammar, std::vector<Item> kernel, State* parent = nullptr):
                                        items(kernel.begin(), kernel.end()) {
            this->parent = parent;
            closure(grammar);
        }
    };

    State* automata;

public:
    YAParser() = default;
    void fit(Grammar g) {
        grammar = std::move(g);

        Grammar::Rule rule_start(Grammar::Token(0), {grammar.sof_t});
        Item item_start(rule_start, {grammar.eof_t});
        automata = new State(grammar, {item_start}, nullptr);
    }
};

#endif //YAPARSER_LRAUTOMATA_H






