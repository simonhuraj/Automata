#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <chrono>
#include <thread>
#include "structures.h"
#include "subset_construction.h"
#include "minimizer.h"

using namespace std;


void generate(int states, long long startFamily, long long count, int threadCount);

void convertNumberToVector(vector<uint8_t> &v, short int len, long long number, short int states);

void getStructureStateComplexities(const DFA &dfa, vector<::uint8_t> &stateComplexities);

void handleFamilyResultsLanguages(::uint8_t languages, long long family, int &maxLanguagesInFamily,
                                  long long &countOfFamiliesWithMaxLanguages, vector<long long> &familiesWithMaxLanguages,
                                  long long* languagesCount);

void handleFamilyResultsUniqueStateComplexities(vector<::uint8_t> &stateComplexities, long long family,
                                                int &maxUniqueStateComplexitiesInFamily,
                                                long long &countOfFamiliesWithMaxUniqueStateComplexities,
                                                vector<long long> &familiesWithMaxUniqueStateComplexities,
                                                long long* uniqueStateComplexitiesCount);

int main() {
    const int states = MAX_STATES;
    const int threadsCount = 1;
    const long long offsetFamily = 0;

    long long limit = (long) (pow(states, states * ALPHABET_LEN) * pow(2, states));
//    long long limit = 30000000000;
    long long part = limit / threadsCount;

    auto start = chrono::high_resolution_clock::now();

    std::vector<std::thread> threads(threadsCount);
    for (int i = 0; i < threadsCount - 1; i++) {
        threads[i] = thread(generate, states, i * part + offsetFamily, part, i);
    }
    long long rest = limit - (threadsCount - 1) * part;
    threads[threadsCount - 1] = thread(generate, states, (threadsCount - 1) * part + offsetFamily, rest, threadsCount - 1);

    for (int i = 0; i < threadsCount; i++) {
        threads[i].join();
    }

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    cout << "duration: " << duration.count() << endl;

    return 0;
}

long long countStates(const DFA &dfa) {
   long long count = 0;

    for (::uint8_t i = 0; i < dfa.states; i++) {
        queue<::uint8_t> queue;
        queue.push(i);
        unordered_set<::uint8_t> visited = unordered_set<::uint8_t>(2 * dfa.states);
        visited.insert(i);

        while (!queue.empty()) {
            ::uint8_t current = queue.front();
            queue.pop();
            for (::uint8_t alphabetIdx = 0; alphabetIdx < ALPHABET_LEN; alphabetIdx++) {
                ::uint8_t destination = dfa.transitions[current][alphabetIdx];
                if (visited.find(destination) == visited.end()) {
                    queue.push(destination);
                    visited.insert(destination);
                }
            }
        }
        if (dfa.numberOfEquivalentStates[i] == 0) count += visited.size();
        else count += dfa.numberOfEquivalentStates[i] * visited.size();
    }
    return count;
}

void generate(int states, long long startFamily, long long count, int threadCount) {
    int vectorLen = (ALPHABET_LEN + 1) * states;
    long long familyNumber = startFamily;

    string fileName = "result" + to_string(threadCount) + ".txt";
    auto resultFile = std::fstream(fileName, std::ios::out);
    resultFile << "families count: " << count << endl;

    // handling languages
    int maxLanguagesInFamily = -1;
    long long countOfFamiliesWithMaxLanguages = 0;
    vector<long long> familiesWithMaxLanguages(10);
    long long languagesCount[MAX_STATES_SUBSET] {0};

    //handling state complexities
    int maxUniqueStateComplexitiesInFamily = -1;
    long long countOfFamiliesWithMaxUniqueStateComplexities = 0;
    vector<long long> familiesWithMaxUniqueStateComplexities(10);
    long long uniqueStateComplexitiesCount[MAX_STATES_SUBSET] {0};

    long long allStateComplexities[MAX_STATES_SUBSET] {0};


    long long beforeMinimization = 0;
    long long afterMinimization = 0;
    for (long i = 0; i < count; ++i) {
        vector<::uint8_t> familyVector(vectorLen, 0);
        convertNumberToVector(familyVector, vectorLen, familyNumber, states);
        DFA dfa;

        build(dfa, familyVector, vectorLen, states);
        beforeMinimization += countStates(dfa);
        minimize(dfa);
        afterMinimization += countStates(dfa);

        vector<::uint8_t> stateCompl(dfa.states);
        getStructureStateComplexities(dfa, stateCompl);

        int summedStateComplexity = 0;
        for (::uint8_t idx = 0; idx < dfa.states; idx++) {
            allStateComplexities[idx] += stateCompl[idx];
            summedStateComplexity += (idx + 1) * stateCompl[idx];
        }
        handleFamilyResultsLanguages(dfa.states, familyNumber, maxLanguagesInFamily, countOfFamiliesWithMaxLanguages,
                                     familiesWithMaxLanguages, languagesCount);
        handleFamilyResultsUniqueStateComplexities(stateCompl, familyNumber, maxUniqueStateComplexitiesInFamily,
                                                   countOfFamiliesWithMaxUniqueStateComplexities,
                                                   familiesWithMaxUniqueStateComplexities,
                                                   uniqueStateComplexitiesCount);

        resultFile << familyNumber << "-" << summedStateComplexity << endl;

        familyNumber++;
    }
    resultFile << "state complexities for all Automata: ";
    for (long long s: allStateComplexities) resultFile << s << ", ";

    resultFile << "\nhighest range of state complexity: " << maxUniqueStateComplexitiesInFamily;
    resultFile << "\nnumber of max range of state complexity: " << countOfFamiliesWithMaxUniqueStateComplexities;
    resultFile << "\nfamilies: ";
    for (long long f: familiesWithMaxUniqueStateComplexities) resultFile << f << ", ";
    resultFile << "\ncount of range of state complexities: ";
    for (long long s: uniqueStateComplexitiesCount) resultFile << s << ", ";

    resultFile << "\nmax languages: " << maxLanguagesInFamily;
    resultFile << "\nnumber of max languages: " << countOfFamiliesWithMaxLanguages;
    resultFile << "\nfamilies: ";
    for (long long f: familiesWithMaxLanguages) resultFile << f << ", ";
    resultFile << "\ncounts: ";
    for (long long s: languagesCount) resultFile << s << ", ";

    resultFile.close();
    cout << beforeMinimization << " - " << afterMinimization << endl;
}

