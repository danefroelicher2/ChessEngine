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
    
    // Use alpha-beta pruning to find the best move
    alphaBeta(board, maxDepth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 
              board.getSideToMove() == Color::WHITE, bestMove);
    
    return bestMove;
}

int Engine::alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, Move& bestMove) {
    // If we've reached the maximum depth or the game is over, evaluate the position
    if (depth == 0 || board.isCheckmate() || board.isStalemate()) {
        return evaluatePosition(board);
    }
    
    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    // Randomize move order to add variety
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(legalMoves.begin(), legalMoves.end(), g);
    
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        
        for (const auto& move : legalMoves) {
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Recursively evaluate the position
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, false, bestMove);
            
            // Update the best move if this move is better
            if (eval > maxEval) {
                maxEval = eval;
                
                if (depth == maxDepth) {
                    bestMove = move;
                }
            }
            
            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        
        for (const auto& move : legalMoves) {
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Recursively evaluate the position
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, true, bestMove);
            
            // Update the best move if this move is better
            if (eval < minEval) {
                minEval = eval;
                
                if (depth == maxDepth) {
                    bestMove = move;
                }
            }
            
            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        
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