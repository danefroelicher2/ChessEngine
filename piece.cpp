#include "piece.h"
#include "board.h"

bool Piece::isBasicallyValid(const Position& pos, const Board& board) const {
    if (!pos.isValid()) return false;
    
    // Check if destination has a piece of same color
    auto pieceAtDest = board.getPieceAt(pos);
    if (pieceAtDest && pieceAtDest->getColor() == color) {
        return false;
    }
    
    return true;
}
k
char Piece::toChar() const {
    char c = ' ';
    switch (type) {
        case PieceType::PAWN: c = 'p'; break;
        case PieceType::KNIGHT: c = 'n'; break;
        case PieceType::BISHOP: c = 'b'; break;
        case PieceType::ROOK: c = 'r'; break;
        case PieceType::QUEEN: c = 'q'; break;
        case PieceType::KING: c = 'k'; break;
        default: c = '?'; break;
    }
    
    if (color == Color::WHITE) {
        c = std::toupper(c);
    }
    
    return c;
}