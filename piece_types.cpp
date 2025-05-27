#include "piece_types.h"
#include "board.h"

// Pawn::getLegalMoves() 
std::vector<Move> Pawn::getLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    int direction = (color == Color::WHITE) ? 1 : -1;
    Position front(position.row + direction, position.col);
    
    // Helper function to add promotion moves correctly
    auto addPromotionMoves = [&](Position from, Position to) {
        // ENHANCED: Validate promotion is only on correct ranks
        bool isPromotionRank = (color == Color::WHITE && to.row == 7) || 
                              (color == Color::BLACK && to.row == 0);
        
        if (isPromotionRank) {
            // Add all four promotion options
            moves.emplace_back(from, to, PieceType::QUEEN);
            moves.emplace_back(from, to, PieceType::ROOK);
            moves.emplace_back(from, to, PieceType::BISHOP);
            moves.emplace_back(from, to, PieceType::KNIGHT);
        } else {
            // Regular move (no promotion)
            moves.emplace_back(from, to);
        }
    };
    
    // Forward move (1 square)
    if (front.isValid() && !board.getPieceAt(front)) {
        addPromotionMoves(position, front);
        
        // Forward move (2 squares) if pawn is on starting row
        bool isStartingRank = (color == Color::WHITE && position.row == 1) || 
                             (color == Color::BLACK && position.row == 6);
        
        if (isStartingRank) {
            Position doubleFront(position.row + 2 * direction, position.col);
            if (doubleFront.isValid() && !board.getPieceAt(doubleFront)) {
                moves.emplace_back(position, doubleFront); // No promotion on double move
            }
        }
    }
    
    // Capture moves (including en passant)
    for (int dCol : {-1, 1}) {
        Position capturePos(position.row + direction, position.col + dCol);
        
        if (capturePos.isValid()) {
            auto pieceAtCapture = board.getPieceAt(capturePos);
            
            // Regular capture
            if (pieceAtCapture && pieceAtCapture->getColor() != color) {
                addPromotionMoves(position, capturePos);
            }
         // En passant capture - CRITICAL FIX: Complete validation
else if (capturePos == board.getEnPassantTarget() && board.getEnPassantTarget().isValid()) {
    // CRITICAL FIX: Validate en passant rank restrictions
    bool validEnPassantRank = false;
    if (color == Color::WHITE && position.row == 4 && capturePos.row == 5) {
        validEnPassantRank = true; // White pawn on 5th rank capturing to 6th
    } else if (color == Color::BLACK && position.row == 3 && capturePos.row == 2) {
        validEnPassantRank = true; // Black pawn on 4th rank capturing to 3rd
    }
    
    if (validEnPassantRank) {
        // Validate that there's actually a pawn to capture
        int capturedPawnRow = (color == Color::WHITE) ? capturePos.row - 1 : capturePos.row + 1;
        auto capturedPawn = board.getPieceAt(Position(capturedPawnRow, capturePos.col));
        
        if (capturedPawn && 
            capturedPawn->getType() == PieceType::PAWN && 
            capturedPawn->getColor() != color) {
            
            // CRITICAL FIX: Additional validation - the captured pawn should be on correct rank
            bool capturedPawnOnCorrectRank = (color == Color::WHITE && capturedPawnRow == 4) ||
                                           (color == Color::BLACK && capturedPawnRow == 3);
            
            if (capturedPawnOnCorrectRank) {
                // En passant is valid (never results in promotion)
                moves.emplace_back(position, capturePos);
            }
        }
    }
}
        }
    }
    
    return moves;
}

// Knight movement logic
std::vector<Move> Knight::getLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> knightOffsets = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    for (const auto& offset : knightOffsets) {
        Position newPos(position.row + offset.first, position.col + offset.second);
        
        if (isBasicallyValid(newPos, board)) {
            moves.emplace_back(position, newPos);
        }
    }
    
    return moves;
}

