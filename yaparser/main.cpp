#include "include/LRAutomata.h"


//
//class YAParser;
//
//namespace parts {
//    class Rule;
//
//    class MetaTerm {
//        friend YAParser;
//        char label;
//        bool starter = false;
//        bool nterm   = false;
//        friend Rule;
//    public:
//        MetaTerm() = default;
//        explicit MetaTerm(char label, bool starter = false): label(label) {
//            this->nterm = std::isupper(label);
//            this->starter = starter;
//        }
//        bool operator==(const MetaTerm& oth) const {
//            return (label == oth.label) && (starter == oth.starter);
//        }
//        bool operator!=(const MetaTerm& oth) const {
//            return !(*this == oth);
//        }
//    };
//
//    class Rule {
//        friend YAParser;
//        MetaTerm L;
//        std::vector<MetaTerm> R;
//    public:
//        Rule(MetaTerm left, std::vector<MetaTerm>&& rule): L(left), R(rule) {
//            size_t cnt = 0;
//            for (auto& mt : R) {
//                ++cnt;
//            }
//        }
//        Rule(char L, const std::string& rule): L(MetaTerm(L)) {
//            size_t i = 0;
//            for (auto& c : rule) {
//                R.emplace_back(c, false);
//                ++i;
//            }
//        }
//    };
//
//    class Grammar {
//        friend YAParser;
//        std::vector<Rule> rules;
//    public:
//        Grammar() = default;
//        Grammar(char startLabel) {
//            assert(std::isupper(startLabel));
//            std::vector<MetaTerm> r0({MetaTerm(startLabel, false)});
//            rules.emplace_back(MetaTerm(startLabel, true), std::move(r0));
//        }
//        void add(char L, const std::string& R) {
//            assert(std::isupper(L));
//            rules.emplace_back(L, R);
//        }
//        Rule& operator[](size_t idx) {
//            return rules[idx];
//        }
//    };
//}
//
///*
// * LR(1) parser implementation
// */
//class YAParser {
//    parts::Grammar G;
//    char eps = '@';
//
//    std::map<char, std::set<char>> First;
//    std::map<char, std::set<char>> Follow;
//
//    struct Config {
//        /*
//         * S -> .aB <=> dotPos = 0
//         * S -> a.B <=> dotPos = 1
//         * S -> aB. <=> dotPos = 2
//         */
////        const parts::Rule& rule;
//        const parts::Rule& rule;
//        size_t size;
//        size_t dotPos = 0;
//        std::unordered_set<char> oracles;
//
//#ifdef DEBUG
//        std::string traceRule;
//        void makeTraceRule() {
//            traceRule.resize(0);
//            traceRule.reserve(oracles.size() * 2 + rule.R.size());
//            traceRule.push_back(rule.L.label);
//            traceRule.append(" -> ");
//            int i = 0; bool marked = false;
//            for (auto& c : rule.R) {
//                if (i == dotPos) {
//                    traceRule.push_back('.');
//                    traceRule.push_back(c.label);
//                    marked = true;
//                } else {
//                    traceRule.push_back(c.label);
//                }
//                ++i;
//            }
//            if (!marked)
//                traceRule.push_back('.');
//            traceRule.append(", [");
//            for (char c : oracles) {
//                traceRule.push_back(c);
//                traceRule.append(", ");
//            }
//            traceRule.append("]");
//        }
//#endif
//
//        Config(const parts::Rule& rule, char oracle): rule(rule) {
//            oracles.insert(oracle);
//            size = rule.R.size();
//#ifdef DEBUG
//            makeTraceRule();
//#endif
//        }
//
//        Config(const parts::Rule& rule, std::unordered_set<char>  oracles):
//                rule(rule), oracles(std::move(oracles)) {
//            size = rule.R.size();
//#ifdef DEBUG
//            makeTraceRule();
//#endif
//        }
//
//    };
//
//    struct State {
//        State* parent = nullptr;
//        std::unordered_map<char, std::vector<Config>> configs;
//        std::map<char, State*> go;
//
//        void add(const parts::Rule& rule, std::unordered_set<char>&& oracles) {
//            configs[rule.L.label].push_back(std::move(Config(rule, std::move(oracles))));
//        }
//
//        void add(const parts::Rule& rule, const std::unordered_set<char>& oracles) {
//            configs[rule.L.label].push_back(std::move(Config(rule, oracles)));
//        }
//
//        template <typename T>
//        void add(T&& cfg) {
//            configs[cfg.rule.L.label].push_back(std::forward<T>(cfg));
//#ifdef DEBUG
//            configs[cfg.rule.L.label].back().makeTraceRule();
//#endif
//        }
//
//        State(State* parent = nullptr, size_t size = 1): parent(parent) {}
//    };
//
//    State* dfa;
//
//    void buildFirst() {
//        // transitive closure
//        while (true) {
//            bool changed = false;
//            for (auto &rule: G.rules) {
//                int pos = 0;
//                if (rule.R[pos].label == eps) ++pos;
//                if (pos >= rule.R.size()) continue;
//
//                size_t size = First[rule.L.label].size();
//
//                if (!isNt(rule.R[pos]))
//                    First[rule.L.label].insert(rule.R[pos].label);
//                else
//                    for (char c: First[rule.R[pos].label])
//                        First[rule.L.label].insert(c == eps ? '$' : c);
//
//                if (size != First[rule.L.label].size())
//                    changed = true;
//            }
//            if (!changed) break;
//        }
//    }
//
//    void buildFollow() {
//        while (true) {
//            bool changed = false;
//            for (auto &rule: G.rules) {
//                for (int pos = 0; pos < rule.R.size(); ++pos) {
//                    auto &mt = rule.R[pos];
//                    size_t size = Follow[mt.label].size();
//                    if (!isNt(mt)) continue;
//                    if (pos + 1 == rule.R.size()) {
//                        auto found = Follow[rule.L.label];
//                        Follow[mt.label].insert(found.begin(), found.end());
//                    } else {
//                        auto &nmt = rule.R[pos + 1];
//                        if (!isNt(nmt))
//                            Follow[mt.label].insert(nmt.label);
//                        else {
//                            auto found = First[nmt.label];
//                            Follow[mt.label].insert(found.begin(), found.end());
//                        }
//                    }
//                    if (size != Follow[mt.label].size())
//                        changed = true;
//                }
//            }
//            if (!changed) break;
//        }
//        Follow['S'].insert('$');
//    }
//
//    void enclose(State* state) {
//        std::queue<std::pair<Config, bool>> Q;
//        for (auto&[_, cfg_bucket] : state->configs) {
//            for (auto& cfg : cfg_bucket) {
//                Q.push({cfg, true});
//            }
//        }
//
//        std::vector<bool> visited(G.rules.size(), false);
//        while (!Q.empty()) {
//            auto[cfg, is_kernel] = std::move(Q.front()); Q.pop(); // move is useless for const
//
//            if (!is_kernel) {
//                state->add(cfg.rule, cfg.oracles);
//            }
//
//            auto mt = cfg.rule.R[cfg.dotPos];
//            if (!isNt(mt)) continue;
//
//            std::unordered_set<char> oracles;
//            if (cfg.dotPos + 1 == cfg.rule.R.size()) {
//                auto found = Follow[cfg.rule.L.label];
//                oracles.insert(found.begin(), found.end());
//            } else {
//                auto nmt = cfg.rule.R[cfg.dotPos + 1];
//                if (isNt(nmt)) {
//                    auto found = First[nmt.label];
//                    oracles.insert(found.begin(), found.end());
//                } else {
//                    oracles.insert(nmt.label);
//                }
//            }
//
//            for (int i = 0; i < G.rules.size(); ++i) {
//                auto& rule = G[i];
//                if (visited[i]) {
////                    if (rule.L == cfg.rule.R[0]) {
////                        state->configs[cfg.rule.L.label].back().oracles.insert(
////                                oracles.begin(), oracles.end()
////                                );
////                    }
//                    continue;
//                }
//                if (rule.L != mt) continue; // todo deal with extra copies
//                visited[i] = true;
//                Q.push({Config(rule, oracles), false});
//            }
//        }
//    }

