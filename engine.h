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
Engine(Game &g, int depth = 3, int ttSizeMB = 64, bool useTimeManagement = false)
    : game(g), maxDepth(depth), transpositionTable(ttSizeMB), nodesSearched(0),
      timeAllocated(0), timeBuffer(100), timeManaged(useTimeManagement),
      positionIsUnstable(false), unstableExtensionPercent(50)
{
    // Initialize tables
    clearKillerMoves();
    clearHistoryTable();
    clearCounterMoves();
}

public:
    void setTimeAllocation(int timeInMs)
    {
        timeAllocated = timeInMs;
        timeManaged = true;
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
    // Time management variables
    int timeAllocated; // time in milliseconds allocated for this move
    int timeBuffer;    // safety buffer to avoid timeout
    bool timeManaged;  // whether to use time management

private:
    // Search instability detection
    bool positionIsUnstable;
    int unstableExtensionPercent; // Additional percentage of time for unstable positions

private:
    // Iterative deepening search
    Move Engine::iterativeDeepeningSearch(Board &board, int maxDepth, uint64_t hashKey)
    {
        principalVariation.clear();
        Move bestMove(Position(), Position());
        int bestScore = 0;

        // For time management
        long nodesPrevious = 0;
        long nodesTotal = 0;

        // For aspiration windows
        int windowSize = 50;

        // Iterative deepening loop
        for (int depth = 1; depth <= maxDepth; depth++)
        {
            std::vector<Move> pv;

            // Record nodes before this iteration
            nodesPrevious = nodesSearched;

            // Color is set to true for maximizing player (WHITE), false for minimizing player (BLACK)
            bool maximizingPlayer = board.getSideToMove() == Color::WHITE;

            int alpha, beta, delta = windowSize;
            int score;

            // For depth 1, use full window
            if (depth == 1)
            {
                alpha = -100000;
                beta = 100000;
                score = pvSearch(board, depth, alpha, beta, maximizingPlayer, pv, hashKey, 0, Move(Position(), Position()));
            }
            else
            {
                // Use aspiration windows for deeper searches
                alpha = bestScore - delta;
                beta = bestScore + delta;

                // Try with narrow window first
                while (true)
                {
                    score = pvSearch(board, depth, alpha, beta, maximizingPlayer, pv, hashKey, 0, Move(Position(), Position()));

                    // If the score falls within our window, we're good
                    if (score > alpha && score < beta)
                    {
                        break;
                    }

                    // If we failed low (score <= alpha), widen the window
                    if (score <= alpha)
                    {
                        alpha = std::max(-100000, alpha - delta);
                        delta *= 2; // Increase window size
                        std::cout << "Aspiration fail low. New alpha: " << alpha << std::endl;
                    }
                    // If we failed high (score >= beta), widen the window
                    else if (score >= beta)
                    {
                        beta = std::min(100000, beta + delta);
                        delta *= 2; // Increase window size
                        std::cout << "Aspiration fail high. New beta: " << beta << std::endl;
                    }

                    // If window is already full, break
                    if (alpha <= -99000 && beta >= 99000)
                    {
                        break;
                    }
                }
            }

            // Store the best move and score if we got valid results
            if (!pv.empty())
            {
                bestMove = pv[0];
                principalVariation = pv;
                bestScore = score;
            }

            // Log the progress
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - searchStartTime);

            // Nodes for this iteration
            long nodesThisIteration = nodesSearched - nodesPrevious;
            nodesTotal = nodesSearched;

            std::cout << "Depth: " << depth
                      << ", Score: " << score
                      << ", Nodes: " << nodesSearched
                      << ", Time: " << duration.count() << "ms"
                      << ", NPS: " << static_cast<long>(nodesSearched * 1000.0 / duration.count())
                      << ", PV: " << getPVString() << std::endl;

            // Time management check
            if (timeManaged && timeAllocated > 0)
            {
                // Check if we should start next iteration
                int timeUsed = duration.count();

                // Estimate time for next iteration: typically 4-5x more nodes required
                long estimatedNodesNext = nodesThisIteration * 4.5;
                double estimatedTimeNext = (double)timeUsed * estimatedNodesNext / nodesThisIteration;

                // If we estimate we'll exceed our time allocation for the next iteration, stop now
                if (timeUsed + estimatedTimeNext + timeBuffer > timeAllocated)
                {
                    std::cout << "Stopping search due to time constraints. Time used: "
                              << timeUsed << "ms, Estimated for next: "
                              << estimatedTimeNext << "ms" << std::endl;
                    break;
                }
            }
        }

        return bestMove;
    }

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