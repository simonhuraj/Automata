# Automata

A C++11 tool for **exhaustively enumerating small deterministic finite automata (DFAs)** built from families of base transition systems. Each family is turned into a DFA via **subset construction**, then **minimized** with Hopcroft’s algorithm. The program collects statistics on state complexity and writes per-thread result files.

## What it does

For a fixed number of base states `n` and alphabet size `k`, the program iterates over every possible family:

- **Transitions** — for each base state and each alphabet symbol, a target state in `0 … n−1`
- **Final states** — a bitmask over which base states are accepting

For each family it:

1. Builds a DFA (powerset construction; the empty subset is excluded)
2. Minimizes the DFA
3. Computes **state complexity** (reachable-set sizes per state, weighted by equivalence class size)
4. Aggregates summary statistics and writes detailed output to `result{thread}.txt`

The search space size is:

```text
n^(n × k) × 2^n
```

Larger `n` or `k` grows very quickly — start small when experimenting.

## Features

- **Subset construction** from encoded families (`subset_construction.cpp`)
- **DFA minimization** via Hopcroft’s algorithm (`minimizer.cpp`)
- **State complexity** metrics per minimized automaton
- **Parallel enumeration** — split the family range across threads (`main.cpp`)
- **Timing** — total run duration printed to stdout
- **Per-thread output** — `result0.txt`, `result1.txt`, … with per-family lines and aggregate summaries

## Requirements

- C++11 compiler (GCC, Clang, or MSVC)
- [CMake](https://cmake.org/) 3.24+

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The executable is `build/automata` (or `build\Debug\automata.exe` on Windows with some generators).

### CLion / IDE

Open the project folder and use the bundled CMake profile (e.g. `cmake-build-debug`).

## Run

From the project directory (so `result*.txt` are written where you expect):

```bash
./build/automata
```

On Windows:

```powershell
.\build\Debug\automata.exe
```

Stdout shows elapsed time in milliseconds and per-thread before/after minimization totals.

## Output

Each thread writes `result{N}.txt` containing:

- `familyNumber-summedStateComplexity` per family
- Aggregated state-complexity histograms
- Counts of families with maximum minimized state count (labeled “languages” in code)
- Families with the widest range of unique state complexities

## Customization

Tune the run by editing two files, then rebuild.

### `structures.h` — capacity and alphabet

| Constant | Role |
|----------|------|
| `MAX_STATES` | Number of **base** states `n` used in `main` (must match your intended search size) |
| `MAX_STATES_SUBSET` | Maximum number of **DFA states** after subset construction (array sizes for transitions, finals, etc.) |
| `ALPHABET_LEN` | Number of symbols in the alphabet (`k`) |

**Important sizing rule:** after subset construction (without the empty subset), there are at most `2^n − 1` DFA states. Set:

```text
MAX_STATES_SUBSET >= 2^MAX_STATES
```

Using `2^MAX_STATES` is a simple safe choice (e.g. `MAX_STATES = 4` → `MAX_STATES_SUBSET = 16`).

`ALPHABET_LEN` is used in transition tables and in the family encoding length `(ALPHABET_LEN + 1) × n`. The `DFA::alphabet` character labels are currently fixed to `'a'`, `'b'` for `k = 2`; if you increase `ALPHABET_LEN`, extend that array and any logic that assumes two symbols.

Example for 4 base states and a binary alphabet:

```cpp
constexpr uint8_t MAX_STATES = 4;
constexpr uint8_t MAX_STATES_SUBSET = 16;  // 2^4
constexpr uint8_t ALPHABET_LEN = 2;
```

### `main.cpp` — parallelism and range

| Setting | Location | Role |
|---------|----------|------|
| `threadsCount` | `main()` | Number of worker threads; each processes a disjoint slice of families |
| `offsetFamily` | `main()` | Skip the first N family indices (resume / partial runs) |
| `limit` | `main()` | By default, full space `n^(n×k) × 2^n`; you can replace with a smaller constant for testing |

Example — use 8 cores:

```cpp
const int threadsCount = 8;
```

Thread `i` writes `result{i}.txt`. Family index ranges are split evenly; the last thread takes any remainder.

`states` in `main` is taken from `MAX_STATES` in `structures.h` — keep them in sync.

## Project layout

```text
main.cpp                 Enumeration, metrics, threading
structures.h / .cpp      DFA definition and I/O
subset_construction.cpp  Family → DFA (powerset)
subset_construction.h
minimizer.cpp / .h       Hopcroft minimization
CMakeLists.txt           Build configuration
```

## Tips

- **Runtime:** `n = 4` and `k = 2` is about 1M families; `n = 5` is billions — use a smaller `limit` or fewer states while developing.
- **Memory:** large `MAX_STATES_SUBSET` increases fixed stack/array usage in each `DFA`.
- **Precision:** `limit` uses `pow()` in `double`; for large exponents prefer an integer formula or a capped `limit` for experiments.

