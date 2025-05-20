
Chess engine /
Before we progress i want you to help me deubgg and fix incomplete engine implementations. provide full code in artificat form so i can copy and paste - Potential Issues and Concerns
1. Incomplete Engine Implementation
I noticed in engine.cpp that there's a fragment starting mid-function (around line 200) without the beginning of the file. This is a critical issue to fix before expansion.
 
cpp
// If it's a good capture (positive SEE) if (seeScore > 0) {     return 4000000 + seeScore; }
The file appears to be missing the first ~200 lines, including key functions like:
* getBestMove()
* iterativeDeepeningSearch()
* Part of getMoveScore()
* Implementation of seeCapture() and see()
2. Missing Initialization in zobrist.cpp
The Zobrist::updateHashKey function assumes the Zobrist hash has been initialized, but there's no check for this:
 
cpp
if (!initialized) initialize();
This check appears in generateHashKey() but is missing in updateHashKey().
3. Potential Memory Issues with Board Copy
In engine.cpp, you create a copy of the board in both search functions:
 
cpp
// Make a copy of the board Board tempBoard = board;
If Board contains large data structures and this happens at every node in the search tree, it could cause performance issues. Consider using a make/unmake approach for moves instead.
4. Move Generation and Castling Logic
There might be a logic issue in how castling is handled. In Board::makeMove(), you check:
 
cpp
if (!canCastle(move)) {     return false; }
But this doesn't account for scenarios where the king passes through a checked square, which should be invalid.

I'll help you debug and fix the incomplete engine implementation. Let's start by providing a complete implementation of engine.cpp to address the missing parts.


Complete engine.cpp Implementation
Code 



Continue

I'll continue with the engine.cpp file and then fix the other issues you identified.


Complete engine.cpp Implementation
Code ∙ Version 2 
Now let's fix the issue in the zobrist.cpp file where the updateHashKey function is missing an initialization check:


Fixed zobrist.cpp Implementation
Code 
Now let's address the memory issue with Board copy operations by implementing a more efficient make/unmake move approach. I'll provide an alternative implementation for the relevant parts of the engine:


Drafting artifact...
 



Continue

Let me continue with the implementation improvements for the chess engine:


Untitled
Text 
Now let's implement the make/unmake approach for the engine search function to avoid expensive board copies:


New board_state.h file for move reversal
Code 
Now let's update the board.h and board.cpp files to support the make/unmake approach:


Modifications for board.h
Code 
Now let's implement the updated makeMove and unmakeMove methods in board.cpp:


Implementation for makeMove and unmakeMove in board.cpp
Code 
Finally, let's modify the Piece class to support the setHasMoved function needed by the unmakeMove implementation:


Modifications for piece.h
Code 
Now let's update the engine search functions to use the make/unmake approach instead of board copying:


Make/Unmake Approach for Engine Search Functions
Code 
Let me provide a summary of all the changes I've made to fix the issues you identified:


Drafting artifact...
 



Continue

Let me continue with the summary of fixes and improvements:


Drafting artifact...
 
Let me also provide a guide on how to potentially further optimize your chess engine in the future:


Drafting artifact...
 



#include "engine.h"
#include <chrono>
#include <limits>
#include <algorithm>
#include <sstream>

