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

// REPLACE the entire store() method in transposition.cpp with this:
void TranspositionTable::store(uint64_t key, int depth, int score, NodeType type, const Move& bestMove) 
{
    size_t idx = index(key);
    TTEntry& entry = table[idx];
    
    // IMPROVED replacement strategy with multiple factors
    
    // 1. Always replace empty slots
    if (entry.key == 0) {
        entry = TTEntry(key, depth, score, type, bestMove, currentAge);
        return;
    }
    
    // 2. Always replace if same position (update with new info)
    if (entry.key == key) {
        // Only replace if new search is deeper or same depth with newer age
        if (depth >= entry.depth || currentAge > entry.age) {
            entry = TTEntry(key, depth, score, type, bestMove, currentAge);
        }
        return;
    }
    
    // 3. Complex replacement decision for different positions
    // Calculate replacement scores for old and new entries
    int oldScore = calculateReplacementScore(entry, currentAge);
    int newScore = calculateReplacementScore(TTEntry(key, depth, score, type, bestMove, currentAge), currentAge);
    
    // Replace if new entry scores higher
    if (newScore > oldScore) {
        entry = TTEntry(key, depth, score, type, bestMove, currentAge);
    }
}

// ADD this helper method to transposition.cpp:
int TranspositionTable::calculateReplacementScore(const TTEntry& entry, int currentAge) const
{
    int score = 0;
    
    // Bonus for depth (deeper searches are more valuable)
    score += entry.depth * 100;
    
    // Penalty for age (older entries are less valuable)
    int ageDiff = currentAge - entry.age;
    score -= ageDiff * 50;
    
    // Bonus for exact scores (more valuable than bounds)
    if (entry.type == NodeType::EXACT) {
        score += 200;
    }
    
    // Bonus for having a best move
    if (entry.bestMove.from.isValid() && entry.bestMove.to.isValid()) {
        score += 150;
    }
    
    return score;
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