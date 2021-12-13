//
// Created by h4zzkr on 08.12.2021.
//

#ifndef YAPARSER_LRAUTOMATA_H
#define YAPARSER_LRAUTOMATA_H

#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <iterator>
#include <sstream>

#include <memory>
#include <unordered_map>
#include <unordered_set> // change on prod
#include <queue>

#include "util.h"

class YAParser {
protected:
    Grammar grammar;

    struct Item {
        std::unordered_set<Grammar::Token, Grammar::hasher, Grammar::key_equal> lookahead;
        Grammar::Rule rule;
        size_t dotPtr = 0;

#ifdef DEBUG
        std::string traceRule;
        void makeTraceRule(const Grammar& grammar) {
            traceRule.resize(0);
            traceRule.reserve(lookahead.size() * 2 + rule.suffix.size());
            traceRule.append(grammar.tkn2str(rule.prefix));
            traceRule.append(" -> ");
            int i = 0; bool marked = false;
            for (auto& c : rule.suffix) {
                if (i == dotPtr) {
                    traceRule.push_back('.');
                    traceRule.append(grammar.tkn2str(c));
                    marked = true;
                } else {
                    traceRule.append(grammar.tkn2str(c));
                }
                ++i;
            }
            if (!marked)
                traceRule.push_back('.');
            traceRule.append(", [");
            for (auto& tkn : lookahead) {
                traceRule.append(grammar.tkn2str(tkn));
                traceRule.append(", ");
            }
            traceRule.append("]");
        }
#endif

        size_t hash(bool drop_lookahead = false) const {
            const size_t prime = 3;
            size_t hsh = rule.prefix.label;
            size_t hsh_suff = 0, hsh_lookahead_item = 0;

            size_t prime_ = prime;
            for (const auto & i : rule.suffix) {
                hsh_suff += i.label * prime_;
                prime_ *= prime_;
            }

            hsh ^= hsh_suff;
            prime_ = prime;
            if (!drop_lookahead) {
                for (auto &o: lookahead) {
                    hsh_lookahead_item += o.label * prime_;
                    prime_ *= prime_;
                }
            }
            hsh ^= hsh_lookahead_item;
            hsh ^= dotPtr;
            hsh %= Grammar::tkn_border;

            return hsh;
        }

        size_t getSize() const { return rule.getSize(); }
        void setRule(const Grammar::Rule& rule) {
            this->rule = rule;
        }
        void setDotPtr(size_t dotPtr) {
            this->dotPtr = dotPtr;
        }
        void uplookahead(const Grammar::Token& tkn) {
            lookahead.insert(tkn);
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
        Grammar::Token getTkn(size_t ptr) const {
            return rule.suffix[ptr];
        }
        Item(Grammar::Rule rule, decltype(lookahead) lookahead):
                rule(std::move(rule)), lookahead(std::move(lookahead)) {}
        Item(Grammar::Rule rule, Grammar::Token tkn): rule(std::move(rule)) {
            lookahead.insert(tkn);
        }
    };

    struct item_hasher {
        size_t operator()(const Item& it) const { return it.hash(true); }
    };
    struct item_equal {
        bool operator()(const Item& it1, const Item& it2) const {
            return it1.hash(true) == it2.hash(true);
        }
    };

    using lookahead_type = std::unordered_set<Grammar::Token, Grammar::hasher, Grammar::key_equal>;
    using automata_item_type = std::unordered_map<Grammar::Token, size_t, Grammar::hasher, Grammar::key_equal>;
    using kernels_list = std::unordered_map<std::unordered_set<Item, item_hasher, item_equal>, size_t,
            hashing::sequence_hasher<item_hasher>, hashing::sequence_equal<item_hasher>>;

    struct State {
        size_t parent_id = -1;
        std::unordered_set<Item, item_hasher, item_equal> items;

        void add(const Grammar::Rule& rule, const lookahead_type& lookahead) {
            items.insert(Item(rule, lookahead));
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
                    auto found = items.find(item);
                    if (found != items.end()) {
                        // дополнить lookahead, если в items уже есть подобное правило
                        auto cpy = *found;
                        items.erase(found);
                        cpy.lookahead.merge(lookahead_type(item.lookahead));
#ifdef DEBUG
                        cpy.makeTraceRule(grammar);
#endif
                        items.insert(std::move(cpy));
                    } else {
                        items.insert(item);
                    }
                }

