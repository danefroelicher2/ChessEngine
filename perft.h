#ifndef PERFT_H
#define PERFT_H

#include "main.h"
#include "board.h"

class PerftTester {
public:
    struct PerftResult {
        uint64_t nodes;
        uint64_t captures;
        uint64_t enPassant;
        uint64_t castles;
        uint64_t promotions;
        uint64_t checks;
        uint64_t checkmates;
        
        PerftResult() : nodes(0), captures(0), enPassant(0), castles(0), 
                       promotions(0), checks(0), checkmates(0) {}
    };
    
    // Run perft test from a given position
    static PerftResult perft(Board& board, int depth);
    
    // Run perft with detailed breakdown (divide)
    static void perftDivide(Board& board, int depth);
    
    // Run standard test suite
    static bool runTestSuite();
    
    // Test individual position
    static bool testPosition(const std::string& fen, int depth, uint64_t expectedNodes);
    
private:
    static PerftResult perftRecursive(Board& board, int depth, bool root = false);
};

#endif // PERFT_H