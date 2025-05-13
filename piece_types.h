#ifndef PIECE_TYPES_H
#define PIECE_TYPES_H

#include "piece.h"

class Pawn : public Piece {
public:
    Pawn(Color c, Position pos) : Piece(PieceType::PAWN, c, pos) {}
    std::vector<Move> getLegalMoves(const Board& board) const override;
};

class Knight : public Piece {
public:
    Knight(Color c, Position pos) : Piece(PieceType::KNIGHT, c, pos) {}
    std::vector<Move> getLegalMoves(const Board& board) const override;
};

class Bishop : public Piece {
public:
    Bishop(Color c, Position pos) : Piece(PieceType::BISHOP, c, pos) {}
    std::vector<Move> getLegalMoves(const Board& board) const override;
};

class Rook : public Piece {
public:
    Rook(Color c, Position pos) : Piece(PieceType::ROOK, c, pos) {}
    std::vector<Move> getLegalMoves(const Board& board) const override;
};

class Queen : public Piece {
public:
    Queen(Color c, Position pos) : Piece(PieceType::QUEEN, c, pos) {}
    std::vector<Move> getLegalMoves(const Board& board) const override;
};

class King : public Piece {
public:
    King(Color c, Position pos) : Piece(PieceType::KING, c, pos) {}
    std::vector<Move> getLegalMoves(const Board& board) const override;
};

#endif // PIECE_TYPES_H