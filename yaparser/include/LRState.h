//
// Created by h4zzkr on 13.12.2021.
//

#ifndef YAPARSER_LRSTATE_H
#define YAPARSER_LRSTATE_H

#include "LRUtil.h"

using namespace hashing;

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
    size_t getRuleId() const { return rule.rule_id; }
    bool isStart() const {
        return Grammar::isStart(rule.prefix);
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
    Item(Grammar::Rule rule, const Grammar::Token& tkn): rule(std::move(rule)) {
        lookahead.insert(tkn);
    }
};

struct State {
    size_t parent_id = -1;
    std::unordered_set<Item, item_hasher<Item>, item_equal<Item>> items;

    using lookahead_type = std::unordered_set<Grammar::Token, Grammar::hasher, Grammar::key_equal>;

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
//                        auto found = grammar.First[item.getTkn(item.dotPtr + 1)];
                        auto found = grammar.First[item.getCur()];
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

#endif //YAPARSER_LRSTATE_H
