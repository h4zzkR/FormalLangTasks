#include <iostream>
#include <utility>
#include <vector>
#include <deque>
#include <string>
#include <cassert>
#include <map>
#include <set>
#include <queue>

class YAParser;

namespace parts {
    class Rule;

    class MetaTerm {
        friend YAParser;
        char label;
        bool starter = false;
        bool nterm   = false;
        friend Rule;
    public:
        MetaTerm() = default;
        explicit MetaTerm(char label, bool starter = false): label(label) {
            this->nterm = std::isupper(label);
            this->starter = starter;
        }
        bool operator==(const MetaTerm& oth) const {
            return (label == oth.label) && (starter == oth.starter);
        }
        bool operator!=(const MetaTerm& oth) const {
            return !(*this == oth);
        }
    };

    class Rule {
        friend YAParser;
        MetaTerm L;
        bool has_start = false;
        std::vector<MetaTerm> R;
        std::vector<size_t>   nterms;
    public:
        Rule(MetaTerm left, std::vector<MetaTerm>&& rule): L(left), R(rule) {
            size_t cnt = 0;
            has_start = (left.starter);
            for (auto& mt : R) {
                if (mt.nterm)
                    nterms.push_back(cnt);
                ++cnt;
            }
        }
        Rule(char L, const std::string& rule): L(MetaTerm(L)) {
            size_t i = 0;
            for (auto& c : rule) {
                R.emplace_back(c, false);
                if (R.back().nterm)
                    nterms.push_back(i);
                ++i;
            }
        }
    };

    class Grammar {
        friend YAParser;
        std::vector<Rule> rules;
    public:
        Grammar() = default;
        Grammar(char startLabel) {
            assert(std::isupper(startLabel));
            std::vector<MetaTerm> r0({MetaTerm(startLabel, false)});
            rules.emplace_back(MetaTerm(startLabel, true), std::move(r0));
        }
        void add(char L, const std::string& R) {
            assert(std::isupper(L));
            rules.emplace_back(L, R);
        }
        Rule& operator[](size_t idx) {
            return rules[idx];
        }
    };
}

class YAParser {
    parts::Grammar G;
    char eps = '@';

    // набор терминалов, в которые может раскрыться нетерминал
    std::map<char, std::set<char>> First;

    struct Config {
        /*
         * S -> .aB <=> dotPos = 0
         * S -> a.B <=> dotPos = 1
         * S -> aB. <=> dotPos = 2
         */
        const parts::Rule& rule;
        size_t size;
        size_t dotPos = 0;
        std::vector<char> oracles;

        Config(const parts::Rule& rule, char oracle): rule(rule) {
            oracles.push_back(oracle);
            size = rule.R.size();
        }

        Config(const parts::Rule& rule, std::vector<char> oracles):
                rule(rule), oracles(std::move(oracles)) {
            size = rule.R.size();
        }

        parts::MetaTerm scan() {
            ++dotPos;
            if (dotPos == size)
                return parts::MetaTerm(-1);
            return rule.R[dotPos-1];
        }
    };
    struct State {
        State* parent = nullptr;
        std::vector<Config> configs;
        size_t nexpanded_cnt; // количество правил, в которых еще не дошли до конца
        std::map<parts::MetaTerm, State*> go;

        void add(const parts::Rule& rule, std::vector<char> oracles) {
            ++nexpanded_cnt;
            configs.emplace_back(rule, std::move(oracles));
        }

        State(State* parent = nullptr): parent(parent) {
            nexpanded_cnt = 0;
        }
    };

    void buildFirst() {
        for (auto& rule : G.rules) {
            int i = 0;
            if (rule.R[i].label == eps) ++i;
            if (i >= rule.R.size()) continue; // todo check
            if (std::islower(rule.R[i].label)) {
                First[rule.L.label].insert(rule.R[i].label);
            }
        }

        for (auto& rule : G.rules) {
            int i = 0;
            if (rule.R[i].label == eps) ++i;
            if (i >= rule.R.size()) continue; // todo check

            if (std::isupper(rule.R[i].label)) {
                if (First.count(rule.L.label)) {
                    for (char c : First[rule.L.label])
                        First[rule.L.label].insert(c);
                }
            }
        }
    }

public:
    YAParser() = default;

    bool isNt(const parts::MetaTerm& t) {
        return std::isupper(t.label);
    }

    /* State with first rule */
    /* сканы сделаем как-нибудь потом (когда разберусь, как получать oracle) */
    // разобрался, TODO
    void enclose(State* state) {
        auto& base_rule = state->configs[0].rule;

        for (auto it : base_rule.nterms) {
            parts::MetaTerm nT = base_rule.R[it];
            for (auto& rule : G.rules) {
                if (rule.L != nT) continue;
                if (it + 1 >= base_rule.R.size()) {
                    state->add(rule, state->configs[0].oracles);
                } else if (!isNt(base_rule.R[it + 1])) {
                    state->add(rule, {base_rule.R[it + 1].label});
                } else {
                    auto found = First[base_rule.R[it + 1].label];
                    state->add(rule, std::vector<char>(found.begin(), found.end()));
                }
            }
        }
    }

    void fit(parts::Grammar&& grammar) {
        G = std::move(grammar);
        buildFirst();
        auto* dfa = new State();
        dfa->add(G[0], {'$'});
        enclose(dfa);
    }
};

int main() {
    parts::Grammar G('S');
//    G.add('S', "aB");
//    G.add('B', "b");
//    G.add('B', "bc");
    G.add('S', "aBd");
    G.add('S', "bB");
    G.add('B', "t");
    G.add('B', "C");
    G.add('C', "g");

    YAParser yap;
    yap.fit(std::move(G));
    int tmp = 0;
    return 0;
}