void convertNumberToVector(vector<uint8_t> &v, short len, long long number, short states) {
    int finalStatesNumber = number % (int) pow(2, states);
    number /= (long long) pow(2, states);

    len--; // to get to last index of array
    // final states
    while (finalStatesNumber > 0) {
        v.at(len) = finalStatesNumber % 2;
        finalStatesNumber /= 2;
        len--;
    }

    len = ALPHABET_LEN * states - 1;
    // transitions
    while (number > 0) {
        v.at(len) = number % states;
        number /= states;
        len--;
    }
}

void getStructureStateComplexities(const DFA &dfa, vector<::uint8_t> &stateComplexities) {

    for (::uint8_t i = 0; i < dfa.states; i++) {
        queue<::uint8_t> queue;
        queue.push(i);
        unordered_set<::uint8_t> visited = unordered_set<::uint8_t>(2 * dfa.states);
        visited.insert(i);

        while (!queue.empty()) {
            ::uint8_t current = queue.front();
            queue.pop();
            for (::uint8_t alphabetIdx = 0; alphabetIdx < ALPHABET_LEN; alphabetIdx++) {
                ::uint8_t destination = dfa.transitions[current][alphabetIdx];
                if (visited.find(destination) == visited.end()) {
                    queue.push(destination);
                    visited.insert(destination);
                }
            }
        }
        stateComplexities[visited.size() - 1] += dfa.numberOfEquivalentStates[i];
    }
}

void handleFamilyResultsLanguages(::uint8_t languages, long long family, int &maxLanguagesInFamily,
                                  long long &countOfFamiliesWithMaxLanguages, vector<long long> &familiesWithMaxLanguages,
                                  long long* languagesCount) {
    languagesCount[languages - 1]++;
    if (languages > maxLanguagesInFamily) {
        maxLanguagesInFamily = languages;
        countOfFamiliesWithMaxLanguages = 1;
        familiesWithMaxLanguages.clear();
        familiesWithMaxLanguages.push_back(family);
    } else if (languages == maxLanguagesInFamily) {
        (countOfFamiliesWithMaxLanguages)++;
        if (familiesWithMaxLanguages.size() < 10) {
            familiesWithMaxLanguages.push_back(family);
        }
    }
}

void handleFamilyResultsUniqueStateComplexities(vector<::uint8_t> &stateComplexities, long long family,
                                                int &maxUniqueStateComplexitiesInFamily,
                                                long long &countOfFamiliesWithMaxUniqueStateComplexities,
                                                vector<long long> &familiesWithMaxUniqueStateComplexities,
                                                long long* uniqueStateComplexitiesCount) {
    int k = 0;
    for (int c: stateComplexities)
        if (c != 0)
            k++;

    uniqueStateComplexitiesCount[k - 1]++;

    if (k > maxUniqueStateComplexitiesInFamily) {
        maxUniqueStateComplexitiesInFamily = k;
        countOfFamiliesWithMaxUniqueStateComplexities = 1;
        familiesWithMaxUniqueStateComplexities.clear();
        familiesWithMaxUniqueStateComplexities.push_back(family);
    } else if (k == maxUniqueStateComplexitiesInFamily) {
        if (familiesWithMaxUniqueStateComplexities.size() < 50)
            familiesWithMaxUniqueStateComplexities.push_back(family);
        (countOfFamiliesWithMaxUniqueStateComplexities)++;
    }
}
