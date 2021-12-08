void closure(Grammar& grammar) {
    bool changed;
    do {
        changed = false;
        decltype(items) cpy_items(items);
        for (auto& item : cpy_items) {
            if (item.dotPtr != item.getSize() && Grammar::isNt(item.getCur())) {
                oracles_type oracles;
                if (item.dotPtr == item.getSize() - 1) {
                    auto found = grammar.Follow[item.rule.prefix];
                    oracles.insert(found.begin(), found.end());
                } else {
                    // todo epsilon
                    auto found = grammar.First[item.getTkn(item.dotPtr + 1)];
                    oracles.insert(found.begin(), found.end());
                }

                for (auto& rule : grammar.rules) {
                    if (rule.prefix != item.getCur()) continue;
                    Item nitem(rule, oracles);
                    bool found = false;

                    // инвалидация итераторов
                    for (auto jitem : cpy_items) {
                        if (nitem.equal(jitem)) {
                            items.erase(jitem);
                            jitem.oracles.insert(nitem.oracles.begin(), nitem.oracles.end());
                            items.insert(std::move(jitem));
                            changed = true;
                            found = true;
                        }
                    }

                    if (!found) {
                        cpy_items.insert(nitem);
                        items.insert(nitem);
                        changed = true;
                    }
                }
                if (!changed) break;
            }
        }
    } while (changed);
}