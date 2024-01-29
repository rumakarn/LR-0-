#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>

using namespace std;

struct LR0Item {
    string production;
    int dotPosition;

    LR0Item(string prod, int dotPos) : production(move(prod)), dotPosition(dotPos) {}

    bool operator==(const LR0Item &other) const {
        return production == other.production && dotPosition == other.dotPosition;
    }
};

namespace std {
    template <>
    struct hash<LR0Item> {
        size_t operator()(const LR0Item &item) const {
            return hash<string>()(item.production) ^ hash<int>()(item.dotPosition);
        }
    };
}

vector<LR0Item> generateLR0Items(const map<string, vector<string>> &grammar) {
    vector<LR0Item> items;
    for (const auto &production : grammar) {
        for (int i = 0; i <= production.second.size(); ++i) {
            items.emplace_back(production.first, i);
        }
    }
    return items;
}

set<LR0Item> closureLR0(const LR0Item &item, const map<string, vector<string>> &grammar) {
    set<LR0Item> closure{item};
    bool changed = true;
    while (changed) {
        changed = false;
        for (const LR0Item &currentItem : closure) {
            string symbolAfterDot = currentItem.production.substr(currentItem.dotPosition, 1);
            if (grammar.find(symbolAfterDot) != grammar.end()) {
                for (const string &prod : grammar.at(symbolAfterDot)) {
                    LR0Item newItem(prod, 0);
                    if (closure.find(newItem) == closure.end()) {
                        closure.insert(newItem);
                        changed = true;
                    }
                }
            }
        }
    }
    return closure;
}

set<LR0Item> gotoLR0(const set<LR0Item> &items, const string &symbol, const map<string, vector<string>> &grammar) {
    set<LR0Item> newItems;
    for (const LR0Item &item : items) {
        string afterDot = item.production.substr(item.dotPosition, 1);
        if (afterDot == symbol) {
            newItems.emplace(item.production, item.dotPosition + 1);
        }
    }
    return closureLR0(newItems, grammar);
}

tuple<vector<set<LR0Item>>, vector<vector<string>>, map<pair<int, string>, int>> constructLR0ParsingTable(const map<string, vector<string>> &grammar) {
    vector<LR0Item> items = generateLR0Items(grammar);
    vector<set<LR0Item>> itemSets{closureLR0(items[0], grammar)};
    map<pair<int, string>, int> transitions;

    for (size_t i = 0; i < itemSets.size(); ++i) {
        for (const auto &symbol : grammar) {
            const set<LR0Item> gotoResult = gotoLR0(itemSets[i], symbol.first, grammar);
            if (!gotoResult.empty()) {
                auto it = find(itemSets.begin(), itemSets.end(), gotoResult);
                if (it == itemSets.end()) {
                    itemSets.push_back(gotoResult);
                    it = prev(itemSets.end());
                }
                transitions[make_pair(i, symbol.first)] = distance(itemSets.begin(), it);
            }
        }
    }

    vector<vector<string>> parsingTable(itemSets.size(), vector<string>(grammar.size() + 1, ""));

    for (size_t i = 0; i < itemSets.size(); ++i) {
        for (const LR0Item &item : itemSets[i]) {
            if (item.dotPosition == item.production.size()) {
                if (item.production[0] == 'S' && item.production[1] == '\'' && item.production[2] == '\0') {
                    parsingTable[i].back() = "accept";
                } else {
                    auto it = find(grammar.begin(), grammar.end(), item.production[0]);
                    if (it != grammar.end()) {
                        size_t index = distance(grammar.begin(), it);
                        parsingTable[i][index] = "reduce " + to_string(index + 1);
                    }
                }
            }
        }

        for (const auto &symbol : grammar) {
            const set<LR0Item> gotoResult = gotoLR0(itemSets[i], symbol.first, grammar);
            auto it = find(itemSets.begin(), itemSets.end(), gotoResult);
            if (it != itemSets.end() && transitions.find(make_pair(i, symbol.first)) == transitions.end()) {
                transitions[make_pair(i, symbol.first)] = distance(itemSets.begin(), it);
            }
        }
    }

    return make_tuple(itemSets, parsingTable, transitions);
}

void printLR0ParsingTable(const vector<set<LR0Item>> &itemSets, const vector<vector<string>> &parsingTable, const map<pair<int, string>, int> &transitions, const map<string, vector<string>> &grammar) {
    vector<string> header;
    for (const auto &symbol : grammar) {
        header.push_back(symbol.first);
    }
    header.push_back("$");

    cout << "\nLR(0) Parsing Table:\n";
    cout << "State\t\t";
    for (const auto &symbol : header) {
        cout << symbol << "\t\t";
    }
    cout << "\n";

    for (size_t i = 0; i < itemSets.size(); ++i) {
        cout << i << "\t\t";
        for (const auto &symbol : header) {
            auto transition = transitions.find(make_pair(i, symbol));
            if (transition != transitions.end()) {
                cout << transition->second << "\t\t";
            } else {
                size_t index = find(header.begin(), header.end(), symbol) - header.begin();
                cout << parsingTable[i][index] << "\t\t";
            }
        }
        cout << "\n";
    }
}

int main() {
    map<string, vector<string>> grammar = {
        {"E", {"E+T", "T"}},
        {"T", {"TE", "F"}},
        {"F", {"F*", "a", "b"}},
    };

    auto [itemSets, parsingTable, transitions] = constructLR0ParsingTable(grammar);
    printLR0ParsingTable(itemSets, parsingTable, transitions, grammar);

    return 0;
}
