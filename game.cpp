#include "engine.h"
#include <limits>
#include <random>
#include <chrono>

// Piece-square tables for positional evaluation
// These tables provide bonuses or penalties based on piece positions
// Values are from the perspective of WHITE, they are mirrored for BLACK

// Pawn position table
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

// Knight position table
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

// Bishop position table
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

// Rook position table
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

// Queen position table
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

// King position table for middlegame
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

// King position table for endgame
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

Move Engine::getBestMove() {
    // Get the current board state
    Board board = game.getBoard();
    
    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    // If there are no legal moves, return an invalid move
    if (legalMoves.empty()) {
        return Move(Position(), Position());
    }
    
    // Initialize the best move to the first legal move
    Move bestMove = legalMoves[0];
    
    // Generate the hash key for the current position
    uint64_t hashKey = Zobrist::generateHashKey(board);
    
    // Increment the age of the transposition table
    transpositionTable.incrementAge();
    
    // Clear killer moves and history table for a new search
    clearKillerMoves();
    clearHistoryTable();
    
    // Use alpha-beta pruning to find the best move
    alphaBeta(board, maxDepth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 
              board.getSideToMove() == Color::WHITE, bestMove, hashKey, 0);
    
    // If by some chance we didn't find a valid move, try a simple approach
    if (!bestMove.from.isValid() || !bestMove.to.isValid()) {
        // Find any legal move as a fallback
        for (const auto& move : legalMoves) {
            // Check if capturing a piece
            if (board.getPieceAt(move.to) != nullptr) {
                return move; // Return the first capture move
            }
        }
        // If no captures, return first legal move
        return legalMoves[0];
    }
    
    return bestMove;
}

int Engine::getMVVLVAScore(PieceType attacker, PieceType victim) const {
    // Get the values of pieces for MVV-LVA calculation
    int victimValue;
    switch (victim) {
        case PieceType::PAWN:   victimValue = PAWN_VALUE; break;
        case PieceType::KNIGHT: victimValue = KNIGHT_VALUE; break;
        case PieceType::BISHOP: victimValue = BISHOP_VALUE; break;
        case PieceType::ROOK:   victimValue = ROOK_VALUE; break;
        case PieceType::QUEEN:  victimValue = QUEEN_VALUE; break;
        case PieceType::KING:   victimValue = KING_VALUE; break;
        default:                victimValue = 0; break;
    }
    
    int attackerValue;
    switch (attacker) {
        case PieceType::PAWN:   attackerValue = PAWN_VALUE; break;
        case PieceType::KNIGHT: attackerValue = KNIGHT_VALUE; break;
        case PieceType::BISHOP: attackerValue = BISHOP_VALUE; break;
        case PieceType::ROOK:   attackerValue = ROOK_VALUE; break;
        case PieceType::QUEEN:  attackerValue = QUEEN_VALUE; break;
        case PieceType::KING:   attackerValue = KING_VALUE; break;
        default:                attackerValue = 0; break;
    }
    
    // MVV-LVA score: 10 * victim value - attacker value
    // The multiplier makes victim value more important than attacker value
    return 10 * victimValue - attackerValue;
}

void Engine::storeKillerMove(const Move& move, int ply) {
    // Don't store captures as killer moves, they'll be handled by MVV-LVA
    // Also don't store invalid moves
    if (!move.from.isValid() || !move.to.isValid()) {
        return;
    }
    
    // Don't store the same killer move twice
    if (killerMoves[ply][0].from.row == move.from.row && 
        killerMoves[ply][0].from.col == move.from.col && 
        killerMoves[ply][0].to.row == move.to.row && 
        killerMoves[ply][0].to.col == move.to.col) {
        return;
    }
    
    // Shift the killer moves and add the new one
    killerMoves[ply][1] = killerMoves[ply][0];
    killerMoves[ply][0] = move;
}

