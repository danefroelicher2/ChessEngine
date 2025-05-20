#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include "main.h"
#include "piece.h"

// Structure to store board state for move reversal
struct BoardState {
    Color sideToMove;
    bool whiteCanCastleKingside;
    bool whiteCanCastleQueenside;
    bool blackCanCastleKingside;
    bool blackCanCastleQueenside;
    Position enPassantTarget;
    int halfMoveClock;
    int fullMoveNumber;
    std::shared_ptr<Piece> capturedPiece;
    bool wasEnPassant;
    bool wasPromotion;
    PieceType originalType;
    bool pieceHasMoved;
    
    BoardState() : 
        sideToMove(Color::WHITE), 
        whiteCanCastleKingside(false), 
        whiteCanCastleQueenside(false),
        blackCanCastleKingside(false),
        blackCanCastleQueenside(false),
        enPassantTarget(Position()),
        halfMoveClock(0),
        fullMoveNumber(1),
        capturedPiece(nullptr),
        wasEnPassant(false),
        wasPromotion(false),
        originalType(PieceType::NONE),
        pieceHasMoved(false) {}
};

#endif // BOARD_STATE_H