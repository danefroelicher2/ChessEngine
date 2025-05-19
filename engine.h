#ifndef ENGINE_H
#define ENGINE_H

#include "main.h"
#include "game.h"
#include "transposition.h"
#include "zobrist.h"

// Maximum search depth - adjust if needed
#define MAX_PLY 64

class Engine {
private:
    int maxDepth;
    Game& game;
    TranspositionTable transpositionTable;
    
    // Killer move tables - stores non-capturing moves that caused beta cutoffs
    // We'll store 2 killer moves per ply
    Move killerMoves[MAX_PLY][2];
    
    // History heuristic table [color][from_square][to_square]
    // Records how often moves lead to beta cutoffs
    int historyTable[2][64][64];
    
public:
    Engine(Game& g, int depth = 3, int ttSizeMB = 64) 
        : game(g), maxDepth(depth), transpositionTable(ttSizeMB) {
        // Initialize killer moves to invalid moves
        clearKillerMoves();
        // Initialize history table
        clearHistoryTable();
    }
    
    // Set the search depth
    void setDepth(int depth) { maxDepth = depth; }
    
    // Set transposition table size
    void setTTSize(int sizeMB) { transpositionTable.resize(sizeMB); }
    
    // Calculate the best move for the current position
    Move getBestMove();
    
    // Clear the transposition table
    void clearTT() { transpositionTable.clear(); }
    
    // Clear the killer moves
    void clearKillerMoves() {
        for (int ply = 0; ply < MAX_PLY; ply++) {
            for (int i = 0; i < 2; i++) {
                killerMoves[ply][i] = Move(Position(), Position());
            }
        }
    }
    
    // Clear the history table
    void clearHistoryTable() {
        for (int color = 0; color < 2; color++) {
            for (int from = 0; from < 64; from++) {
                for (int to = 0; to < 64; to++) {
                    historyTable[color][from][to] = 0;
                }
            }
        }
    }
    
private:
    // Alpha-beta minimax search algorithm with transposition table
    int alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, Move& bestMove, uint64_t hashKey, int ply);
    
    // Evaluate a board position
    int evaluatePosition(const Board& board);
    
    // MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) scoring
    int getMVVLVAScore(PieceType attacker, PieceType victim) const;
    
    // Store a killer move
    void storeKillerMove(const Move& move, int ply);
    
    // Check if a move is a killer move at the current ply
    bool isKillerMove(const Move& move, int ply) const;
    
    // Update history heuristic table for a move that caused beta cutoff
    void updateHistoryScore(const Move& move, int depth, Color color);
    
    // Get the history score for a move
    int getHistoryScore(const Move& move, Color color) const;
    
    // Get score for move ordering
    int getMoveScore(const Move& move, const Board& board, const Move& ttMove, int ply, Color sideToMove) const;
    
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

#endif // ENGINE_H