#include "engine.h"
#include <chrono>
#include <limits>
#include <algorithm>
#include <sstream>

// Initialize piece-square tables
// These tables provide positional bonuses for pieces on specific squares
const int Engine::pawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5, 5, 10, 25, 25, 10, 5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0};

const int Engine::knightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};

const int Engine::bishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 5, 5, 5, 5, -10,
    -10, 0, 5, 0, 0, 5, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};

const int Engine::rookTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0};

const int Engine::queenTable[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20};

const int Engine::kingMiddleGameTable[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20, 0, 0, 0, 0, 20, 20,
    20, 30, 10, 0, 0, 10, 30, 20};

const int Engine::kingEndGameTable[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10, 0, 0, -10, -20, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -30, 0, 0, 0, 0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50};

// Clear the killer moves
void Engine::clearKillerMoves()
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
void Engine::clearCounterMoves()
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
// Add this method to engine.cpp
void Engine::generateCaptureMoves(const Board& board, std::vector<Move>& captures) const
{
    captures.clear();
    
    Color sideToMove = board.getSideToMove();
    Position enPassantTarget = board.getEnPassantTarget();
    
    // Loop through all squares to find our pieces
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position from(row, col);
            auto piece = board.getPieceAt(from);
            
            if (!piece || piece->getColor() != sideToMove) continue;
            
            // Generate captures for each piece type
            switch (piece->getType()) {
                case PieceType::PAWN:
                    generatePawnCaptures(board, from, captures);
                    break;
                    
                case PieceType::KNIGHT:
                    generateKnightCaptures(board, from, captures);
                    break;
                    
                case PieceType::BISHOP:
                    generateBishopCaptures(board, from, captures);
                    break;
                    
                case PieceType::ROOK:
                    generateRookCaptures(board, from, captures);
                    break;
                    
                case PieceType::QUEEN:
                    generateQueenCaptures(board, from, captures);
                    break;
                    
                case PieceType::KING:
                    generateKingCaptures(board, from, captures);
                    break;
                    
                default:
                    break;
            }
        }
    }
}

// Add these helper methods to engine.cpp as well:

void Engine::generatePawnCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::PAWN) return;
    
    Color color = piece->getColor();
    int direction = (color == Color::WHITE) ? 1 : -1;
    
    // Diagonal captures
    for (int dCol : {-1, 1}) {
        Position to(from.row + direction, from.col + dCol);
        
        if (!to.isValid()) continue;
        
        auto target = board.getPieceAt(to);
        
        // Regular capture
        if (target && target->getColor() != color) {
            // Check for promotion
            if (to.row == 0 || to.row == 7) {
                captures.emplace_back(from, to, PieceType::QUEEN);
                captures.emplace_back(from, to, PieceType::ROOK);
                captures.emplace_back(from, to, PieceType::BISHOP);
                captures.emplace_back(from, to, PieceType::KNIGHT);
            } else {
                captures.emplace_back(from, to);
            }
        }
        // En passant capture
        else if (to == board.getEnPassantTarget()) {
            captures.emplace_back(from, to);
        }
    }
}

void Engine::generateKnightCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::KNIGHT) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> knightMoves = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    for (const auto& offset : knightMoves) {
        Position to(from.row + offset.first, from.col + offset.second);
        
        if (!to.isValid()) continue;
        
        auto target = board.getPieceAt(to);
        if (target && target->getColor() != color) {
            captures.emplace_back(from, to);
        }
    }
}

void Engine::generateBishopCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::BISHOP) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> directions = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    
    for (const auto& dir : directions) {
        for (int distance = 1; distance < 8; distance++) {
            Position to(from.row + dir.first * distance, from.col + dir.second * distance);
            
            if (!to.isValid()) break;
            
            auto target = board.getPieceAt(to);
            if (target) {
                if (target->getColor() != color) {
                    captures.emplace_back(from, to);
                }
                break; // Can't go further
            }
        }
    }
}

void Engine::generateRookCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::ROOK) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (const auto& dir : directions) {
        for (int distance = 1; distance < 8; distance++) {
            Position to(from.row + dir.first * distance, from.col + dir.second * distance);
            
            if (!to.isValid()) break;
            
            auto target = board.getPieceAt(to);
            if (target) {
                if (target->getColor() != color) {
                    captures.emplace_back(from, to);
                }
                break; // Can't go further
            }
        }
    }
}

void Engine::generateQueenCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    // Queen moves like both rook and bishop
    generateRookCaptures(board, from, captures);
    generateBishopCaptures(board, from, captures);
}

void Engine::generateKingCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::KING) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    for (const auto& dir : directions) {
        Position to(from.row + dir.first, from.col + dir.second);
        
        if (!to.isValid()) continue;
        
        auto target = board.getPieceAt(to);
        if (target && target->getColor() != color) {
            captures.emplace_back(from, to);
        }
    }
}

// Clear the history table
void Engine::clearHistoryTable()
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

// Fixed LMR reduction calculation
int Engine::calculateLMRReduction(int depth, int moveIndex, bool foundPV, bool isCapture,
                                  bool isCheck, bool isKillerMove) const
{
    // Don't reduce if depth is too shallow
    if (depth < LMR_MIN_DEPTH)
    {
        return 0;
    }

    // Don't reduce the first few moves
    if (moveIndex < LMR_MIN_MOVE_INDEX)
    {
        return 0;
    }

    // Don't reduce the PV move (first move when foundPV is false)
    if (!foundPV)
    {
        return 0; // This is the PV move, don't reduce
    }

    // Don't reduce captures, checks, or killer moves
    if (isCapture || isCheck || isKillerMove)
    {
        return 0;
    }

    // Calculate base reduction using logarithmic formula
    double logDepth = std::log(static_cast<double>(depth));
    double logMoveIndex = std::log(static_cast<double>(moveIndex + 1));

    double reduction = LMR_BASE_REDUCTION +
                       (logDepth * LMR_DEPTH_FACTOR) +
                       (logMoveIndex * LMR_MOVE_FACTOR);

    // Convert to integer and clamp between 1 and 3
    int intReduction = static_cast<int>(std::round(reduction));
    intReduction = std::max(1, std::min(3, intReduction));

    return intReduction;
}

