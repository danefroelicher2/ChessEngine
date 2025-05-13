#include "transposition.h"

TranspositionTable::TranspositionTable(int sizeMB) {
    currentAge = 0;
    resize(sizeMB);
}

void TranspositionTable::resize(int sizeMB) {
    // Calculate number of entries
    // Each entry is typically around 16-24 bytes, let's say 20 bytes average
    size_t numEntries = (sizeMB * 1024 * 1024) / sizeof(TTEntry);
    
    // Make it a power of 2 (helps with modulo operations)
    size_t powerOf2 = 1;
    while (powerOf2 < numEntries) {
        powerOf2 *= 2;
    }
    powerOf2 /= 2; // Ensure we don't exceed the requested size
    
    size = powerOf2;
    table.resize(size);
    clear();
}

void TranspositionTable::store(uint64_t key, int depth, int score, NodeType type, const Move& bestMove) {
    size_t idx = index(key);
    TTEntry& entry = table[idx];
    
    // Replacement strategy: 
    // 1. Always replace if the new entry has a deeper search
    // 2. Always replace if the current entry is from an older age
    // 3. Otherwise, keep the existing entry
    
    if (entry.key == 0 || // Empty slot
        depth >= entry.depth || // Deeper search
        currentAge != entry.age) { // Older entry
        
        entry = TTEntry(key, depth, score, type, bestMove, currentAge);
    }
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove) {
    size_t idx = index(key);
    TTEntry& entry = table[idx];
    
    // Check if we have a matching position
    if (entry.key == key) {
        // Always return the best move, even if we can't use the score
        bestMove = entry.bestMove;
        
        // Only use the score if the depth is sufficient
        if (entry.depth >= depth) {
            // Adjust the score based on the node type
            switch (entry.type) {
                case NodeType::EXACT:
                    score = entry.score;
                    return true;
                
                case NodeType::ALPHA:
                    if (entry.score <= alpha) {
                        score = alpha;
                        return true;
                    }
                    break;
                
                case NodeType::BETA:
                    if (entry.score >= beta) {
                        score = beta;
                        return true;
                    }
                    break;
            }
        }
    }
    
    return false;
}

void TranspositionTable::clear() {
    for (size_t i = 0; i < size; i++) {
        table[i] = TTEntry();
    }
    currentAge = 0;
}