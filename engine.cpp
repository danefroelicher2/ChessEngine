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
    
    // Use alpha-beta pruning to find the best move
    alphaBeta(board, maxDepth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 
              board.getSideToMove() == Color::WHITE, bestMove, hashKey);
    
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

int Engine::alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, Move& bestMove, uint64_t hashKey) {
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
    
    // If we have a promising move from the transposition table, try it first
    if (ttMove.from.isValid() && ttMove.to.isValid()) {
        auto it = std::find_if(legalMoves.begin(), legalMoves.end(), 
                              [&ttMove](const Move& m) { 
                                  return m.from.row == ttMove.from.row && 
                                         m.from.col == ttMove.from.col && 
                                         m.to.row == ttMove.to.row && 
                                         m.to.col == ttMove.to.col; 
                              });
        
        if (it != legalMoves.end()) {
            // Move the transposition table move to the front
            std::swap(*it, legalMoves[0]);
        }
    }
    
    // Add some move ordering heuristics - try captures first
    std::stable_sort(legalMoves.begin(), legalMoves.end(), 
                    [&board](const Move& a, const Move& b) {
                        bool aCaptures = board.getPieceAt(a.to) != nullptr;
                        bool bCaptures = board.getPieceAt(b.to) != nullptr;
                        return aCaptures && !bCaptures; // true if a captures and b doesn't
                    });
    
    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];
    
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        
        for (const auto& move : legalMoves) {
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            Move dummy(Position(), Position());
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, false, dummy, newHashKey);
            
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
        
        for (const auto& move : legalMoves) {
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            Move dummy(Position(), Position());
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, true, dummy, newHashKey);
            
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