// Store PV at a specific depth
void Engine::storePV(int depth, const std::vector<Move> &pv)
{
    if (depth >= 0 && depth < MAX_PLY)
    {
        pvTable[depth] = pv;
    }
}

// Check if a move is in the PV at a specific depth and ply
bool Engine::isPVMove(const Move &move, int depth, int ply) const
{
    if (depth < 0 || depth >= MAX_PLY || ply >= static_cast<int>(pvTable[depth].size()))
    {
        return false;
    }

    const Move &pvMove = pvTable[depth][ply];
    return (pvMove.from.row == move.from.row &&
            pvMove.from.col == move.from.col &&
            pvMove.to.row == move.to.row &&
            pvMove.to.col == move.to.col);
}

// Get the best move for the current position
Move Engine::getBestMove()
{
    // Reset search statistics
    resetStats();
    searchStartTime = std::chrono::high_resolution_clock::now();

    // Get a copy of the board
    Board board = game.getBoard();

    // Increment transposition table age
    transpositionTable.incrementAge();

    // Initialize Zobrist hashing
    uint64_t hashKey = zobristHasher.generateHashKey(board);

    // Use iterative deepening to find the best move
    return iterativeDeepeningSearch(board, maxDepth, hashKey);
}

// Iterative deepening search
Move Engine::iterativeDeepeningSearch(Board &board, int maxDepth, uint64_t hashKey)
{
    principalVariation.clear();
    Move bestMove(Position(), Position());
    Move previousBestMove(Position(), Position());
    int bestScore = 0;
    int previousScore = 0;

    // For time management
    long nodesPrevious = 0;

    // For aspiration windows
    int windowSize = 50;

    // For instability detection
    int bestMoveChanges = 0;
    int scoreSwings = 0;
    bool isUnstable = false;

    // Initialize PV table for each depth
    for (int i = 0; i < MAX_PLY; i++)
    {
        pvTable[i].clear();
    }

    // Iterative deepening loop
    for (int depth = 1; depth <= maxDepth; depth++)
    {
        std::vector<Move> pv;

        // Record nodes before this iteration
        nodesPrevious = nodesSearched;

        // Store previous iteration's results
        previousBestMove = bestMove;
        previousScore = bestScore;

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
            bestScore = score;
            principalVariation = pv;

            // Store this iteration's PV
            storePV(depth, pv);
        }

        // Instability detection
        if (depth >= 2)
        {
            // Check if best move changed
            if (bestMove.from.row != previousBestMove.from.row ||
                bestMove.from.col != previousBestMove.from.col ||
                bestMove.to.row != previousBestMove.to.row ||
                bestMove.to.col != previousBestMove.to.col)
            {
                bestMoveChanges++;
                std::cout << "Best move changed from " << previousBestMove.toString()
                          << " to " << bestMove.toString() << std::endl;
            }

            // Check for significant evaluation swings
            const int SCORE_SWING_THRESHOLD = 50; // Centipawns
            if (std::abs(bestScore - previousScore) > SCORE_SWING_THRESHOLD)
            {
                scoreSwings++;
                std::cout << "Score swing detected: " << previousScore
                          << " -> " << bestScore << std::endl;
            }

            // Determine if position is unstable
            isUnstable = (bestMoveChanges >= 2 || scoreSwings >= 1) && depth >= 3;

            if (isUnstable && !positionIsUnstable)
            {
                std::cout << "Position detected as unstable. Allocating more time." << std::endl;
                positionIsUnstable = true;
            }
        }

        // Log the progress
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - searchStartTime);

        // Nodes for this iteration
        long nodesThisIteration = nodesSearched - nodesPrevious;

        std::cout << "Depth: " << depth
                  << ", Score: " << score
                  << ", Nodes: " << nodesSearched
                  << ", Time: " << duration.count() << "ms";

        if (duration.count() > 0)
        {
            std::cout << ", NPS: " << static_cast<long>(nodesSearched * 1000.0 / duration.count());
        }

        std::cout << ", PV: " << getPVString() << std::endl;

        // Time management check - decide whether to continue to next depth
        if (timeManaged && timeAllocated > 0)
        {
            int timeUsed = duration.count();

            // Calculate adjusted time allocation based on position stability
            int adjustedTimeAllocation = timeAllocated;
            if (positionIsUnstable)
            {
                adjustedTimeAllocation += (timeAllocated * unstableExtensionPercent) / 100;
                std::cout << "Extending time allocation to " << adjustedTimeAllocation
                          << "ms due to position instability" << std::endl;
            }

            // Stop if we've used most of our time
            if (timeUsed >= adjustedTimeAllocation - timeBuffer)
            {
                std::cout << "Stopping search due to time limit. Time used: "
                          << timeUsed << "ms" << std::endl;
                break;
            }

            // Estimate time for next iteration: typically 4-5x more nodes required
            if (nodesThisIteration > 0 && depth > 1)
            {
                double branchingFactor = 4.5; // Conservative estimate
                long estimatedNodesNext = static_cast<long>(nodesThisIteration * branchingFactor);
                double estimatedTimeNext = (double)timeUsed * estimatedNodesNext / nodesThisIteration;

                // If we estimate we'll exceed our time allocation, stop now
                if (timeUsed + estimatedTimeNext + timeBuffer > adjustedTimeAllocation)
                {
                    std::cout << "Stopping search due to time estimation. Time used: "
                              << timeUsed << "ms, Estimated for next: "
                              << estimatedTimeNext << "ms" << std::endl;
                    break;
                }
            }
        }
    }

    return bestMove;
}

