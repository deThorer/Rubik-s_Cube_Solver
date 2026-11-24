# Rubik's Cube Solver

A C++ Rubik's Cube solver that models the cube three different ways and solves it with four search algorithms: **DFS**, **BFS**, **IDDFS**, and **IDA\*** guided by a **corner pattern database** heuristic.

---

## Features

- Three interchangeable cube representations behind one abstract interface:
  - `RubiksCube3dArray` — `char cube[6][3][3]`, the most readable
  - `RubiksCube1dArray` — `char cube[54]`, flattened with index math
  - `RubiksCubeBitboard` — six `uint64_t`, one per face, the fastest
- All 18 face turns in half-turn metric: `U D L R F B`, each as `X`, `X'`, `X2`
- Four solvers, written as templates so any representation can be plugged in:
  - **DFS** (depth-limited), **BFS** (optimal, memory-heavy), **IDDFS** (optimal, low memory), **IDA\*** (heuristic)
- **Corner pattern database**: 88,179,840 corner states (8! × 3^7) stored as 4-bit entries in a 50 MB `NibbleArray`, indexed with a **Lehmer-code permutation rank**
- Command-line driver that scrambles, solves, times, and verifies the solution

---

## Project Structure

```
Rubik-s_Cube_Solver/
├── Model/
│   ├── RubiksCube.h / .cpp         # Abstract base class: enums, move dispatch, printing, corner helpers
│   ├── RubiksCube3dArray.cpp       # 6x3x3 char array model + Hash3d
│   ├── RubiksCube1dArray.cpp       # 54-char array model + Hash1d
│   └── RubiksCubeBitboard.cpp      # 6 x uint64 bitboard model + HashBitboard
├── Solver/
│   ├── DFSSolver.h                 # Depth-limited DFS with backtracking
│   ├── BFSSolver.h                 # BFS with visited set and parent-move map
│   ├── IDDFSSolver.h               # Iterative deepening over DFSSolver
│   └── IDAstarSolver.h             # Bounded best-first search with the corner heuristic
├── PatternDatabases/
│   ├── NibbleArray.h / .cpp        # Packs two 4-bit values per byte
│   ├── PatternDatabase.h / .cpp    # Abstract DB: get/set moves, save/load file
│   ├── CornerPatternDatabase.*     # Corner-state indexing (permutation rank x orientation)
│   ├── CornerDBMaker.*             # BFS that fills the corner DB and writes it to disk
│   ├── PermutationIndexer.h        # O(n) Lehmer-code ranking of a permutation
│   └── math.h / .cpp               # factorial, nPk, nCk
├── main.cpp                        # CLI: scramble -> solve -> verify
└── CMakeLists.txt
```

The cube model `.cpp` files define their classes entirely inline and are `#include`d by the solvers (they carry `#pragma once`), so CMake compiles only the real translation units.

---

## Build & Run

Requires a C++17 compiler (GCC or MinGW, because the code uses `<bits/stdc++.h>`) and CMake 3.16+.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/rubiks_cube_solver --solver ida --scramble 12
```

Options:

| Flag | Default | Meaning |
|------|---------|---------|
| `--solver` | `ida` | `ida`, `iddfs`, `bfs` or `dfs` |
| `--scramble` | `8` | Number of random moves applied to the solved cube |
| `--db` | `Databases/cornerDepth8.bin` | Corner pattern database file |

The first IDA\* run builds the corner database by BFS (a few minutes, ~3 GB peak RAM) and saves it; later runs load the 50 MB file in under a second.

### Example output

```
Scramble (12 moves): B U D' U' R2 D2 B2 F2 L F2 U' L
Solver: ida
Solution (10 moves): L' U F2 L' F2 B2 D2 R2 D B'
Time: 0.105 s
Verified: cube is solved.
```

(The random scramble can contain moves that cancel out, so the solution is often shorter than the scramble.)

---

## How It Works

### Cube representation

Faces are indexed `UP=0, LEFT=1, FRONT=2, RIGHT=3, BACK=4, DOWN=5` and colours `WHITE, GREEN, RED, BLUE, ORANGE, YELLOW` in the same order, so the solved cube has colour *i* on face *i*. Each model implements the 18 moves; `X'` is three `X` turns and `X2` is two.

In the **bitboard**, each face is one `uint64_t`. The 8 non-centre stickers are stored clockwise (positions 0–7), 8 bits each, with the colour one-hot encoded (`1 << colour`). Centres never move, so they are not stored. Rotating a face clockwise is then a 16-bit rotate of that word, and moving a row of three stickers between faces is masking and shifting 24 bits.

### Solvers

- **DFS**: tries all 18 moves recursively up to a depth limit, undoing each with `invert()` on the way back. Fast but not optimal.
- **BFS**: explores level by level with a `visited` hash map and a `move_done` map (the move that produced each state). Optimal, but memory grows as 18^d, so it is practical only to about 6–7 moves.
- **IDDFS**: runs DFS with limits 1, 2, 3, … — BFS's optimality with DFS's memory.
- **IDA\***: each iteration uses a priority queue ordered by f = g + h (ties broken by smaller h) and prunes any node whose f exceeds the current bound. The smallest pruned f becomes the next bound. The path is rebuilt from `move_done` the same way as in BFS. Strictly, this is a bounded best-first (A\*-style) search rather than the textbook recursive IDA\*.

### Heuristic: corner pattern database

h(cube) = the number of moves needed to solve just the 8 corners. Solving the whole cube also solves the corners, so this never overestimates (it is **admissible**).

- **Index**: corner permutation rank (0 … 8! − 1, via a Lehmer code) × 3^7 + orientation of 7 corners in base 3. The 8th orientation is determined by the other seven (the sum is always 0 mod 3), which is why 3^7 and not 3^8.
- **Storage**: each distance fits in 4 bits, so two entries share a byte: 88,179,840 / 2 ≈ 44 MB of data.
- **Generation**: BFS from the solved cube, recording the first depth each corner state is reached, up to depth 8. Unreached entries are initialised to 9 (`0x99`), still a valid lower bound.

---

## Complexity notes

- Branching factor 18; the full cube has about 4.3 × 10^19 states, and any position can be solved in 20 moves or fewer (God's number, half-turn metric).
- BFS/IDDFS are only practical for short scrambles; IDA\* with the corner database handles about 12–14 move scrambles in seconds.
- Next steps for a stronger solver: edge pattern databases (the Korf approach, taking the max of corner and edge heuristics), pruning redundant move pairs (e.g. `R` after `R`, or `L R` vs `R L`), and Kociemba's two-phase algorithm.

---

## Acknowledgements

The source files' headers credit Lakshya Mittal as the original author of the cube models, solvers, and pattern database code.
