#include <vector>
#include "minimizer.h"

void getEquivalentClasses(DFA &dfa, unordered_set<unordered_set<::uint8_t>, UnorderedSetOfUintHash> &p);

void
selectRepresentatives(DFA &dfa, unordered_set<unordered_set<::uint8_t>, UnorderedSetOfUintHash> &equivalentClasses);

void getFinalStates(const DFA &dfa, unordered_set<::uint8_t> &finalStates);

void getNonFinalStates(const DFA &dfa, unordered_set<::uint8_t> &nonFinalStates);

void
intersect(const unordered_set<::uint8_t> &x, const unordered_set<::uint8_t> &y, unordered_set<::uint8_t> &intersection);

void
subtract(const unordered_set<::uint8_t> &x, const unordered_set<::uint8_t> &y, unordered_set<::uint8_t> &subtraction);


void minimize(DFA &dfa) {
    unordered_set<unordered_set<::uint8_t>, UnorderedSetOfUintHash> equivalentClasses;
    getEquivalentClasses(dfa, equivalentClasses);
    selectRepresentatives(dfa, equivalentClasses);
}

void getEquivalentClasses(DFA &dfa, unordered_set<unordered_set<::uint8_t>, UnorderedSetOfUintHash> &p) {
    unordered_set<unordered_set<::uint8_t>, UnorderedSetOfUintHash> w;
    unordered_set<::uint8_t> finalStates = unordered_set<::uint8_t>(2 * dfa.states);;
    unordered_set<::uint8_t> nonFinalStates = unordered_set<::uint8_t>(2 * dfa.states);;

    getFinalStates(dfa, finalStates);
    getNonFinalStates(dfa, nonFinalStates);

    if (!finalStates.empty()) {
        p.insert(finalStates);
        w.insert(finalStates);
    }
    if (!nonFinalStates.empty()) {
        p.insert(nonFinalStates);
        w.insert(nonFinalStates);
    }

    unordered_set<::uint8_t> setFromW;
    while (!w.empty()) {
        setFromW = *w.begin();
        w.erase(w.begin());

        for (short letterIdx = 0; letterIdx < ALPHABET_LEN; letterIdx++) {
            unordered_set<::uint8_t> x;
            for (::uint8_t state = 0; state < dfa.states; state++) {
                if (setFromW.find(dfa.transitions[state][letterIdx]) != setFromW.end()) {
                    x.insert(state);
                }
            }

            unordered_set<unordered_set<::uint8_t>, UnorderedSetOfUintHash> copyOfP;
            copyOfP = p;
            for (const auto & itr : copyOfP) {
                unordered_set<::uint8_t> intersection;
                intersect(x, itr, intersection);

                unordered_set<::uint8_t> subtraction;
                subtract(x, itr, subtraction);

                if (!intersection.empty() && !subtraction.empty()) {
                    p.erase(itr);
                    p.insert(intersection);
                    p.insert(subtraction);

                    if (w.find(itr) != w.end()) {
                        w.erase(itr);
                        w.insert(intersection);
                        w.insert(subtraction);
                    } else {
                        if (intersection.size() <= subtraction.size()) w.insert(intersection);
                        else w.insert(subtraction);
                    }
                }
            }
        }
    }
}

void
selectRepresentatives(DFA &dfa, unordered_set<unordered_set<::uint8_t>, UnorderedSetOfUintHash> &equivalentClasses) {
    vector<::uint8_t> representatives(2*dfa.states);
    unordered_set<::uint8_t> equivalentClassRepresentatives = unordered_set<::uint8_t>(2 * dfa.states);;
    vector<::uint8_t> numberOfEquivalentStates(2 * dfa.states);

    // from each class of equivalence take the minimal state as a representative,
    // put it in set of all representatives and mark the amount of with him equivalent states
    for (auto set: equivalentClasses) {
        ::uint8_t minimum = *set.begin();
        for (::uint8_t element: set) {
            if (element < minimum) minimum = element;
        }
        equivalentClassRepresentatives.insert(minimum);
        numberOfEquivalentStates[minimum] = set.size();
        for (::uint8_t element: set) {
            representatives.at(element) = minimum;
        }
    }

    // reroute representatives to be indexed from 0 to equivalentClasses.size() - 1
    ::uint8_t reroute = 1;
    for (::uint8_t i = 1; i < dfa.states; i++) {
        if (reroute == equivalentClasses.size()) break;
        bool rerouted = false;
        for (::uint8_t j = 0; j < representatives.size(); j++) {
            if (representatives.at(j) == i) {
                representatives.at(j) = reroute;
                rerouted = true;
            }
        }
        if (rerouted) reroute++;
    }

    // create new transitions with only the representatives as states, which are relabeled to use only values from 0..equivalentClasses.size()-1
    // create new final states also based on rerouted representatives
    // set number of equivalent states into DFA
    vector<vector<::uint8_t>> newTransitions(equivalentClasses.size(), vector<::uint8_t>(ALPHABET_LEN));
    vector<bool> newFinalStates(equivalentClasses.size());
    for (::uint8_t r: equivalentClassRepresentatives) {
        for (short alphabetIdx = 0; alphabetIdx < ALPHABET_LEN; ++alphabetIdx) {
            newTransitions[representatives[r]][alphabetIdx] = representatives[dfa.transitions[r][alphabetIdx]];
        }
        newFinalStates[representatives[r]] = dfa.finalStates[r];
        dfa.numberOfEquivalentStates[representatives[r]] = numberOfEquivalentStates[r];
    }

    // set new transitions, final states and number of states into DFA
    for (::uint8_t i = 0; i < newTransitions.size(); i++) {
        for (::uint8_t j = 0; j < ALPHABET_LEN; j++) {
            dfa.transitions[i][j] = newTransitions[i][j];
        }
        dfa.finalStates[i] = newFinalStates[i];
    }
    dfa.states = equivalentClasses.size();
}


void intersect(const unordered_set<::uint8_t> &x, const unordered_set<::uint8_t> &y,
               unordered_set<::uint8_t> &intersection) {
    for (::uint8_t s: x) {
        if (y.find(s) != y.end()) intersection.insert(s);
    }
}

void
subtract(const unordered_set<::uint8_t> &x, const unordered_set<::uint8_t> &y, unordered_set<::uint8_t> &subtraction) {
    for (::uint8_t s: y) {
        if (x.find(s) == x.end()) subtraction.insert(s);
    }
}

void getFinalStates(const DFA &dfa, unordered_set<::uint8_t> &finalStates) {
    for (uint8_t i = 0; i < dfa.states; i++) {
        if (dfa.finalStates[i]) finalStates.insert(i);
    }
}

void getNonFinalStates(const DFA &dfa, unordered_set<::uint8_t> &nonFinalStates) {
    for (uint8_t i = 0; i < dfa.states; i++) {
        if (!dfa.finalStates[i]) nonFinalStates.insert(i);
    }
}