// DELETE the duplicate time management code that comes after this (around 20-30 lines)

// Get the principal variation as a string
std::string Engine::getPVString() const
{
    std::stringstream ss;

    for (size_t i = 0; i < principalVariation.size(); i++)
    {
        ss << principalVariation[i].toString() << " ";
    }

    return ss.str();
}

// Get depth adjustment for move ordering and late move reductions
int Engine::getDepthAdjustment(const Move &move, const Board &board, bool isPVMove, int moveIndex) const
{
    // For PV moves, no reduction
    if (isPVMove)
    {
        return 0;
    }

    // Losing captures shouldn't get full depth
    auto capturedPiece = board.getPieceAt(move.to);
    if (capturedPiece && seeCapture(board, move) < 0)
    {
        return -1;
    }

    // First few moves should get full depth
    if (moveIndex < 3)
    {
        return 0;
    }

    // Late moves get progressively less depth
    int reduction = 0;

    // Base reduction on move index
    if (moveIndex >= 3)
    {
        reduction = 1;
    }
    if (moveIndex >= 6)
    {
        reduction = 2;
    }
    if (moveIndex >= 12)
    {
        reduction = 3;
    }

    // Don't reduce too much at low depths
    reduction = std::min(reduction, 2);

    return -reduction;
}

// Static Exchange Evaluation (SEE)
int Engine::seeCapture(const Board &board, const Move &move) const
{
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece)
    {
        return 0; // Invalid move - no piece at source
    }

    auto capturedPiece = board.getPieceAt(move.to);
    if (!capturedPiece)
    {
        // Check for en passant
        if (movingPiece->getType() == PieceType::PAWN &&
            move.to == board.getEnPassantTarget())
        {
            return PAWN_VALUE; // En passant captures a pawn
        }
        return 0; // Not a capture
    }

    // REMOVED DUPLICATE LINE HERE - movingPiece already declared above

    // Create a temporary board with the capture made
    Board tempBoard = board;
    tempBoard.setPieceAt(move.from, nullptr);
    tempBoard.setPieceAt(move.to, movingPiece);

    int captureValue = getPieceValue(capturedPiece->getType());
    int attackerValue = getPieceValue(movingPiece->getType());

    // Calculate what happens if the opponent recaptures
    int opponentResponse = see(tempBoard, move.to, movingPiece->getColor(), attackerValue);

    return captureValue - opponentResponse;
}

// Static Exchange Evaluation - simulates a sequence of captures on a square
int Engine::see(const Board &board, const Position &square, Color side, int captureValue) const
{
    // Find all attackers of the given square, sorted by piece value (least valuable first)
    std::vector<std::pair<int, Position>> attackers;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);

            if (piece && piece->getColor() != side)
            {
                // Check if this piece can attack the target square
                auto moves = piece->getLegalMoves(board);

                for (const auto &move : moves)
                {
                    if (move.to == square)
                    {
                        int pieceValue = getPieceValue(piece->getType());
                        attackers.emplace_back(pieceValue, pos);
                        break; // Only need to know it can attack, not how many ways
                    }
                }
            }
        }
    }

    // If no attackers, the previous capture stands
    if (attackers.empty())
    {
        return 0;
    }

    // Sort attackers by piece value (least valuable first for optimal play)
    std::sort(attackers.begin(), attackers.end());

    // Take the least valuable attacker
    int attackerValue = attackers[0].first;

    // Create a temporary board to simulate the capture
    Board tempBoard = board;

    // Remove the attacking piece and place it on the target square
    tempBoard.setPieceAt(attackers[0].second, nullptr);
    auto attackingPiece = board.getPieceAt(attackers[0].second);
    tempBoard.setPieceAt(square, attackingPiece);

    // Recursively calculate the score if the opponent recaptures
    // Note: we flip the side and negate the result
    int opponentResponse = see(tempBoard, square, (side == Color::WHITE) ? Color::BLACK : Color::WHITE, attackerValue);

    // The score is: what we capture minus what the opponent gets back
    return std::max(0, captureValue - opponentResponse);
}

// Get the approximate value of a piece for SEE
int Engine::getPieceValue(PieceType type) const
{
    switch (type)
    {
    case PieceType::PAWN:
        return PAWN_VALUE;
    case PieceType::KNIGHT:
        return KNIGHT_VALUE;
    case PieceType::BISHOP:
        return BISHOP_VALUE;
    case PieceType::ROOK:
        return ROOK_VALUE;
    case PieceType::QUEEN:
        return QUEEN_VALUE;
    case PieceType::KING:
        return KING_VALUE;
    default:
        return 0;
    }
}

// MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) scoring for move ordering
int Engine::getMVVLVAScore(PieceType attacker, PieceType victim) const
{
    // A simple 6x6 matrix for MVV-LVA
    // Victim (Most Valuable) on top, Attacker (Least Valuable) on left side
    static const int mvvLvaScores[6][6] = {
        // Pawn, Knight, Bishop, Rook, Queen, King (victim)
        {105, 205, 305, 405, 505, 605}, // Pawn (attacker)
        {104, 204, 304, 404, 504, 604}, // Knight
        {103, 203, 303, 403, 503, 603}, // Bishop
        {102, 202, 302, 402, 502, 602}, // Rook
        {101, 201, 301, 401, 501, 601}, // Queen
        {100, 200, 300, 400, 500, 600}  // King
    };

    return mvvLvaScores[static_cast<int>(attacker)][static_cast<int>(victim)];
}

// Store a killer move
void Engine::storeKillerMove(const Move &move, int ply)
{
    if (ply >= MAX_PLY)
        return;

    // Don't store if it's already the first killer move
    if (killerMoves[ply][0].from.row == move.from.row &&
        killerMoves[ply][0].from.col == move.from.col &&
        killerMoves[ply][0].to.row == move.to.row &&
        killerMoves[ply][0].to.col == move.to.col)
    {
        return;
    }

    // Shift the second killer move to make room for the new one
    killerMoves[ply][1] = killerMoves[ply][0];
    killerMoves[ply][0] = move;
}