// Bishop movement logic
std::vector<Move> Bishop::getLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };
    
    for (const auto& dir : directions) {
        for (int distance = 1; distance < 8; distance++) {
            Position newPos(position.row + dir.first * distance, 
                          position.col + dir.second * distance);
            
            if (!newPos.isValid()) break;
            
            auto pieceAtDest = board.getPieceAt(newPos);
            
            if (!pieceAtDest) {
                // Empty square, can move here
                moves.emplace_back(position, newPos);
            } else if (pieceAtDest->getColor() != color) {
                // Capture opponent's piece
                moves.emplace_back(position, newPos);
                break;
            } else {
                // Blocked by own piece
                break;
            }
        }
    }
    
    return moves;
}

// Rook movement logic
std::vector<Move> Rook::getLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> directions = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };
    
    for (const auto& dir : directions) {
        for (int distance = 1; distance < 8; distance++) {
            Position newPos(position.row + dir.first * distance, 
                          position.col + dir.second * distance);
            
            if (!newPos.isValid()) break;
            
            auto pieceAtDest = board.getPieceAt(newPos);
            
            if (!pieceAtDest) {
                // Empty square, can move here
                moves.emplace_back(position, newPos);
            } else if (pieceAtDest->getColor() != color) {
                // Capture opponent's piece
                moves.emplace_back(position, newPos);
                break;
            } else {
                // Blocked by own piece
                break;
            }
        }
    }
    
    return moves;
}

// Queen movement logic
std::vector<Move> Queen::getLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    for (const auto& dir : directions) {
        for (int distance = 1; distance < 8; distance++) {
            Position newPos(position.row + dir.first * distance, 
                          position.col + dir.second * distance);
            
            if (!newPos.isValid()) break;
            
            auto pieceAtDest = board.getPieceAt(newPos);
            
            if (!pieceAtDest) {
                // Empty square, can move here
                moves.emplace_back(position, newPos);
            } else if (pieceAtDest->getColor() != color) {
                // Capture opponent's piece
                moves.emplace_back(position, newPos);
                break;
            } else {
                // Blocked by own piece
                break;
            }
        }
    }
    
    return moves;
}

// King movement logic
std::vector<Move> King::getLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    const std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    // Regular moves
    for (const auto& dir : directions) {
        Position newPos(position.row + dir.first, position.col + dir.second);
        
        if (isBasicallyValid(newPos, board)) {
            moves.emplace_back(position, newPos);
        }
    }
    
    // Castling moves
    if (!hasMoved && !board.isInCheck()) {
        // Kingside castling
        Position kingsidePos(position.row, position.col + 2);
        if (position.col == 4 && 
            ((color == Color::WHITE && board.getWhiteCanCastleKingside()) ||
             (color == Color::BLACK && board.getBlackCanCastleKingside()))) {
            
            // Check if squares between king and rook are empty
            if (!board.getPieceAt(Position(position.row, position.col + 1)) &&
                !board.getPieceAt(Position(position.row, position.col + 2))) {
                
                // Check if the king would move through or into check
                if (!board.isSquareAttacked(Position(position.row, position.col + 1), 
                                         (color == Color::WHITE) ? Color::BLACK : Color::WHITE) &&
                    !board.isSquareAttacked(Position(position.row, position.col + 2), 
                                         (color == Color::WHITE) ? Color::BLACK : Color::WHITE)) {
                    
                    moves.emplace_back(position, kingsidePos);
                }
            }
        }
        
        // Queenside castling
        Position queensidePos(position.row, position.col - 2);
        if (position.col == 4 && 
            ((color == Color::WHITE && board.getWhiteCanCastleQueenside()) ||
             (color == Color::BLACK && board.getBlackCanCastleQueenside()))) {
            
            // Check if squares between king and rook are empty
            if (!board.getPieceAt(Position(position.row, position.col - 1)) &&
                !board.getPieceAt(Position(position.row, position.col - 2)) &&
                !board.getPieceAt(Position(position.row, position.col - 3))) {
                
                // Check if the king would move through or into check
                if (!board.isSquareAttacked(Position(position.row, position.col - 1), 
                                         (color == Color::WHITE) ? Color::BLACK : Color::WHITE) &&
                    !board.isSquareAttacked(Position(position.row, position.col - 2), 
                                         (color == Color::WHITE) ? Color::BLACK : Color::WHITE)) {
                    
                    moves.emplace_back(position, queensidePos);
                }
            }
        }
    }
    
    return moves;
}