// Initialize piece-square tables
// These tables provide positional bonuses for pieces on specific squares
const int Engine::pawnTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int Engine::knightTable[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

const int Engine::bishopTable[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5,  5,  5,  5,  5,-10,
    -10,  0,  5,  0,  0,  5,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

const int Engine::rookTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};

const int Engine::queenTable[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
    0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

const int Engine::kingMiddleGameTable[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

const int Engine::kingEndGameTable[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

// Get the best move for the current position
Move Engine::getBestMove() {
    // Reset search statistics
    resetStats();
    searchStartTime = std::chrono::high_resolution_clock::now();
    
    // Get a copy of the board
    Board board = game.getBoard();
    
    // Increment transposition table age
    transpositionTable.incrementAge();
    
    // Initialize Zobrist hashing
    Zobrist::initialize();
    uint64_t hashKey = Zobrist::generateHashKey(board);
    
    // Use iterative deepening to find the best move
    return iterativeDeepeningSearch(board, maxDepth, hashKey);
}

// Iterative deepening search
Move Engine::iterativeDeepeningSearch(Board& board, int maxDepth, uint64_t hashKey) {
    principalVariation.clear();
    Move bestMove(Position(), Position());
    int bestScore = 0;
    
    // Iterative deepening loop
    for (int depth = 1; depth <= maxDepth; depth++) {
        std::vector<Move> pv;
        
        // Color is set to true for maximizing player (WHITE), false for minimizing player (BLACK)
        bool maximizingPlayer = board.getSideToMove() == Color::WHITE;
        
        // Search with a full window
        int score = pvSearch(board, depth, -100000, 100000, maximizingPlayer, pv, hashKey, 0, Move(Position(), Position()));
        
        // Store the best move and score
        if (!pv.empty()) {
            bestMove = pv[0];
            principalVariation = pv;
            bestScore = score;
        }
        
        // Log the progress
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - searchStartTime);
        
        std::cout << "Depth: " << depth 
                  << ", Score: " << score 
                  << ", Nodes: " << nodesSearched 
                  << ", Time: " << duration.count() << "ms" 
                  << ", NPS: " << static_cast<long>(nodesSearched * 1000.0 / duration.count())
                  << ", PV: " << getPVString() << std::endl;
    }
    
    return bestMove;
}

// Get the principal variation as a string
std::string Engine::getPVString() const {
    std::stringstream ss;
    
    for (size_t i = 0; i < principalVariation.size(); i++) {
        ss << principalVariation[i].toString() << " ";
    }
    
    return ss.str();
}

// Static Exchange Evaluation (SEE)
int Engine::seeCapture(const Board& board, const Move& move) const {
    auto capturedPiece = board.getPieceAt(move.to);
    if (!capturedPiece) return 0; // Not a capture
    
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece) return 0; // Should never happen
    
    // Get the value of the captured piece
    int captureValue = getPieceValue(capturedPiece->getType());
    
    // Make the capture and see what happens
    return captureValue - see(board, move.to, movingPiece->getColor(), getPieceValue(movingPiece->getType()));
}

// Recursive SEE function
int Engine::see(const Board& board, const Position& square, Color side, int captureValue) const {
    // Find the least valuable attacker of the opposite color
    auto leastValuableAttacker = Position();
    int leastValuableAttackerValue = 100000;
    PieceType leastValuableAttackerType = PieceType::NONE;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);
            
            if (piece && piece->getColor() != side) {
                // Check if this piece can capture the target
                auto moves = piece->getLegalMoves(board);
                
                for (const auto& move : moves) {
                    if (move.to == square) {
                        int pieceValue = getPieceValue(piece->getType());
                        
                        if (pieceValue < leastValuableAttackerValue) {
                            leastValuableAttackerValue = pieceValue;
                            leastValuableAttacker = pos;
                            leastValuableAttackerType = piece->getType();
                        }
                        
                        break;
                    }
                }
            }
        }
    }
    
    // If no attacker found, the previous capture was the end of the sequence
    if (leastValuableAttackerType == PieceType::NONE) {
        return 0;
    }
    
    // Recursive SEE if there are more captures
    int score = captureValue - see(board, square, (side == Color::WHITE) ? Color::BLACK : Color::WHITE, leastValuableAttackerValue);
    
    return std::max(0, score); // Don't make a capture if it's worse than doing nothing
}