                if (item.dotPtr != item.getSize() && Grammar::isNt(item.getCur())) {
                    lookahead_type lookahead;
                    if (item.dotPtr == item.getSize() - 1) {
                        auto found = grammar.Follow[item.rule.prefix];
                        lookahead.insert(found.begin(), found.end());
                    } else {
                        // todo epsilon
                        if (!Grammar::isNt(item.getTkn(item.dotPtr + 1))) {
                            lookahead.insert(item.getTkn(item.dotPtr + 1));
                        } else {
                            auto found = grammar.First[item.getTkn(item.dotPtr + 1)];
                            lookahead.insert(found.begin(), found.end());
                        }
                    }
                    for (auto& rule : grammar.rules) {
                        if (rule.prefix != item.getCur()) continue;
                        Item nitem(rule, lookahead);
#ifdef DEBUG
                        nitem.makeTraceRule(grammar);
#endif
                        auto found = items.find(nitem);
                        if (found == items.end())
                            Q.push({std::move(nitem), false});
                        else {
                            auto cpy = *found;
                            items.erase(found);
                            cpy.lookahead.merge(lookahead_type(lookahead));
#ifdef DEBUG
                            cpy.makeTraceRule(grammar);
#endif
                            items.insert(std::move(cpy));
                        }
                    }
                }
            }
        }

        State(size_t parent_id): parent_id(parent_id) {}
        State(Grammar& grammar, const std::vector<Item>& kernel, size_t parent_id):
                items(kernel.begin(), kernel.end()), parent_id(parent_id) {
            closure(grammar);
        }
        State(Grammar& grammar, decltype(items)&& kernel, size_t parent_id):
                            items(std::move(kernel)), parent_id(parent_id) {
            closure(grammar);
        }
    };

    void updateAutomata(size_t state_id, kernels_list& list) {
        /// # FIND KERNELS FOR ALL AVIABLE TRANSITIONS # ///
        std::unordered_map<Grammar::Token, std::unordered_set<Item, item_hasher, item_equal>, Grammar::hasher, Grammar::key_equal> kernels;
        for (auto& item : automata[state_id].first.items) {
            if (item.dotPtr == item.getSize()) continue;
            auto transit = item.getCur();
            auto cpy_item = item;
            ++cpy_item.dotPtr;
#ifdef DEBUG
            cpy_item.makeTraceRule(grammar);
#endif
            kernels[transit].insert(std::move(cpy_item));
        }
        /// # CREATE NEW STATES OR CONNECT WITH OLDERS # ///
        for (auto&[tkn, kernel] : kernels) {
            auto found = list.find(kernel);
            if (found != list.end()) {
                automata[state_id].second[tkn] = found->second;
                continue;
            }
            automata[state_id].second[tkn] = automata.size();
            list.insert({kernel, automata.size()});
            automata.emplace_back(
                    State(grammar, std::move(kernel), state_id), automata_item_type{}
                    );
            updateAutomata(automata.size() - 1, list);
        }
    }

    void buildAutomata() {
        /// # INIT AUTOMATA WITH STARTER RULE # ///
        kernels_list list;
        std::unordered_set<Item, item_hasher, item_equal> kernel;
        kernel.emplace(Grammar::Rule(Grammar::Token(0), {grammar.sof_t}), grammar.eof_t);
        list.insert({kernel, 0});
        automata.emplace_back(State(grammar, std::move(kernel), -1), automata_item_type{});

        /// # START RECURSIVE BUILDING # ///
        updateAutomata(0, list);
    }

    std::vector<std::pair<State,automata_item_type>> automata;

public:
    YAParser() = default;
    void fit(Grammar g) {
        grammar = std::move(g);
        grammar.buildFirst();
        grammar.buildFollow();
        buildAutomata();
//        connectState(0);
//
//        std::queue<State*> Q;
//        Q.push(&automata.back());
//
//        while (!Q.empty()) {
//
//        }
    }
};

#endif //YAPARSER_LRAUTOMATA_H






