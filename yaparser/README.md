# YetAnotherPARSER
___
**Running tests and code coverage:**
```
cmake -DCMAKE_BUILD_TYPE:STRING=Debug .. && make coverage_report
```
You can check coverage in `build/parser-coverage.html`.
___
## LR(1) grammar parser.
Reference: Dragon book & https://serokell.io/blog/how-to-implement-lr1-parser

## Description
Core of parser are LR tables, that implemented as vector of states of situations with action table. All aux data structures are hash tables, so it will be O(1)* complexity in most operations with them. Complexity is standard for LR-parsers family.
