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

            // new fake start TODO DEBUG
            std::vector<MetaTerm> r0({MetaTerm(startLabel, false)});
//            std::vector<MetaTerm> r0({MetaTerm('T', false), MetaTerm('T')});
            rules.emplace_back(MetaTerm(startLabel, true), std::move(r0));
//            rules.emplace_back(MetaTerm(startLabel, true), std::move(r0));
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

/*
 * LR(1) parser implementation
 */
class YAParser {
    parts::Grammar G;
    char eps = '@';
    parts::MetaTerm epst = parts::MetaTerm(eps);
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

        [[maybe_unused]] size_t nexpanded_cnt; // количество правил, в которых еще не дошли до конца
        std::map<char, State*> go;

        // первые kernel_size в configs отдаются под kernel
        size_t kernel_size = 1;

        void add(const parts::Rule& rule, std::vector<char> oracles) {
             configs.push_back(std::move(Config(rule, std::move(oracles))));
        }

        void add(const Config& cfg) {
            configs.push_back(cfg);
        }

        State(State* parent = nullptr, size_t size = 1): parent(parent), kernel_size(size) {
            nexpanded_cnt = 0;
        }
    };

    State* dfa;

    void buildFirst() {
        // сначала правила, которые раскрываются в один терминал
        for (auto& rule : G.rules) {
            if (rule.R[0].label == eps) {
                First[rule.L.label].insert(eps);
                continue;
            }
            if (std::islower(rule.R[0].label))
                First[rule.L.label].insert(rule.R[0].label);
        }

        for (auto& rule : G.rules) {
            int pos = 0;
            if (rule.R[pos].label == eps) ++pos;
            if (pos >= rule.R.size()) continue;
            if (rule.R.size() <= 1) continue; // уже проверили

            if (std::islower(rule.R[pos].label)) {
                First[rule.L.label].insert(rule.R[0].label);
            } else {
                for (char c : First[rule.R[pos].label]) {
                    First[rule.L.label].insert(c == eps ? '$' : c);
                }
            }
        }
    }

    void enclose(State* state) {
        std::queue<std::pair<Config, bool>> Q;
        for (int i = 0; i < state->kernel_size; ++i) {
            Q.push({state->configs[i], true});
        }

        while (!Q.empty()) {
            auto[cfg, is_kernel] = std::move(Q.front()); Q.pop();
            auto metaterm = cfg.rule.R[cfg.dotPos];

            if (!isNt(metaterm)) {
                if (!is_kernel) {
                    state->add(cfg.rule, cfg.oracles);
                }
            } else {
                for (auto& rule : G.rules) {
                    if (rule.L != metaterm) continue;
                    std::vector<char> oracles;
                    if (cfg.dotPos + 1 >= cfg.rule.R.size()) {
                        oracles.push_back('$');
                    } else {
                        auto nmetaterm = cfg.rule.R[cfg.dotPos + 1];
                        if (!isNt(nmetaterm)) {
                            oracles.push_back(nmetaterm.label);
                        } else {
                            auto found = First[nmetaterm.label];
                            oracles.reserve(found.size());
                            for (auto c : found) {
                                oracles.push_back(c);
                            }
                        }
                    }
                    Config newcfg(rule, std::move(oracles));
                    Q.push({newcfg, false});
                }
            }
        }
    } // todo extend kernel

    void step(std::queue<State*>& Q) {
        auto state = Q.front(); Q.pop();
        char c = 'a';
        for (; c != 'Z' + 1; (c == 'z') ? c = 'A' : ++c) {
            for (const auto& cfg : state->configs) {
                if (cfg.rule.R.size() <= cfg.dotPos || cfg.rule.R[cfg.dotPos].label != c) {
                    continue;
                }

                bool found = false;
                State* node;
                if (!state->go.count(c)) {
                    node = new State(state);
                    state->go[c] = node;
                } else {
                    node = state->go[c];
                    ++node->kernel_size;
                    found = true;
                }

                node->add(cfg);
                ++(node->configs[node->configs.size() - 1].dotPos);
                if (!found)
                    Q.push(node);
            }
        }
    }

public:
    YAParser() = default;

    bool isNt(const parts::MetaTerm& t) {
        return std::isupper(t.label);
    }

    void fit(parts::Grammar&& grammar) {
        G = std::move(grammar);
        buildFirst();

        dfa = new State(nullptr, 1);
        dfa->add(G[0], {'$'});
        enclose(dfa);
        std::queue<State*> Q; Q.push(dfa);
        step(Q);

        while (!Q.empty()) {
            State* state = Q.front();
            enclose(state);
            step(Q);
        }
        int tmp = 0;
    }

    ~YAParser() {
        // todo smart ptrs
    }
};

int main() {
    parts::Grammar G('S');
//    G.add('S', "aBd");
//    G.add('S', "bB");
//    G.add('B', "t");
//    G.add('B', "C");
//    G.add('C', "g");

    G.add('S', "aBd");
    G.add('S', "aT");
    G.add('T', "c");
    G.add('B', "t");

    YAParser yap;
    yap.fit(std::move(G));
    int tmp = 0;
    return 0;
}