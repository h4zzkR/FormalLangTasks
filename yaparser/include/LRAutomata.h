#ifndef YAPARSER_LRAUTOMATA_H
#define YAPARSER_LRAUTOMATA_H

#include <stack>
#include "LRGrammar.h"
#include "LRUtil.h"
#include "LRState.h"

using namespace hashing;

//struct ActionLabel {
//    virtual ~ActionLabel() {}
//};
//
//struct ShiftLabel: public ActionLabel {
//    size_t state_id;
//    explicit ShiftLabel(size_t state_id): state_id(state_id) {}
//    ~ShiftLabel() override = default;
//};
//struct ReduceLabel: public ActionLabel {
//    size_t state_id;
//    explicit ReduceLabel(size_t state_id): state_id(state_id) {}
//    ~ReduceLabel() override = default;
//};
//struct AcceptLabel: public ActionLabel {
//    ~AcceptLabel() override = default;
//};
//struct ConflictLabel: public ActionLabel {
//    ~ConflictLabel() override = default;
//};

class YAParser {
protected:
    Grammar grammar;

    struct Trace {
        struct StackItem {
            // tkn is 0 => just automata_state_id
            // tkn is not 0 => nterm

            size_t autom_state_id = 0;
            Grammar::Token tkn{0};

            bool isT() {
                if (tkn.label == 0)
                    return false;
                return !Grammar::isNt(tkn);
            }
            bool isTkn() {
                return (tkn.label != 0);
            }
            bool isNull() { return tkn.label == 0; }

            StackItem(size_t id, Grammar::Token tkn): autom_state_id(id), tkn(std::move(tkn)) {}
        };

        std::stack<StackItem> stack;
        std::vector<Grammar::Token> input;
        ActionLabel action_now{};

        void push(const Grammar::Token& tkn) {
            if (tkn.label != 0)
                stack.push({static_cast<size_t>(-1), tkn});
        }

        void push(size_t id) {
            stack.push({id, Grammar::Token(0)});
        }

        void pop() { stack.pop(); }

        Trace() {
            stack.emplace(0, std::move(Grammar::Token(0)));
        }

        Trace(std::vector<Grammar::Token>&& input): input(std::move(input)) {
            stack.emplace(0, std::move(Grammar::Token(0)));
        }
    };

    using automata_item_type = std::unordered_map<Grammar::Token, size_t, Grammar::hasher, Grammar::key_equal>;
    using kernels_list = std::unordered_map<std::unordered_set<Item, item_hasher<Item, std::false_type>, item_equal<Item, std::false_type>>, size_t,
            sequence_hasher<item_hasher<Item, std::false_type>>, sequence_equal<item_hasher<Item, std::false_type>>>;

    void updateAutomata(size_t state_id, kernels_list &list) {
        /// # FIND KERNELS FOR ALL AVIABLE TRANSITIONS # ///
        std::unordered_map<Grammar::Token, std::unordered_set<Item, item_hasher<Item, std::false_type>,
                item_equal<Item, std::false_type>>, Grammar::hasher, Grammar::key_equal> kernels;
        for (auto item: automata[state_id].first.items) {
            if (item.dotPtr == item.getSize()) continue;
            auto transit = item.getCur();
            ++item.dotPtr;
#ifdef DEBUG
            item.makeTraceRule(grammar);
#endif
            kernels[transit].insert(std::move(item));
        }
        /// # CREATE NEW STATES OR CONNECT WITH OLDERS # ///
        for (auto&[tkn, kernel]: kernels) {
            auto found = list.find(kernel);
            if (found != list.end()) {
                automata[state_id].second[tkn] = found->second;
                continue;
            }
            automata[state_id].second[tkn] = automata.size();
            list.insert({kernel, automata.size()});
            automata.emplace_back(
                    State(grammar, extractItems(kernel), state_id), automata_item_type{}
            );
            updateAutomata(automata.size() - 1, list);
        }
    }

    template <typename T>
    std::vector<Item> extractItems(T& kernel) {
        std::vector<Item> vec;
        vec.reserve(kernel.size());
        for (auto it = kernel.begin(); it != kernel.end(); ) {
            vec.push_back(std::move(kernel.extract(it++).value()));
        }
        return vec;
    }

