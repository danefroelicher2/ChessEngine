#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "common.h"
#include "piece.h"

class Zobrist {
private:
    // Random keys for pieces at each position
    // [piece_type][color][position]
    uint64_t pieceKeys[6][2][64];
    
    // Random key for side to move (when it's black's turn)
    uint64_t sideToMoveKey;
    
    // Random keys for castling rights
    uint64_t castlingKeys[4]; // WK, WQ, BK, BQ
    
    // Random keys for en passant files
    uint64_t enPassantKeys[8];
    
    // Has this been initialized?
    bool initialized;
    
public:
    // Constructor - initializes all random keys
    Zobrist();
    
    // Initialize all random keys (called by constructor)
    void initialize();
    
    // Generate a hash key for a given board position
    uint64_t generateHashKey(const Board& board);
    
    // Update a hash key when making a move (faster than regenerating)
    uint64_t updateHashKey(uint64_t currentKey, const Move& move, const Board& board);
};

#endif // ZOBRIST_H