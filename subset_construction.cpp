#include "subset_construction.h"

using namespace std;

void generateAllMultiStates(
        unordered_map<unordered_set<::uint8_t>, ::uint8_t, UnorderedSetOfUintHash, UnorderedSetEqual> &multiStates,
        short states) {
    ::uint8_t id = 0;
    for (int i = 1; i < (1 << states); i++) {
        unordered_set<::uint8_t> subset;
        for (uint8_t j = 0; j < states; j++) {
            if ((i & (1 << j)) > 0) subset.insert(j);
        }
        multiStates[subset] = id++;
    }
}

void build(DFA &dfa, vector<::uint8_t> &family, short vectorLen, short states) {
    unordered_map<unordered_set<::uint8_t>, ::uint8_t, UnorderedSetOfUintHash, UnorderedSetEqual> multiStates;
    generateAllMultiStates(multiStates, states);

    for (const auto &entry: multiStates) {
        for (short alphabetIdx = 0; alphabetIdx < ALPHABET_LEN; alphabetIdx++) {
            bool finalState = false;
            unordered_set<uint8_t> destination;
            for (uint8_t state: entry.first) {
                destination.insert(family[ALPHABET_LEN * state + alphabetIdx]);
                if (family[vectorLen - states + state]) finalState = true;
            }

            dfa.transitions[entry.second][alphabetIdx] = multiStates[destination];
            dfa.finalStates[entry.second] = finalState;
        }
    }
    dfa.states = multiStates.size();
}