// Check if a move is a killer move at the current ply
bool Engine::isKillerMove(const Move &move, int ply) const
{
    if (ply >= MAX_PLY)
        return false;

    return (killerMoves[ply][0].from.row == move.from.row &&
            killerMoves[ply][0].from.col == move.from.col &&
            killerMoves[ply][0].to.row == move.to.row &&
            killerMoves[ply][0].to.col == move.to.col) ||
           (killerMoves[ply][1].from.row == move.from.row &&
            killerMoves[ply][1].from.col == move.from.col &&
            killerMoves[ply][1].to.row == move.to.row &&
            killerMoves[ply][1].to.col == move.to.col);
}

// REPLACE the entire storeCounterMove() method with this fixed version:
void Engine::storeCounterMove(const Move &lastMove, const Move &counterMove)
{
    if (!lastMove.from.isValid() || !lastMove.to.isValid())
        return;

    // FIXED: We need to get the piece that MADE the last move, not what's at the destination now
    // The piece that made the last move is now at lastMove.to, but we need its original info
    
    // Get the current board state AFTER the move was made
    auto pieceAtDestination = game.getBoard().getPieceAt(lastMove.to);
    if (!pieceAtDestination)
        return;

    // CRITICAL FIX: The counter move should be indexed by the OPPONENT's move
    // But we want to store OUR counter move against THEIR piece type and color
    int opponentPieceType = static_cast<int>(pieceAtDestination->getType());
    int opponentColor = (pieceAtDestination->getColor() == Color::WHITE) ? 0 : 1;
    int fromIdx = lastMove.from.row * 8 + lastMove.from.col;
    int toIdx = lastMove.to.row * 8 + lastMove.to.col;

    // Store the counter move: "When opponent plays [pieceType][color] from [from] to [to], 
    // we should respond with [counterMove]"
    counterMoves[opponentPieceType][opponentColor][fromIdx][toIdx] = counterMove;
}

// ALSO REPLACE the getCounterMove() method with this fixed version:
Move Engine::getCounterMove(const Move &lastMove) const
{
    if (!lastMove.from.isValid() || !lastMove.to.isValid())
        return Move(Position(), Position());

    // Get the piece that made the last move (opponent's piece)
    auto piece = game.getBoard().getPieceAt(lastMove.to);
    if (!piece)
        return Move(Position(), Position());

    // FIXED: Use the same indexing as storage
    int pieceType = static_cast<int>(piece->getType());
    int color = (piece->getColor() == Color::WHITE) ? 0 : 1;
    int fromIdx = lastMove.from.row * 8 + lastMove.from.col;
    int toIdx = lastMove.to.row * 8 + lastMove.to.col;

    return counterMoves[pieceType][color][fromIdx][toIdx];
}

void Engine::updateHistoryScore(const Move &move, int depth, Color color)
{
    int colorIdx = (color == Color::WHITE) ? 0 : 1;
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;

    // Bonus based on depth - deeper searches get higher bonuses
    int bonus = depth * depth;

    // FIXED: Proper aging algorithm instead of simple division
    // Age the move that caused the cutoff (increase its score)
    historyTable[colorIdx][fromIdx][toIdx] += bonus;

    // Age all other moves for this color (decrease their scores slightly)
    // This is called "aging" - recent moves become more important
    for (int from = 0; from < 64; from++)
    {
        for (int to = 0; to < 64; to++)
        {
            if (from != fromIdx || to != toIdx) // Don't age the move that just caused cutoff
            {
                // Reduce other moves' scores by a small amount (aging factor)
                // This makes recent successful moves more prominent
                historyTable[colorIdx][from][to] = 
                    (historyTable[colorIdx][from][to] * 15) / 16; // Reduce by 1/16th
            }
        }
    }

    // Prevent overflow with better bounds checking
    const int MAX_HISTORY_SCORE = 8192;
    if (historyTable[colorIdx][fromIdx][toIdx] > MAX_HISTORY_SCORE)
    {
        // Scale down ALL scores proportionally to maintain relative relationships
        for (int c = 0; c < 2; c++)
        {
            for (int from = 0; from < 64; from++)
            {
                for (int to = 0; to < 64; to++)
                {
                    historyTable[c][from][to] = (historyTable[c][from][to] * 3) / 4;
                }
            }
        }
    }
}

// Get the history score for a move
int Engine::getHistoryScore(const Move &move, Color color) const
{
    int colorIdx = (color == Color::WHITE) ? 0 : 1;
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;

    return historyTable[colorIdx][fromIdx][toIdx];
}

// Check if a move is part of the principal variation (deprecated version for compatibility)
bool Engine::isPVMove(const Move &move, const std::vector<Move> &pv, int ply) const
{
    if (pv.size() <= static_cast<size_t>(ply))
        return false;

    const Move &pvMove = pv[ply];
    return (pvMove.from.row == move.from.row &&
            pvMove.from.col == move.from.col &&
            pvMove.to.row == move.to.row &&
            pvMove.to.col == move.to.col);
}

// REPLACE the beginning of getMoveScore() method with this improved version:

int Engine::getMoveScore(const Move &move, const Board &board, const Move &ttMove,
                         const std::vector<Move> &pv, int ply, Color sideToMove,
                         const Move &lastMove) const
{
    // CRITICAL: Check for valid moving piece first
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece) {
        return -999999; // Invalid move - heavily penalize
    }

    // 1. Transposition table move (highest priority)
    if (ttMove.from.isValid() && ttMove.to.isValid() &&
        ttMove.from.row == move.from.row && ttMove.from.col == move.from.col &&
        ttMove.to.row == move.to.row && ttMove.to.col == move.to.col)
    {
        return 10000000; // Highest priority
    }

    // 2. Principal variation moves from ALL depths (not just current)
    for (int d = maxDepth; d >= 1; d--)
    {
        if (isPVMove(move, d, ply))
        {
            return 9000000 + d * 1000; // Very high priority, deeper searches get bonus
        }
    }

    // 3. Winning captures (positive SEE) - ordered by victim value then SEE score
    auto capturedPiece = board.getPieceAt(move.to);
    if (capturedPiece) {
        int seeScore = seeCapture(board, move);
        
        if (seeScore > 0) {
            // Good captures: prioritize by victim value, then by SEE score
            int victimValue = getPieceValue(capturedPiece->getType());
            int attackerValue = getPieceValue(movingPiece->getType());
            
            // MVV-LVA: Most Valuable Victim - Least Valuable Attacker
            int mvvLvaScore = (victimValue * 100) - (attackerValue / 10);
            
            return 8000000 + mvvLvaScore + seeScore;
        }
        // Even captures: still prioritize over non-captures but below good ones
        else if (seeScore == 0) {
            int victimValue = getPieceValue(capturedPiece->getType());
            return 7000000 + victimValue;
        }
        // Bad captures: lowest priority among captures
        else {
            return 6000000 + seeScore; // seeScore is negative here
        }
    }

    // 4. Counter moves (moves that refute the opponent's last move)
    if (lastMove.from.isValid() && lastMove.to.isValid())
    {
        Move counter = getCounterMove(lastMove);
        if (counter.from.isValid() && counter.to.isValid() &&
            counter.from.row == move.from.row && counter.from.col == move.from.col &&
            counter.to.row == move.to.row && counter.to.col == move.to.col)
        {
            return 5000000;
        }
    }

    // 5. Killer moves (moves that caused beta cutoffs at this ply)
    if (isKillerMove(move, ply))
    {
        // First killer move gets slightly higher score than second
        if (ply < MAX_PLY && 
            killerMoves[ply][0].from.row == move.from.row &&
            killerMoves[ply][0].from.col == move.from.col &&
            killerMoves[ply][0].to.row == move.to.row &&
            killerMoves[ply][0].to.col == move.to.col)
        {
            return 4000100;
        }
        return 4000000;
    }

    // 6. History heuristic (how often this move caused cutoffs in the past)
    int historyScore = getHistoryScore(move, sideToMove);
    
    // 7. Basic positional scoring for remaining moves
    int positionalScore = 0;
    
    // Bonus for moves toward the center
    int centerDistance = abs(move.to.row - 3.5) + abs(move.to.col - 3.5);
    positionalScore += (7 - centerDistance) * 10;
    
    // Bonus for advancing pawns
    if (movingPiece->getType() == PieceType::PAWN) {
        if (sideToMove == Color::WHITE) {
            positionalScore += move.to.row * 20;
        } else {
            positionalScore += (7 - move.to.row) * 20;
        }
    }
    
    return historyScore + positionalScore;
}