bool Engine::isKillerMove(const Move& move, int ply) const {
    // Check if the move matches either of the killer moves at this ply
    for (int i = 0; i < 2; i++) {
        if (killerMoves[ply][i].from.row == move.from.row && 
            killerMoves[ply][i].from.col == move.from.col && 
            killerMoves[ply][i].to.row == move.to.row && 
            killerMoves[ply][i].to.col == move.to.col) {
            return true;
        }
    }
    
    return false;
}

void Engine::updateHistoryScore(const Move& move, int depth, Color color) {
    // Convert position to indices
    int fromIndex = move.from.row * 8 + move.from.col;
    int toIndex = move.to.row * 8 + move.to.col;
    int colorIndex = (color == Color::WHITE) ? 0 : 1;
    
    // Increment history score based on depth
    // Deeper moves get higher scores because they're more likely to be significant
    historyTable[colorIndex][fromIndex][toIndex] += depth * depth;
    
    // Avoid overflow by capping the history score
    if (historyTable[colorIndex][fromIndex][toIndex] > 1000000) {
        // Scale down all history scores for this color
        for (int from = 0; from < 64; from++) {
            for (int to = 0; to < 64; to++) {
                historyTable[colorIndex][from][to] /= 2;
            }
        }
    }
}

int Engine::getHistoryScore(const Move& move, Color color) const {
    // Convert position to indices
    int fromIndex = move.from.row * 8 + move.from.col;
    int toIndex = move.to.row * 8 + move.to.col;
    int colorIndex = (color == Color::WHITE) ? 0 : 1;
    
    // Return the history score
    return historyTable[colorIndex][fromIndex][toIndex];
}

int Engine::getMoveScore(const Move& move, const Board& board, const Move& ttMove, int ply, Color sideToMove) const {
    // 1. Transposition table move gets highest priority
    if (ttMove.from.isValid() && ttMove.to.isValid() &&
        ttMove.from.row == move.from.row && ttMove.from.col == move.from.col &&
        ttMove.to.row == move.to.row && ttMove.to.col == move.to.col) {
        return 4000000; // Highest score
    }
    
    // 2. Captures scored by MVV-LVA
    auto capturedPiece = board.getPieceAt(move.to);
    if (capturedPiece) {
        auto movingPiece = board.getPieceAt(move.from);
        if (movingPiece) {
            return 3000000 + getMVVLVAScore(movingPiece->getType(), capturedPiece->getType());
        }
    }
    
    // 3. Killer moves
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
    
    // 4. History heuristic for non-captures
    return getHistoryScore(move, sideToMove);
}

int Engine::alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, Move& bestMove, uint64_t hashKey, int ply) {
    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(), Position());
    int score;
    
    // Probe the transposition table
    if (transpositionTable.probe(hashKey, depth, alpha, beta, score, ttMove)) {
        return score; // Return cached result if available
    }
    
    // If we've reached the maximum depth or the game is over, evaluate the position
    if (depth == 0 || board.isCheckmate() || board.isStalemate()) {
        return evaluatePosition(board);
    }
    
    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto& move : legalMoves) {
        int moveScore = getMoveScore(move, board, ttMove, ply, board.getSideToMove());
        scoredMoves.push_back(std::make_pair(moveScore, move));
    }
    
    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(), 
              [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) {
                  return a.first > b.first;
              });
    
    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];
    
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
            Move dummy(Position(), Position());
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, false, dummy, newHashKey, ply + 1);
            
            // Update the best move if this move is better
            if (eval > maxEval) {
                maxEval = eval;
                localBestMove = move;
                
                if (depth == maxDepth) {
                    bestMove = move;
                }
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
            Move dummy(Position(), Position());
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, true, dummy, newHashKey, ply + 1);
            
            // Update the best move if this move is better
            if (eval < minEval) {
                minEval = eval;
                localBestMove = move;
                
                if (depth == maxDepth) {
                    bestMove = move;
                }
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