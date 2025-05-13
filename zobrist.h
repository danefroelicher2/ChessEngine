#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "main.h"
#include "piece.h"

class Zobrist {
private:
    // Random keys for pieces at each position
    // [piece_type][color][position]
    static uint64_t pieceKeys[6][2][64];
    
    // Random key for side to move (when it's black's turn)
    static uint64_t sideToMoveKey;
    
    // Random keys for castling rights
    static uint64_t castlingKeys[4]; // WK, WQ, BK, BQ
    
    // Random keys for en passant files
    static uint64_t enPassantKeys[8];
    
    // Has this been initialized?
    static bool initialized;
    
public:
    // Initialize all random keys
    static void initialize();
    
    // Generate a hash key for a given board position
    static uint64_t generateHashKey(const Board& board);
    
    // Update a hash key when making a move (faster than regenerating)
    static uint64_t updateHashKey(uint64_t currentKey, const Move& move, const Board& board);
};

#endif // ZOBRIST_H