// Quiescence search for handling captures at leaf nodes
int Engine::quiescenceSearch(Board &board, int alpha, int beta, uint64_t hashKey, int ply)
{
    // Track nodes searched
    nodesSearched++;

    // Maximum recursion depth check
    // Maximum recursion depth check for quiescence
    if (ply >= MAX_QSEARCH_DEPTH)
        return evaluatePosition(board);

    // Stand-pat score (evaluate the current position without making any moves)
    int standPat = evaluatePosition(board);

    // Beta cutoff
    if (standPat >= beta)
        return beta;

    // Update alpha if stand-pat score is better
    if (standPat > alpha)
        alpha = standPat;

    // Generate capturing moves and checks
    std::vector<Move> qMoves;
    bool inCheck = board.isInCheck();

// Generate capturing moves and checks efficiently
std::vector<Move> qMoves;
bool inCheck = board.isInCheck();

if (inCheck) {
    // If in check, we need all legal moves (no choice but to generate all)
    auto legalMoves = board.generateLegalMoves();
    qMoves = legalMoves;
} else {
    // Generate only captures directly - much more efficient!
    generateCaptureMoves(board, qMoves);
}

if (qMoves.empty())
    return standPat;

    if (qMoves.empty())
        return standPat;

    // Score and sort the moves
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto &move : qMoves)
    {
        // Score the move
        int moveScore = 0;

        auto movingPiece = board.getPieceAt(move.from);
        auto capturedPiece = board.getPieceAt(move.to);

        bool isCapture = capturedPiece != nullptr ||
                         (movingPiece->getType() == PieceType::PAWN &&
                          move.to == board.getEnPassantTarget());

        // Delta pruning - skip captures that can't improve alpha
        if (isCapture && !inCheck && ply > 0)
        {
            // Get the maximum possible material gain from this capture
            int captureValue = 0;
            if (capturedPiece)
            {
                captureValue = getPieceValue(capturedPiece->getType());
            }
            else if (move.to == board.getEnPassantTarget())
            {
                captureValue = PAWN_VALUE;
            }

            // Add potential promotion bonus
            int promotionBonus = 0;
            if (movingPiece->getType() == PieceType::PAWN &&
                (move.to.row == 0 || move.to.row == 7))
            {
                promotionBonus = QUEEN_VALUE - PAWN_VALUE;
            }

            // Delta margin - a buffer to account for positional gains
            const int DELTA_MARGIN = 200;

            // Skip if even the maximum possible gain can't improve alpha
            if (standPat + captureValue + promotionBonus + DELTA_MARGIN <= alpha)
            {
                continue; // Skip this capture - it can't improve alpha
            }

            // Additional futility pruning for bad captures
            if (capturedPiece && seeCapture(board, move) < -50)
            {
                continue; // Skip obviously bad captures
            }
        }

        if (capturedPiece)
        {
            // MVV-LVA scoring for captures
            moveScore = 10000000 + getMVVLVAScore(movingPiece->getType(), capturedPiece->getType());

            // Static Exchange Evaluation (SEE)
            int seeScore = seeCapture(board, move);
            if (seeScore < 0)
            {
                // Skip bad captures at deeper ply depths
                if (ply > 2 && !inCheck)
                    continue;

                // Penalize bad captures, but still consider them
                moveScore += seeScore;
            }
        }
        else if (board.getPieceAt(move.from)->getType() == PieceType::PAWN &&
                 move.to == board.getEnPassantTarget())
        {
            // En passant capture
            moveScore = 10000000 + getMVVLVAScore(movingPiece->getType(), PieceType::PAWN);
        }
        else
        {
            // Non-capture move (likely a check)
            // Score lower than captures but still consider
            moveScore = 1000000;
        }

        scoredMoves.push_back(std::make_pair(moveScore, move));
    }

    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const std::pair<int, Move> &a, const std::pair<int, Move> &b)
              {
                  return a.first > b.first;
              });

    // Make each move and recursively search
    for (const auto &scoredMove : scoredMoves)
    {
        const Move &move = scoredMove.second;

        // Save board state for unmaking move
        BoardState previousState;

        // Calculate new hash key BEFORE making the move
        uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

        // Make the move
        if (!board.makeMove(move, previousState))
            continue;

        // Recursively search
        int score = -quiescenceSearch(board, -beta, -alpha, newHashKey, ply + 1);

        // Unmake the move
        board.unmakeMove(move, previousState);

        // Beta cutoff
        if (score >= beta)
            return beta;

        // Update alpha
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

// Principal Variation Search (PVS) - optimization of alpha-beta
int Engine::pvSearch(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                     std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove)
{
    // Track nodes searched
    nodesSearched++;

    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(), Position());
    int score;

    pv.clear();

    // Probe the transposition table
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, ttMove))
    {
        return score; // Return cached result if available (but don't use TT at root)
    }

    // Check for search termination conditions
    if (ply >= MAX_PLY)
    {
        return evaluatePosition(board);
    }

    // If we've reached the maximum depth, use quiescence search
    if (depth <= 0)
    {
        return quiescenceSearch(board, alpha, beta, hashKey, ply);
    }

    // Check if we should extend the search depth
    int extension = 0;

    // 1. Check extension - extend search when in check
    if (board.isInCheck())
    {
        extension = 1;
    }

    // If the game is over, return the evaluation
    if (board.isCheckmate() || board.isStalemate())
    {
        return evaluatePosition(board);
    }

    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();

    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty())
    {
        if (board.isInCheck())
        {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        }
        else
        {
            // Stalemate
            return 0;
        }
    }

    // 2. Singular Move Extension - if only one legal move, extend
    if (legalMoves.size() == 1 && depth >= 2)
    {
        extension = std::max(extension, 1);
    }

    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto &move : legalMoves)
    {
        int moveScore = getMoveScore(move, board, ttMove, principalVariation, ply, board.getSideToMove(), lastMove);

        // Early pruning of very bad captures
        if (depth >= 3)
        {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece)
            {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2)
                {
                    continue;
                }
            }
        }

        scoredMoves.push_back(std::make_pair(moveScore, move));
    }

    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const std::pair<int, Move> &a, const std::pair<int, Move> &b)
              {
                  return a.first > b.first;
              });

    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];
    bool foundPV = false;

    // This will be used to store the principal variation
    std::vector<Move> childPV;

// REPLACE the entire "if (maximizingPlayer)" clause in pvSearch() with this:
if (maximizingPlayer)
{
    int maxEval = std::numeric_limits<int>::min();

    for (size_t i = 0; i < scoredMoves.size(); i++)
    {
        const Move &move = scoredMoves[i].second;

        // Determine move characteristics for LMR
        bool isPVMoveCheck = false;
        for (int d = 1; d <= maxDepth; d++)
        {
            if (isPVMove(move, d, ply))
            {
                isPVMoveCheck = true;
                break;
            }
        }

        bool isCapture = board.getPieceAt(move.to) != nullptr;
        bool isKillerMoveCheck = isKillerMove(move, ply);

        // Save board state for unmaking move
        BoardState previousState;

        // Make the move
        if (!board.makeMove(move, previousState))
            continue;

        bool isCheckMove = board.isInCheck();

        // Calculate depth adjustment
        int moveExtension = extension;

        // Recapture Extension
        if (lastMove.to.isValid() && move.to == lastMove.to)
        {
            moveExtension = std::max(moveExtension, 1);
        }

        // Pawn Push Extension
        auto piece = board.getPieceAt(move.to); // Use move.to since piece is now there
        if (piece && piece->getType() == PieceType::PAWN)
        {
            int destRow = (board.getSideToMove() == Color::BLACK) ? 6 : 1; // 7th rank (flipped because we switched sides)
            if (move.to.row == destRow)
            {
                moveExtension = std::max(moveExtension, 1);
            }
        }

        // Calculate LMR reduction
        int lmrReduction = calculateLMRReduction(depth, i, foundPV, isCapture, isCheckMove, isKillerMoveCheck);

        // Final depth after adjustments
        int newDepth = depth - 1 + moveExtension - lmrReduction;
        newDepth = std::max(0, newDepth);

        // Calculate the new hash key after the move
        uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

        // Recursively evaluate the position
        childPV.clear();
        int eval;

        if (foundPV)
        {
            // For non-PV moves, try LMR first if applicable
            if (lmrReduction > 0)
            {
                // CONSISTENT: Correct null window search
                eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);

                // FIXED: Consistent re-search condition - if LMR fails high, re-search
                if (eval > alpha)
                {
                    newDepth = depth - 1 + moveExtension; // Full depth
                    childPV.clear();
                    eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);
                }
            }
            else
            {
                // No reduction, do null window search
                eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);
            }

            // If we get a fail-high, re-search with full window
            if (eval > alpha && eval < beta)
            {
                childPV.clear();
                eval = -pvSearch(board, newDepth, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
            }
        }
        else
        {
            // First move gets a full window search
            eval = -pvSearch(board, newDepth, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
            foundPV = true;
        }

        // Unmake the move
        board.unmakeMove(move, previousState);

        // Update the best move if this move is better
        if (eval > maxEval)
        {
            maxEval = eval;
            localBestMove = move;

            // Update principal variation
            pv.clear();
            pv.push_back(move);
            pv.insert(pv.end(), childPV.begin(), childPV.end());
        }

        // Alpha-beta pruning
        alpha = std::max(alpha, eval);
        if (beta <= alpha)
        {
            // Store this move as a killer move if it's not a capture
            if (!isCapture)
            {
                storeKillerMove(move, ply);
                updateHistoryScore(move, depth, Color::WHITE); // Use WHITE since maximizing

                if (lastMove.from.isValid() && lastMove.to.isValid())
                {
                    storeCounterMove(lastMove, move);
                }
            }

            nodeType = NodeType::BETA;
            break;
        }
    }

    // Store result in transposition table
    if (maxEval > originalAlpha && maxEval < beta)
    {
        nodeType = NodeType::EXACT;
    }
    transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);

    return maxEval;
}
    // REPLACE the entire "else" clause (minimizing player section) in pvSearch() with this:
