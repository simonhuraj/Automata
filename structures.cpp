
#include "structures.h"

using namespace std;

ostream& operator<<(ostream& os, DFA &dfa) {
    os << "States: " << dfa.states << ", initial: " << dfa.initialState << "transitions: ";
    for (int i = 0; i < dfa.states; ++i) {
        for (int j = 0; j < ALPHABET_LEN; ++j) {
            os << dfa.transitions[i][j];
        }
    }
    os << ", final: ";
    for (int i = 0; i < dfa.states; ++i) {
        os << dfa.finalStates[i];
    }
    return os;
}