    void buildAutomata() {
        /// # INIT AUTOMATA WITH STARTER RULE # ///
        kernels_list list; // отслеживать дубликаты
        std::unordered_set<Item, item_hasher<Item, std::false_type>, item_equal<Item, std::false_type>> kernel;
        kernel.emplace(Grammar::Rule(Grammar::Token(0), {grammar.sof_t}), grammar.eof_t);
        list.insert({kernel, 0});

        automata.emplace_back(State(grammar, extractItems(kernel), -1), automata_item_type{});
        /// # START RECURSIVE BUILDING # ///
        updateAutomata(0, list);
    }
    void buildAction() {
        /// # PRE-CALCULATION OF TRANSITIONS # ///
        action.resize(automata.size());
        for (size_t i = 0; i < action.size(); ++i) {
            for (auto& item : automata[i].first.items) {
                if (item.dotPtr == item.getSize()) {
                    // reduce
                    for (auto& lookahead_item : item.lookahead) {
                        bool used = action[i].count(lookahead_item);
                        if (lookahead_item == grammar.eof_t && item.isStart()) {
                            if (used && action[i][lookahead_item].type != 'a')
                                action[i][lookahead_item].type = 'c';
                            else
                                action[i][lookahead_item] = ActionLabel('a');
                        } else {
                            if (used && action[i][lookahead_item].type != 'r')
                                action[i][lookahead_item].type = 'c';
                            else
                                action[i][lookahead_item] = ActionLabel('r', item.getRuleId());
                        }
                    }
                } else {
                    auto tkn = item.getCur();
                    if (Grammar::isNt(tkn)) continue;
                    bool used = action[i].count(tkn);
                    auto trace = item.traceRule;
                    if (used && action[i][tkn].type != 's')
                        action[i][tkn].type = 'c';
                    else {
                        auto& next = automata[i].second[tkn];
                        action[i][tkn] = ActionLabel('s', next);
                    }
                }
            }
        }
    }

    bool parse(const std::string& word) {
        // reversed
        auto splitted = split(word);
        trace.input.push_back(grammar.eof_t);
        for (auto it = splitted.rbegin(); it != splitted.rend(); ++it)
            trace.input.emplace_back(*it);

        while (1) {
            auto top = trace.stack.top();
            auto tkn = trace.input.back();
            // there is no such transition
            if (!action[top.autom_state_id].count(tkn))
                return false;
            auto act = action[top.autom_state_id][tkn];
            if (act.type == 's') {
                trace.push(tkn);
                trace.push(act.item_id);
                trace.input.pop_back();
            } else if (act.type == 'r') {
                auto& rule = grammar.rules[act.item_id];

                for (int i = 0; i < rule.getSize();) {
                    if (top.isTkn()) {
                        ++i; // nonterminal
                    }
                    trace.pop();
                    top = trace.stack.top();
                }

                // 0 a 2
                top = trace.stack.top();
                trace.push(rule.prefix); // 0 a 2 B
                trace.push(automata[top.autom_state_id].second[rule.prefix]); // 0 a 2 B 3
            } else if (act.type == 'a')
                return true;
            else if (act.type == 'c') {
                throw std::runtime_error("Conflict: grammar is ambigious");
            }
        }
    }

    void detachAux() {
        decltype(grammar.Follow) prey1 = std::move(grammar.Follow);
        decltype(grammar.First) prey2 = std::move(grammar.First);
    }

    std::vector<std::pair<State, automata_item_type>> automata;
    Trace trace;
    std::vector<std::unordered_map<Grammar::Token, ActionLabel, Grammar::hasher, Grammar::key_equal>> action;

public:
    YAParser() = default;

    void fit(Grammar g) {
        grammar = std::move(g);
        grammar.buildFirst();
        grammar.buildFollow();
        buildAutomata();
        buildAction();
        detachAux();
   }

    bool predict(const std::string& word) {
        bool out = parse(word);
        return out;
    }
};

#endif //YAPARSER_LRAUTOMATA_H