else
{
    // Minimizing player
    int minEval = std::numeric_limits<int>::max();

    for (size_t i = 0; i < scoredMoves.size(); i++)
    {
        const Move &move = scoredMoves[i].second;

        // Determine move characteristics for LMR
        bool isPVMoveCheck = false;
        for (int d = 1; d <= maxDepth; d++)
        {
            if (isPVMove(move, d, ply))
            {
                isPVMoveCheck = true;
                break;
            }
        }

        bool isCapture = board.getPieceAt(move.to) != nullptr;
        bool isKillerMoveCheck = isKillerMove(move, ply);

        // Save board state for unmaking move
        BoardState previousState;

        // Make the move
        if (!board.makeMove(move, previousState))
            continue;

        bool isCheckMove = board.isInCheck();

        // Calculate depth adjustment
        int moveExtension = extension;

        // Recapture Extension
        if (lastMove.to.isValid() && move.to == lastMove.to)
        {
            moveExtension = std::max(moveExtension, 1);
        }

        // Pawn Push Extension
        auto piece = board.getPieceAt(move.to);
        if (piece && piece->getType() == PieceType::PAWN)
        {
            int destRow = (board.getSideToMove() == Color::BLACK) ? 6 : 1; // 7th rank
            if (move.to.row == destRow)
            {
                moveExtension = std::max(moveExtension, 1);
            }
        }

        // Calculate LMR reduction
        int lmrReduction = calculateLMRReduction(depth, i, foundPV, isCapture, isCheckMove, isKillerMoveCheck);

        // Final depth after adjustments
        int newDepth = depth - 1 + moveExtension - lmrReduction;
        newDepth = std::max(0, newDepth);

        // Calculate the new hash key after the move
        uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

        // Recursively evaluate the position
        childPV.clear();
        int eval;

        if (foundPV)
        {
            // For non-PV moves, try LMR first if applicable
            if (lmrReduction > 0)
            {
                // FIXED: Correct null window for minimizing player
                eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);

                // If LMR search fails high, re-search at full depth
                if (eval < beta)
                {
                    newDepth = depth - 1 + moveExtension; // Full depth
                    childPV.clear();
                    eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);
                }
            }
            else
            {
                // No reduction, do null window search
                eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);
            }

            // If we get a fail-high, re-search with full window
            if (eval < beta && eval > alpha)
            {
                childPV.clear();
                eval = -pvSearch(board, newDepth, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
            }
        }
        else
        {
            // First move gets a full window search
            eval = -pvSearch(board, newDepth, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
            foundPV = true;
        }

        // Unmake the move
        board.unmakeMove(move, previousState);

        // Update the best move if this move is better
        if (eval < minEval)
        {
            minEval = eval;
            localBestMove = move;

            // Update principal variation
            pv.clear();
            pv.push_back(move);
            pv.insert(pv.end(), childPV.begin(), childPV.end());
        }

        // Alpha-beta pruning
        beta = std::min(beta, eval);
        if (beta <= alpha)
        {
            // Store this move as a killer move if it's not a capture
            if (!isCapture)
            {
                storeKillerMove(move, ply);
                updateHistoryScore(move, depth, Color::BLACK); // Use BLACK since minimizing

                if (lastMove.from.isValid() && lastMove.to.isValid())
                {
                    storeCounterMove(lastMove, move);
                }
            }

            nodeType = NodeType::ALPHA;
            break;
        }
    }

    // Store result in transposition table
    if (minEval > originalAlpha && minEval < beta)
    {
        nodeType = NodeType::EXACT;
    }
    transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);

    return minEval;
}