// Get the approximate value of a piece for SEE
int Engine::getPieceValue(PieceType type) const {
    switch (type) {
        case PieceType::PAWN: return PAWN_VALUE;
        case PieceType::KNIGHT: return KNIGHT_VALUE;
        case PieceType::BISHOP: return BISHOP_VALUE;
        case PieceType::ROOK: return ROOK_VALUE;
        case PieceType::QUEEN: return QUEEN_VALUE;
        case PieceType::KING: return KING_VALUE;
        default: return 0;
    }
}

// MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) scoring for move ordering
int Engine::getMVVLVAScore(PieceType attacker, PieceType victim) const {
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
void Engine::storeKillerMove(const Move& move, int ply) {
    if (ply >= MAX_PLY) return;
    
    // Don't store if it's already the first killer move
    if (killerMoves[ply][0].from.row == move.from.row && 
        killerMoves[ply][0].from.col == move.from.col && 
        killerMoves[ply][0].to.row == move.to.row && 
        killerMoves[ply][0].to.col == move.to.col) {
        return;
    }
    
    // Shift the second killer move to make room for the new one
    killerMoves[ply][1] = killerMoves[ply][0];
    killerMoves[ply][0] = move;
}

// Check if a move is a killer move at the current ply
bool Engine::isKillerMove(const Move& move, int ply) const {
    if (ply >= MAX_PLY) return false;
    
    return (killerMoves[ply][0].from.row == move.from.row && 
            killerMoves[ply][0].from.col == move.from.col && 
            killerMoves[ply][0].to.row == move.to.row && 
            killerMoves[ply][0].to.col == move.to.col) ||
           (killerMoves[ply][1].from.row == move.from.row && 
            killerMoves[ply][1].from.col == move.from.col && 
            killerMoves[ply][1].to.row == move.to.row && 
            killerMoves[ply][1].to.col == move.to.col);
}

// Store a counter move
void Engine::storeCounterMove(const Move& lastMove, const Move& counterMove) {
    if (!lastMove.from.isValid() || !lastMove.to.isValid()) return;
    
    auto piece = game.getBoard().getPieceAt(lastMove.to);
    if (!piece) return;
    
    int pieceType = static_cast<int>(piece->getType());
    int color = (piece->getColor() == Color::WHITE) ? 0 : 1;
    int fromIdx = lastMove.from.row * 8 + lastMove.from.col;
    int toIdx = lastMove.to.row * 8 + lastMove.to.col;
    
    counterMoves[pieceType][color][fromIdx][toIdx] = counterMove;
}

// Get counter move for an opponent's move
Move Engine::getCounterMove(const Move& lastMove) const {
    if (!lastMove.from.isValid() || !lastMove.to.isValid()) return Move(Position(), Position());
    
    auto piece = game.getBoard().getPieceAt(lastMove.to);
    if (!piece) return Move(Position(), Position());
    
    int pieceType = static_cast<int>(piece->getType());
    int color = (piece->getColor() == Color::WHITE) ? 0 : 1;
    int fromIdx = lastMove.from.row * 8 + lastMove.from.col;
    int toIdx = lastMove.to.row * 8 + lastMove.to.col;
    
    return counterMoves[pieceType][color][fromIdx][toIdx];
}

// Update history heuristic table for a move that caused beta cutoff
void Engine::updateHistoryScore(const Move& move, int depth, Color color) {
    int colorIdx = (color == Color::WHITE) ? 0 : 1;
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;
    
    // Bonus based on depth - deeper searches get higher bonuses
    int bonus = depth * depth;
    
    // Increase history score, but prevent overflow
    historyTable[colorIdx][fromIdx][toIdx] += bonus;
    
    // Prevent overflow by scaling down if the score gets too high
    if (historyTable[colorIdx][fromIdx][toIdx] > 10000) {
        for (int c = 0; c < 2; c++) {
            for (int from = 0; from < 64; from++) {
                for (int to = 0; to < 64; to++) {
                    historyTable[c][from][to] /= 2;
                }
            }
        }
    }
}

// Get the history score for a move
int Engine::getHistoryScore(const Move& move, Color color) const {
    int colorIdx = (color == Color::WHITE) ? 0 : 1;
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;
    
    return historyTable[colorIdx][fromIdx][toIdx];
}

// Check if a move is part of the principal variation
bool Engine::isPVMove(const Move& move, const std::vector<Move>& pv, int ply) const {
    if (pv.size() <= static_cast<size_t>(ply)) return false;
    
    const Move& pvMove = pv[ply];
    return (pvMove.from.row == move.from.row && 
            pvMove.from.col == move.from.col && 
            pvMove.to.row == move.to.row && 
            pvMove.to.col == move.to.col);
}

// Get score for move ordering
int Engine::getMoveScore(const Move& move, const Board& board, const Move& ttMove, 
                        const std::vector<Move>& pv, int ply, Color sideToMove, 
                        const Move& lastMove) const {
    // 1. Transposition table move
    if (ttMove.from.isValid() && ttMove.to.isValid() &&
        ttMove.from.row == move.from.row && ttMove.from.col == move.from.col &&
        ttMove.to.row == move.to.row && ttMove.to.col == move.to.col) {
        return 10000000;
    }
    
    // 2. Principal variation move
    if (isPVMove(move, pv, ply)) {
        return 9000000;
    }
    
    // 3. Captures (scored by MVV-LVA or SEE)
    auto movingPiece = board.getPieceAt(move.from);
    auto capturedPiece = board.getPieceAt(move.to);
    
    if (capturedPiece) {
        // Calculate Static Exchange Evaluation score
        int seeScore = seeCapture(board, move);
        
        // If it's a good capture (positive SEE)
        if (seeScore > 0) {
            return 4000000 + seeScore;
        } 
        // Still prioritize captures, but lower than good captures
        else {
            return 3000000 + getMVVLVAScore(movingPiece->getType(), capturedPiece->getType());
        }
    }
    
    // 4. Counter move
    if (lastMove.from.isValid() && lastMove.to.isValid()) {
        Move counter = getCounterMove(lastMove);
        if (counter.from.isValid() && counter.to.isValid() &&
            counter.from.row == move.from.row && counter.from.col == move.from.col &&
            counter.to.row == move.to.row && counter.to.col == move.to.col) {
            return 2500000;
        }
    }
    
    // 5. Killer moves
    if (isKillerMove(move, ply)) {
        // First killer move gets slightly higher score than second
        if (killerMoves[ply][0].from.row == move.from.row && 
            killerMoves[ply][0].from.col == move.from.col && 
            killerMoves[ply][0].to.row == move.to.row && 
            killerMoves[ply][0].to.col == move.to.col) {
            return 2000100;
        }
        return 2000000;
    }
    
    // 6. History heuristic for non-captures
    return getHistoryScore(move, sideToMove);
}

// Quiescence search - continues capturing sequences beyond regular search
int Engine::quiescenceSearch(Board& board, int alpha, int beta, uint64_t hashKey, int ply) {
    // Track nodes searched
    nodesSearched++;
    
    // Stand-pat score (evaluate the current position without making any moves)
    int standPat = evaluatePosition(board);
    
    // Beta cutoff
    if (standPat >= beta)
        return beta;
    
    // Update alpha if stand-pat score is better
    if (standPat > alpha)
        alpha = standPat;
    
    // Maximum recursion depth for quiescence search
    if (ply >= MAX_PLY - 1)
        return standPat;
    
    // Flag to track if we're in check
    bool inCheck = board.isInCheck();

    // Stage 1: If we're in check, we need to generate all legal moves
    if (inCheck) {
        auto legalMoves = board.generateLegalMoves();
        
        // Score and sort the moves
        std::vector<std::pair<int, Move>> scoredMoves;
        for (const auto& move : legalMoves) {
            int moveScore = 1000; // Base score for check evasions
            
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece) {
                auto movingPiece = board.getPieceAt(move.from);
                moveScore = 10000 + getMVVLVAScore(movingPiece->getType(), capturedPiece->getType());
            }
            
            scoredMoves.push_back(std::make_pair(moveScore, move));
        }
        
        // Sort moves by score (descending)
        std::sort(scoredMoves.begin(), scoredMoves.end(),
                [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) {
                    return a.first > b.first;
                });
        
        // Search all legal moves when in check
        for (const auto& scoredMove : scoredMoves) {
            const Move& move = scoredMove.second;
            
            // Save board state for unmaking move
            BoardState previousState;
            
            // Make the move
            if (!board.makeMove(move, previousState))
                continue;
            
            // Calculate new hash key
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Track the capture square if this move is a capture
            Position newLastCaptureSquare = lastCaptureSquare;
            if (board.getPieceAt(move.to) != nullptr) {
                newLastCaptureSquare = move.to;
            }
            
            // Recursively search
            int score = -quiescenceSearch(board, -beta, -alpha, newHashKey, ply + 1, qDepth + 1, newLastCaptureSquare);
            
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
    
    // If not in check, proceed with selective quiescence search using staged move generation
    
    if (qMoves.empty())
        return standPat;
    
    // Score and sort the moves
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto& move : qMoves) {
        // Score the move
        int moveScore = 0;
        
        auto movingPiece = board.getPieceAt(move.from);
        auto capturedPiece = board.getPieceAt(move.to);
        
        bool isCapture = capturedPiece != nullptr || 
                         (movingPiece->getType() == PieceType::PAWN && 
                          move.to == board.getEnPassantTarget());
        
        // Delta pruning - skip captures that can't improve alpha
        if (isCapture && !inCheck && qDepth > 0) {
            // Get the maximum possible material gain from this capture
            int captureValue = 0;
            if (capturedPiece) {
                captureValue = getPieceValue(capturedPiece->getType());
            } else if (move.to == board.getEnPassantTarget()) {
                captureValue = PAWN_VALUE;
            }
            
            // Add potential promotion bonus
            int promotionBonus = 0;
            if (movingPiece->getType() == PieceType::PAWN && 
                (move.to.row == 0 || move.to.row == 7)) {
                promotionBonus = QUEEN_VALUE - PAWN_VALUE;
            }
            
            // Delta margin - a buffer to account for positional gains
            const int DELTA_MARGIN = 200;
            
            // Skip if even the maximum possible gain can't improve alpha
            if (standPat + captureValue + promotionBonus + DELTA_MARGIN <= alpha) {
                continue;  // Skip this capture - it can't improve alpha
            }
        }
        
        if (capturedPiece) {
            // MVV-LVA scoring for captures
            moveScore = 10000000 + getMVVLVAScore(movingPiece->getType(), capturedPiece->getType());
            
            // Static Exchange Evaluation (SEE)
            int seeScore = seeCapture(board, move);
            if (seeScore < 0) {
                // Skip bad captures at deeper ply depths
                if (qDepth > 2 && !inCheck) continue;
                
                // Penalize bad captures, but still consider them
                moveScore += seeScore;
            }
        } else if (board.getPieceAt(move.from)->getType() == PieceType::PAWN && 
                  move.to == board.getEnPassantTarget()) {
            // En passant capture
            moveScore = 10000000 + getMVVLVAScore(movingPiece->getType(), PieceType::PAWN);
        } else {
            // Non-capture move (likely a check)
            // Score lower than captures but still consider
            moveScore = 1000000;
        }
        
        scoredMoves.push_back(std::make_pair(moveScore, move));
    }
    
    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) {
                  return a.first > b.first;
              });
    
    // Make each move and recursively search
    for (const auto& scoredMove : scoredMoves) {
        const Move& move = scoredMove.second;
        
        // Save board state for unmaking move
        BoardState previousState;
        
        // Make the move
        if (!board.makeMove(move, previousState))
            continue;
        
        // Calculate new hash key
        uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
        
        // Apply adaptive depth - reduce depth for specific move types
        int newQDepth = qDepth + 1;
        
        // Recursively search
        int score = -quiescenceSearch(board, -beta, -alpha, newHashKey, ply + 1, newQDepth);
        
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

// Principal Variation Search (PVS) - a more efficient version of alpha-beta for PV nodes
int Engine::pvSearch(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, 
                    std::vector<Move>& pv, uint64_t hashKey, int ply, Move lastMove) {
    // Track nodes searched
    nodesSearched++;
    
    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(), Position());
    int score;
    
    pv.clear();
    
    // Probe the transposition table
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, ttMove)) {
        return score; // Return cached result if available (but don't use TT at root)
    }
    
    // If we've reached the maximum depth or the game is over, evaluate the position
     if (depth == 0) {
        return quiescenceSearch(board, alpha, beta, hashKey, ply);
    }
    
    // If the game is over, return the evaluation
    if (depth == 0) {
        return quiescenceSearch(board, alpha, beta, hashKey, ply, 0);
    }
    
    // If the game is over, return the evaluation
    if (board.isCheckmate() || board.isStalemate()) {
        return evaluatePosition(board);
    }
    
    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty()) {
        if (board.isInCheck()) {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        } else {
            // Stalemate
            return 0;
        }
    }
    
    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto& move : legalMoves) {
        int moveScore = getMoveScore(move, board, ttMove, principalVariation, ply, board.getSideToMove(), lastMove);
        
        // Early pruning of very bad captures
        if (depth >= 3) {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece) {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2) {
                    continue;
                }
            }
        }
        
        scoredMoves.push_back(std::make_pair(moveScore, move));
    }
    
    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(), 
              [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) {
                  return a.first > b.first;
              });
    
    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];
    bool foundPV = false;
    
    // This will be used to store the principal variation
    std::vector<Move> childPV;
    
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        
        for (size_t i = 0; i < scoredMoves.size(); i++) {
            const Move& move = scoredMoves[i].second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval;
            
            // Full window search for first move, null window for others
            if (foundPV) {
                // Try a null window search first
                eval = -pvSearch(tempBoard, depth - 1, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);
                
                // If we might fail high, do a full window search
                if (eval > alpha && eval < beta) {
                    childPV.clear();
                    eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
                }
            } else {
                // First move gets a full window search
                eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
            }
            
            // Update the best move if this move is better
            if (eval > maxEval) {
                maxEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
                
                foundPV = true;
            }
            
            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::BETA; // Fail high
                break;
            }
        }
        
        // Store result in transposition table
        if (maxEval > originalAlpha && maxEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);
        
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        
        for (size_t i = 0; i < scoredMoves.size(); i++) {
            const Move& move = scoredMoves[i].second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval;
            
            // Full window search for first move, null window for others
            if (foundPV) {
                // Try a null window search first
                eval = -pvSearch(tempBoard, depth - 1, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);
                
                // If we might fail high, do a full window search
                if (eval > alpha && eval < beta) {
                    childPV.clear();
                    eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
                }
            } else {
                // First move gets a full window search
                eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
            }
            
            // Update the best move if this move is better
            if (eval < minEval) {
                minEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
                
                foundPV = true;
            }
            
            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::ALPHA; // Fail low
                break;
            }
        }
        
        // Store result in transposition table
        if (minEval > originalAlpha && minEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);
        
        return minEval;
    }
}

