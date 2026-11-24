//
// Created by Lakshya Mittal on 18-02-2022.
//

#include "CornerDBMaker.h"

using namespace std;

// The BFS below stops at depth 8, so any corner state it never reaches is at
// least 9 moves from solved. Initialising every nibble to 9 (0x99 = two
// nibbles of 9) keeps the heuristic admissible. With the old 0xFF default,
// unreached states read as 15, which over-estimates and breaks IDA*.
CornerDBMaker::CornerDBMaker(string _fileName) : CornerDBMaker(_fileName, 0x99) {}

CornerDBMaker::CornerDBMaker(string _fileName, uint8_t init_val)
        : fileName(_fileName), cornerDB(init_val) {}

bool CornerDBMaker::bfsAndStore() {
    RubiksCubeBitboard cube;
    queue<RubiksCubeBitboard> q;
    q.push(cube);
    cornerDB.setNumMoves(cube, 0);
    int curr_depth = 0;
    while (!q.empty()) {
        int n = q.size();
        curr_depth++;
        if (curr_depth == 9) break;
        for (int counter = 0; counter < n; counter++) {
            RubiksCubeBitboard node = q.front();
            q.pop();
            for (int i = 0; i < 18; i++) {
                auto curr_move = RubiksCube::MOVE(i);
                node.move(curr_move);
                if ((int) cornerDB.getNumMoves(node) > curr_depth) {
                    cornerDB.setNumMoves(node, curr_depth);
                    q.push(node);
                }
                node.invert(curr_move);
            }
        }
    }

    cornerDB.toFile(fileName);
    return true;
}
