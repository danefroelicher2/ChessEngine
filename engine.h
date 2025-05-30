#ifndef ENGINE_H
#define ENGINE_H

#include "main.h"
#include "game.h"
#include "transposition.h"
#include "zobrist.h"
#include <mutex>
#include <atomic>

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
    std::vector<std::vector<Move>> pvTable; // Stores PV for each depth

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

    // Time management variables
    int timeAllocated; // time in milliseconds allocated for this move
    int timeBuffer;    // safety buffer to avoid timeout
    bool timeManaged;  // whether to use time management

    // Search instability detection
    bool positionIsUnstable;
    mutable std::mutex timeMutex;
    std::atomic<bool> searchShouldStop{false};
    std::atomic<bool> timeManagementActive{false};
    int unstableExtensionPercent; // Additional percentage of time for unstable positions

public:
    Engine(Game &g, int depth = 3, int ttSizeMB = 64, bool useTimeManagement = false)
        : game(g), maxDepth(depth), transpositionTable(ttSizeMB), nodesSearched(0),
          timeAllocated(0), timeBuffer(100), timeManaged(useTimeManagement),
          positionIsUnstable(false), unstableExtensionPercent(50)
    {
        // Initialize PV table
        pvTable.resize(MAX_PLY);
        for (int i = 0; i < MAX_PLY; i++)
        {
            pvTable[i].clear();
        }

        // Initialize tables
        clearKillerMoves();
        clearHistoryTable();
        clearCounterMoves();
    }

    void setTimeAllocation(int timeInMs)
    {
        timeAllocated = timeInMs;
        timeManaged = true;
    }

    // Set time for this move in milliseconds
    void setTimeForMove(int timeMs)
    {
        timeAllocated = timeMs;
        timeManaged = true;
    }

    // Enable/disable time management
    void setTimeManagement(bool enabled) { timeManaged = enabled; }

    // Check if we should stop searching due to time
    bool shouldStopSearch() const;

    // Set the search depth
    void setDepth(int depth) { maxDepth = depth; }

    // Set transposition table size
    void setTTSize(int sizeMB) { transpositionTable.resize(sizeMB); }

    // Calculate the best move for the current position
    Move getBestMove();

    // Clear the transposition table
    void clearTT() { transpositionTable.clear(); }

    // Clear the killer moves
    void clearKillerMoves();

    // Clear the counter moves
    void clearCounterMoves();

    // Clear the history table
    void clearHistoryTable();

    // Get the principal variation as a string
    std::string getPVString() const;

    // Get the number of nodes searched
    long getNodesSearched() const { return nodesSearched; }

    // Reset search statistics
    void resetStats() { nodesSearched = 0; }

private:

// Add to engine.h private section:
void generateCaptureMoves(const Board& board, std::vector<Move>& captures) const;
void generatePawnCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
void generateKnightCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
void generateBishopCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
void generateRookCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
void generateQueenCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
void generateKingCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
void generateCheckEvasions(const Board& board, std::vector<Move>& evasions) const;
void generatePromotions(const Board& board, std::vector<Move>& promotions) const;

// Add to engine.h private members:
Zobrist zobristHasher;
    // Helper function to get depth adjustment for a move
    int getDepthAdjustment(const Move &move, const Board &board, bool isPVMove, int moveIndex) const;

    // Store PV at a specific depth
    void storePV(int depth, const std::vector<Move> &pv);

    // Check if a move is in the PV at a specific depth and ply
    bool isPVMove(const Move &move, int depth, int ply) const;

    // Iterative deepening search
    Move iterativeDeepeningSearch(Board &board, int maxDepth, uint64_t hashKey);

    // Alpha-beta minimax search algorithm with transposition table
    int alphaBeta(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                  std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove);

    // Principal Variation Search (PVS) - optimization of alpha-beta
    int pvSearch(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                 std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove);

    // Quiescence search for handling captures at leaf nodes
    int quiescenceSearch(Board &board, int alpha, int beta, uint64_t hashKey, int ply);

    // Static Exchange Evaluation (SEE)
    int seeCapture(const Board &board, const Move &move) const;
    int see(const Board &board, const Position &square, Color side, int capture_value) const;

    // Get the approximate value of a piece for SEE
    int getPieceValue(PieceType type) const;

    // Evaluate a board position
    int evaluatePosition(const Board &board);

    // MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) scoring
    int getMVVLVAScore(PieceType attacker, PieceType victim) const;

    int calculateLMRReduction(int depth, int moveIndex, bool foundPV, bool isCapture,
                              bool isCheck, bool isKillerMove) const;

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

    // Check if a move is part of the principal variation (deprecated version for compatibility)
    bool isPVMove(const Move &move, const std::vector<Move> &pv, int ply) const;

    // Piece value tables for positional evaluation
    static const int pawnTable[64];
    static const int knightTable[64];
    static const int bishopTable[64];
    static const int rookTable[64];
    static const int queenTable[64];
    static const int kingMiddleGameTable[64];
    static const int kingEndGameTable[64];
    static const int LMR_MIN_DEPTH = 3;            // Minimum depth to apply LMR
    static const int LMR_MIN_MOVE_INDEX = 3;       // Start reducing after this many moves
    static const double LMR_BASE_REDUCTION = 0.75; // Base reduction factor
    static const double LMR_DEPTH_FACTOR = 0.5;    // How much depth affects reduction
    static const double LMR_MOVE_FACTOR = 0.3;     // How much move index affects reduction

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