// Regular alpha-beta search (kept for reference/fallback)
int Engine::alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, 
                    std::vector<Move>& pv, uint64_t hashKey, int ply, Move lastMove) {
    // Track nodes searched
    nodesSearched++;
    
    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(), Position());
    int score;
    
    pv.clear();
    
    // Probe the transposition table
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, ttMove)) {
        return score; // Return cached result if available (but don't use TT at root)
    }
    
    // If we've reached the maximum depth, use quiescence search
    if (depth == 0) {
        return quiescenceSearch(board, alpha, beta, hashKey, ply);
    }
    // If we've reached the maximum depth, use quiescence search
     if (depth == 0) {
        return quiescenceSearch(board, alpha, beta, hashKey, ply);
    }
    
    // If the game is over, return the evaluation
    if (board.isCheckmate() || board.isStalemate()) {
        return evaluatePosition(board);
    }
    
    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty()) {
        if (board.isInCheck()) {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        } else {
            // Stalemate
            return 0;
        }
    }
    
    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto& move : legalMoves) {
        int moveScore = getMoveScore(move, board, ttMove, principalVariation, ply, board.getSideToMove(), lastMove);
        
        // Early pruning of very bad captures
        if (depth >= 3) {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece) {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2) {
                    continue;
                }
            }
        }
        
        scoredMoves.push_back(std::make_pair(moveScore, move));
    }
    
    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(), 
              [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) {
                  return a.first > b.first;
              });
    
    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];
    
    // This will be used to store the principal variation
    std::vector<Move> childPV;
    
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        
        for (const auto& scoredMove : scoredMoves) {
            const Move& move = scoredMove.second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, false, childPV, newHashKey, ply + 1, move);
            
            // Update the best move if this move is better
            if (eval > maxEval) {
                maxEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }
            
            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::BETA; // Fail high
                break;
            }
        }
        
        // Store result in transposition table
        if (maxEval > originalAlpha && maxEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);
        
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        
        for (const auto& scoredMove : scoredMoves) {
            const Move& move = scoredMove.second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, true, childPV, newHashKey, ply + 1, move);
            
            // Update the best move if this move is better
            if (eval < minEval) {
                minEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }
            
            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::ALPHA; // Fail low
                break;
            }
        }
        
        // Store result in transposition table
        if (minEval > originalAlpha && minEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);
        
        return minEval;
    }
}

