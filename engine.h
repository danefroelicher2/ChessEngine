#ifndef ENGINE_H
#define ENGINE_H

#include "main.h"
#include "game.h"
#include "transposition.h"
#include "zobrist.h"

// Maximum search depth - adjust if needed
#define MAX_PLY 64
#define MAX_QSEARCH_DEPTH 8

class Engine
{
private:
    int maxDepth;
    Game &game;
    TranspositionTable transpositionTable;

    // Principal Variation (PV) storage
    std::vector<Move> principalVariation;

    // Killer move tables - stores non-capturing moves that caused beta cutoffs
    // We'll store 2 killer moves per ply
    Move killerMoves[MAX_PLY][2];

    // Counter move heuristic table [piece_type][color][from_square][to_square]
    // Stores moves that were effective against specific opponent moves
    Move counterMoves[6][2][64][64];

    // History heuristic table [color][from_square][to_square]
    // Records how often moves lead to beta cutoffs
    int historyTable[2][64][64];

    // Search statistics
    long nodesSearched;
    std::chrono::time_point<std::chrono::high_resolution_clock> searchStartTime;

public:
    Engine(Game &g, int depth = 3, int ttSizeMB = 64)
        : game(g), maxDepth(depth), transpositionTable(ttSizeMB), nodesSearched(0)
    {
        // Initialize tables
        clearKillerMoves();
        clearHistoryTable();
        clearCounterMoves();
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
    void clearKillerMoves()
    {
        for (int ply = 0; ply < MAX_PLY; ply++)
        {
            for (int i = 0; i < 2; i++)
            {
                killerMoves[ply][i] = Move(Position(), Position());
            }
        }
    }

    // Clear the counter moves
    void clearCounterMoves()
    {
        for (int pieceType = 0; pieceType < 6; pieceType++)
        {
            for (int color = 0; color < 2; color++)
            {
                for (int from = 0; from < 64; from++)
                {
                    for (int to = 0; to < 64; to++)
                    {
                        counterMoves[pieceType][color][from][to] = Move(Position(), Position());
                    }
                }
            }
        }
    }

    // Clear the history table
    void clearHistoryTable()
    {
        for (int color = 0; color < 2; color++)
        {
            for (int from = 0; from < 64; from++)
            {
                for (int to = 0; to < 64; to++)
                {
                    historyTable[color][from][to] = 0;
                }
            }
        }
    }

    // Get the principal variation as a string
    std::string getPVString() const;

    // Get the number of nodes searched
    long getNodesSearched() const { return nodesSearched; }

    // Reset search statistics
    void resetStats() { nodesSearched = 0; }

private:
    // Iterative deepening search
    Move iterativeDeepeningSearch(Board &board, int maxDepth, uint64_t hashKey);

    // Alpha-beta minimax search algorithm with transposition table
    int alphaBeta(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                  std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove);

    // Principal Variation Search (PVS) - optimization of alpha-beta
    int pvSearch(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                 std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove);

    // Quiescence search for handling captures at leaf nodes
int quiescenceSearch(Board& board, int alpha, int beta, uint64_t hashKey, int ply, int qDepth);

    // Static Exchange Evaluation (SEE)
    int seeCapture(const Board &board, const Move &move) const;
    int see(const Board &board, const Position &square, Color side, int capture_value) const;

    // Get the approximate value of a piece for SEE
    int getPieceValue(PieceType type) const;

    // Evaluate a board position
    int evaluatePosition(const Board &board);

    // MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) scoring
    int getMVVLVAScore(PieceType attacker, PieceType victim) const;

    // Store a killer move
    void storeKillerMove(const Move &move, int ply);

    // Check if a move is a killer move at the current ply
    bool isKillerMove(const Move &move, int ply) const;

    // Store a counter move
    void storeCounterMove(const Move &lastMove, const Move &counterMove);

    // Get counter move for an opponent's move
    Move getCounterMove(const Move &lastMove) const;

    // Update history heuristic table for a move that caused beta cutoff
    void updateHistoryScore(const Move &move, int depth, Color color);

    // Get the history score for a move
    int getHistoryScore(const Move &move, Color color) const;

    // Get score for move ordering
    int getMoveScore(const Move &move, const Board &board, const Move &ttMove,
                     const std::vector<Move> &pv, int ply, Color sideToMove,
                     const Move &lastMove) const;

    // Check if a move is part of the principal variation
    bool isPVMove(const Move &move, const std::vector<Move> &pv, int ply) const;

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
    bool isEndgame(const Board &board) const;
};

#endif // ENGINE_H