//void step(std::queue<State*>& Q) {
//    auto state = Q.front(); Q.pop();
//    char c = 'a';
//    for (; c != 'Z' + 1; (c == 'z') ? c = 'A' : ++c) {
//        for (auto cfg : state->configs) {
//            if (cfg.rule.R.size() <= cfg.dotPos || cfg.rule.R[cfg.dotPos].label != c) {
//                continue;
//            }
//
//            bool found = false;
//            State* node;
//            if (!state->go.count(c)) {
//                node = new State(state);
//                state->go[c] = node;
//            } else {
//                node = state->go[c];
//                ++node->kernel_size;
//                found = true;
//            }
//
//            ++(cfg.dotPos);
//            node->add(std::move(cfg));
//            if (!found)
//                Q.push(node);
//        }
//    }
//}

//
//public:
//    YAParser() = default;
//
//    bool isNt(const parts::MetaTerm& t) {
//        return std::isupper(t.label);
//    }
//
//    void fit(parts::Grammar&& grammar) {
//        G = std::move(grammar);
//        buildFirst();
//        buildFollow();
//
//        dfa = new State(nullptr, 1);
//        dfa->add(G[0], {'$'});
//        enclose(dfa);
//        std::queue<State*> Q; Q.push(dfa);
////        step(Q);
//
////        while (!Q.empty()) {
////            State* state = Q.front();
////            if (state->configs[0].traceRule == "S -> Tb.C, [$, ]") {
////                bool flag = true;
////            }
////            enclose(state);
////            step(Q);
////            int tmp = 0;
////        }
//        int tmp = 0;
//    }
//
//    ~YAParser() {
//        // todo
//    }
//};

int main() {
    Grammar G("S");
    G.add("S -> S S");
    G.add("S -> T c");
    G.add("S -> x");
    G.add("T -> T T");
    G.add("T -> k");


    YAParser parser;
    parser.fit(G);
    int tmp = 0;
}