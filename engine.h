#ifndef ENGINE_H
#define ENGINE_H

#include "main.h"
#include "game.h"
#include "transposition.h"
#include "zobrist.h"

class Engine {
private:
    int maxDepth;
    Game& game;
    TranspositionTable transpositionTable;
    
public:
    Engine(Game& g, int depth = 3, int ttSizeMB = 64) 
        : game(g), maxDepth(depth), transpositionTable(ttSizeMB) {}
    
    // Set the search depth
    void setDepth(int depth) { maxDepth = depth; }
    
    // Set transposition table size
    void setTTSize(int sizeMB) { transpositionTable.resize(sizeMB); }
    
    // Calculate the best move for the current position
    Move getBestMove();
    
    // Clear the transposition table
    void clearTT() { transpositionTable.clear(); }
    
private:
    // Alpha-beta minimax search algorithm with transposition table
    int alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, Move& bestMove, uint64_t hashKey);
    
    // Evaluate a board position
    int evaluatePosition(const Board& board);
    
    // Piece value tables for positional evaluation
    static const int pawnTable[64];
    static const int knightTable[64];
    static const int bishopTable[64];
    static const int rookTable[64];
    static const int queenTable[64];
    static const int kingMiddleGameTable[64];
    static const int kingEndGameTable[64];
    
    // Piece values
    static const int PAWN_VALUE = 100;
    static const int KNIGHT_VALUE = 320;
    static const int BISHOP_VALUE = 330;
    static const int ROOK_VALUE = 500;
    static const int QUEEN_VALUE = 900;
    static const int KING_VALUE = 20000;
    
    // Check if the game is in the endgame phase
    bool isEndgame(const Board& board) const;
};