// Evaluation function
int Engine::evaluatePosition(const Board& board) {
    int whiteScore = 0;
    int blackScore = 0;
    bool isEndgamePhase = isEndgame(board);
    
    // Loop through all squares on the board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);
            
            if (!piece) continue;
            
            // Base piece value
            int pieceValue = 0;
            
            // Positional value based on piece-square tables
            int positionalValue = 0;
            
            // Calculate the index for the piece-square tables
            int tableIndex = row * 8 + col;
            
            // Adjust the index for black pieces (mirror the board)
            int blackTableIndex = (7 - row) * 8 + col;
            
            switch (piece->getType()) {
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
                    if (isEndgamePhase) {
                        positionalValue = kingEndGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                    } else {
                        positionalValue = kingMiddleGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                    }
                    break;
                    
                default:
                    break;
            }
            
            // Add the piece value and positional value to the appropriate side's score
            if (piece->getColor() == Color::WHITE) {
                whiteScore += pieceValue + positionalValue;
            } else {
                blackScore += pieceValue + positionalValue;
            }
        }
    }
    
    // Check for checkmate and stalemate
    if (board.isCheckmate()) {
        if (board.getSideToMove() == Color::WHITE) {
            // Black wins
            return -100000;
        } else {
            // White wins
            return 100000;
        }
    } else if (board.isStalemate()) {
        // Draw
        return 0;
    }
    
    // Calculate the final score from white's perspective
    int score = whiteScore - blackScore;
    
    // Adjust the score based on the side to move
    return board.getSideToMove() == Color::WHITE ? score : -score;
}

bool Engine::isEndgame(const Board& board) const {
    int pieceCount = 0;
    bool whiteQueenPresent = false;
    bool blackQueenPresent = false;
    
    // Count the number of pieces and check if queens are present
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            
            if (piece && piece->getType() != PieceType::KING && piece->getType() != PieceType::PAWN) {
                pieceCount++;
                
                if (piece->getType() == PieceType::QUEEN) {
                    if (piece->getColor() == Color::WHITE) {
                        whiteQueenPresent = true;
                    } else {
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
