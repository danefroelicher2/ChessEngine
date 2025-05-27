#include "piece_types.h"
#include "board.h"

// Pawn movement logic
std::vector<Move> Pawn::getLegalMoves(const Board& board) const {
    std::vector<Move> moves;
    int direction = (color == Color::WHITE) ? 1 : -1;
    Position front(position.row + direction, position.col);
    
    // Forward move (1 square)
    if (front.isValid() && !board.getPieceAt(front)) {
        // Check for promotion
        if (front.row == 0 || front.row == 7) {
            moves.emplace_back(position, front, PieceType::QUEEN);
            moves.emplace_back(position, front, PieceType::ROOK);
            moves.emplace_back(position, front, PieceType::BISHOP);
            moves.emplace_back(position, front, PieceType::KNIGHT);
        } else {
            moves.emplace_back(position, front);
        }

        if (front.isValid() && !board.getPieceAt(front)) {
    // Check for promotion
    if (front.row == 0 || front.row == 7) {
        // Always add all promotion options for completeness
        moves.emplace_back(position, front, PieceType::QUEEN);
        moves.emplace_back(position, front, PieceType::ROOK);
        moves.emplace_back(position, front, PieceType::BISHOP);
        moves.emplace_back(position, front, PieceType::KNIGHT);
    } else {
        moves.emplace_back(position, front);
    }
        
        // Forward move (2 squares) if pawn is on starting row
        if ((color == Color::WHITE && position.row == 1) || 
            (color == Color::BLACK && position.row == 6)) {
            Position doubleFront(position.row + 2 * direction, position.col);
            if (doubleFront.isValid() && !board.getPieceAt(doubleFront)) {
                moves.emplace_back(position, doubleFront);
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
                // Check for promotion
                if (capturePos.row == 0 || capturePos.row == 7) {
                    moves.emplace_back(position, capturePos, PieceType::QUEEN);
                    moves.emplace_back(position, capturePos, PieceType::ROOK);
                    moves.emplace_back(position, capturePos, PieceType::BISHOP);
                    moves.emplace_back(position, capturePos, PieceType::KNIGHT);
                } else {
                    moves.emplace_back(position, capturePos);
                }
            }
            // En passant capture
            else if (capturePos == board.getEnPassantTarget()) {
                moves.emplace_back(position, capturePos);
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