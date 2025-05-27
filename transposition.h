#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "main.h"
#include "piece.h"

// Node types for transposition table entries
enum class NodeType
{
    EXACT, // Exact score
    ALPHA, // Upper bound (fail-low)
    BETA   // Lower bound (fail-high)
};

// Structure for transposition table entries
struct TTEntry
{
    uint64_t key;  // Zobrist hash key
    int depth;     // Depth of the search
    int score;     // Score of the position
    NodeType type; // Type of node (exact, alpha, beta)
    Move bestMove; // Best move from this position
    int age;       // Age of the entry (for replacement strategy)

    TTEntry() : key(0), depth(0), score(0), type(NodeType::EXACT), bestMove(Position(), Position()), age(0) {}

    TTEntry(uint64_t k, int d, int s, NodeType t, Move bm, int a)
        : key(k), depth(d), score(s), type(t), bestMove(bm), age(a) {}
};

class TranspositionTable
{
private:
    std::vector<TTEntry> table;
    size_t size;
    int currentAge;
    int calculateReplacementScore(const TTEntry &entry, int currentAge) const;

public:
    // Constructor with table size (power of 2)
    TranspositionTable(int sizeMB = 64);

    // Resize the table
    void resize(int sizeMB);

    // Store a position in the table
    void store(uint64_t key, int depth, int score, NodeType type, const Move &bestMove);

    // Probe the table for a position
    bool probe(uint64_t key, int depth, int alpha, int beta, int &score, Move &bestMove);

    // Clear the table
    void clear();

    // Increment the age (typically done at the start of a new search)
    void incrementAge() { currentAge++; }

    // Get the current age
    int getAge() const { return currentAge; }

    // Get table size
    size_t getSize() const { return size; }

    // Calculate the index in the table for a given key
    size_t index(uint64_t key) const { return key % size; }
};

#endif // TRANSPOSITION_H