// Regular alpha-beta search (kept for reference/fallback)
int Engine::alphaBeta(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                      std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove)
{
    // Track nodes searched
    nodesSearched++;

    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(), Position());
    int score;

    pv.clear();

    // Probe the transposition table
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, ttMove))
    {
        return score; // Return cached result if available (but don't use TT at root)
    }

    // If we've reached the maximum depth, use quiescence search
    if (depth == 0)
    {
        return quiescenceSearch(board, alpha, beta, hashKey, ply);
    }

    // If the game is over, return the evaluation
    if (board.isCheckmate() || board.isStalemate())
    {
        return evaluatePosition(board);
    }

    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();

    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty())
    {
        if (board.isInCheck())
        {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        }
        else
        {
            // Stalemate
            return 0;
        }
    }

    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto &move : legalMoves)
    {
        int moveScore = getMoveScore(move, board, ttMove, principalVariation, ply, board.getSideToMove(), lastMove);

        // Early pruning of very bad captures
        if (depth >= 3)
        {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece)
            {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2)
                {
                    continue;
                }
            }
        }

        scoredMoves.push_back(std::make_pair(moveScore, move));
    }

    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const std::pair<int, Move> &a, const std::pair<int, Move> &b)
              {
                  return a.first > b.first;
              });

    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];

    // This will be used to store the principal variation
    std::vector<Move> childPV;

    if (maximizingPlayer)
    {
        int maxEval = std::numeric_limits<int>::min();

        for (const auto &scoredMove : scoredMoves)
        {
            const Move &move = scoredMove.second;

            // Save board state for unmaking move
            BoardState previousState;

            // Calculate new hash key BEFORE making the move
            uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

            // Make the move
            if (!board.makeMove(move, previousState))
                continue;

            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(board, depth - 1, alpha, beta, false, childPV, newHashKey, ply + 1, move);

            // Unmake the move
            board.unmakeMove(move, previousState);

            // Update the best move if this move is better
            if (eval > maxEval)
            {
                maxEval = eval;
                localBestMove = move;

                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }

            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
            {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr)
                {
                    // Update killer moves table
                    storeKillerMove(move, ply);

                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());

                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid())
                    {
                        storeCounterMove(lastMove, move);
                    }
                }

                nodeType = NodeType::BETA; // Fail high
                break;
            }
        }

        // Store result in transposition table
        if (maxEval > originalAlpha && maxEval < beta)
        {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);

        return maxEval;
    }
    else
    {
        int minEval = std::numeric_limits<int>::max();

        for (const auto &scoredMove : scoredMoves)
        {
            const Move &move = scoredMove.second;

            // Save board state for unmaking move
            BoardState previousState;

            // Calculate new hash key BEFORE making the move
            uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

            // Make the move
            if (!board.makeMove(move, previousState))
                continue;

            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(board, depth - 1, alpha, beta, true, childPV, newHashKey, ply + 1, move);

            // Unmake the move
            board.unmakeMove(move, previousState);

            // Update the best move if this move is better
            if (eval < minEval)
            {
                minEval = eval;
                localBestMove = move;

                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }

            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha)
            {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr)
                {
                    // Update killer moves table
                    storeKillerMove(move, ply);

                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());

                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid())
                    {
                        storeCounterMove(lastMove, move);
                    }
                }

                nodeType = NodeType::ALPHA; // Fail low
                break;
            }
        }

        // Store result in transposition table
        if (minEval > originalAlpha && minEval < beta)
        {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);

        return minEval;
    }
}

// Evaluation function
int Engine::evaluatePosition(const Board &board)
{
    int whiteScore = 0;
    int blackScore = 0;
    bool isEndgamePhase = isEndgame(board);

    // Loop through all squares on the board
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);

            if (!piece)
                continue;

            // Base piece value
            int pieceValue = 0;

            // Positional value based on piece-square tables
            int positionalValue = 0;

            // Calculate the index for the piece-square tables
            int tableIndex = row * 8 + col;

            // Adjust the index for black pieces (mirror the board)
            int blackTableIndex = (7 - row) * 8 + col;

            switch (piece->getType())
            {
            case PieceType::PAWN:
                pieceValue = PAWN_VALUE;
                positionalValue = pawnTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;

            case PieceType::KNIGHT:
                pieceValue = KNIGHT_VALUE;
                positionalValue = knightTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;

            case PieceType::BISHOP:
                pieceValue = BISHOP_VALUE;
                positionalValue = bishopTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;

            case PieceType::ROOK:
                pieceValue = ROOK_VALUE;
                positionalValue = rookTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;

            case PieceType::QUEEN:
                pieceValue = QUEEN_VALUE;
                positionalValue = queenTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;

            case PieceType::KING:
                pieceValue = KING_VALUE;
                if (isEndgamePhase)
                {
                    positionalValue = kingEndGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                }
                else
                {
                    positionalValue = kingMiddleGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                }
                break;

            default:
                break;
            }

            // Add the piece value and positional value to the appropriate side's score
            if (piece->getColor() == Color::WHITE)
            {
                whiteScore += pieceValue + positionalValue;
            }
            else
            {
                blackScore += pieceValue + positionalValue;
            }
        }
    }

    // Check for checkmate and stalemate
    if (board.isCheckmate())
    {
        if (board.getSideToMove() == Color::WHITE)
        {
            // Black wins
            return -100000;
        }
        else
        {
            // White wins
            return 100000;
        }
    }
    else if (board.isStalemate())
    {
        // Draw
        return 0;
    }

    // Calculate the final score from white's perspective
    int score = whiteScore - blackScore;

    // Adjust the score based on the side to move
    return board.getSideToMove() == Color::WHITE ? score : -score;
}
bool Engine::shouldStopSearch() const
{
    if (!timeManaged || timeAllocated <= 0)
        return false;

    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - searchStartTime);

    // Allow some buffer time to avoid timeout
    int safeTimeLimit = timeAllocated - timeBuffer;

    return elapsed.count() >= safeTimeLimit;
}

bool Engine::isEndgame(const Board &board) const
{
    int pieceCount = 0;
    bool whiteQueenPresent = false;
    bool blackQueenPresent = false;

    // Count the number of pieces and check if queens are present
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            auto piece = board.getPieceAt(Position(row, col));

            if (piece && piece->getType() != PieceType::KING && piece->getType() != PieceType::PAWN)
            {
                pieceCount++;

                if (piece->getType() == PieceType::QUEEN)
                {
                    if (piece->getColor() == Color::WHITE)
                    {
                        whiteQueenPresent = true;
                    }
                    else
                    {
                        blackQueenPresent = true;
                    }
                }
            }
        }
    }

    // Consider it an endgame if:
    // 1. Both queens are missing, or
    // 2. There are few minor pieces left
    return (!whiteQueenPresent && !blackQueenPresent) || (pieceCount <= 6);
}