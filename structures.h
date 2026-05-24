
#ifndef AUTOMATA_STRUCTURES_H
#define AUTOMATA_STRUCTURES_H

#include <cstdint>
#include <cmath>
#include <array>
#include <ostream>
#include <unordered_set>
#include <set>
#include <vector>

using namespace std;

constexpr uint8_t MAX_STATES = 3;
constexpr uint8_t MAX_STATES_SUBSET = 8; // 2^MAX_STATES
constexpr uint8_t ALPHABET_LEN = 2;

class DFA {
public:
    short states = 0;
    uint8_t initialState = 0;
    array<char, ALPHABET_LEN> alphabet = {'a', 'b'};
    array<array<uint8_t, ALPHABET_LEN>, MAX_STATES_SUBSET> transitions {0};
    array<bool, MAX_STATES_SUBSET> finalStates {0};
    array<uint8_t, MAX_STATES_SUBSET> numberOfEquivalentStates {0};
    DFA() = default;
};

ostream& operator<<(ostream& os, DFA &dfa);

class MultiState {
public:
    uint8_t id = 0;
    unordered_set<::uint8_t> states;
    MultiState() = default;
    explicit MultiState(short int id) : id(id) {};
    bool operator== (const MultiState &other) const { return id == other.id; };
};

struct UnorderedSetOfUintHash {
    size_t operator()(const unordered_set<::uint8_t>& s) const {
        size_t hashValue = 0;
        for (int element : s) {
            hashValue ^= std::hash<int>()(element);
        }
        return hashValue;
    }
};

struct UnorderedSetEqual {
    bool operator()(const std::unordered_set<::uint8_t>& s1, const std::unordered_set<::uint8_t>& s2) const {
        return s1 == s2;
    }
};

#endif //AUTOMATA_STRUCTURES_H
