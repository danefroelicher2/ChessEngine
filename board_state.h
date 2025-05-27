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
    
    // CRITICAL FIX: Add destructor to ensure proper cleanup
    ~BoardState() {
        // Explicitly reset shared_ptr to ensure cleanup
        if (capturedPiece) {
            capturedPiece.reset();
        }
    }
    
    // CRITICAL FIX: Add copy constructor for safe copying
    BoardState(const BoardState& other) :
        sideToMove(other.sideToMove),
        whiteCanCastleKingside(other.whiteCanCastleKingside),
        whiteCanCastleQueenside(other.whiteCanCastleQueenside),
        blackCanCastleKingside(other.blackCanCastleKingside),
        blackCanCastleQueenside(other.blackCanCastleQueenside),
        enPassantTarget(other.enPassantTarget),
        halfMoveClock(other.halfMoveClock),
        fullMoveNumber(other.fullMoveNumber),
        capturedPiece(other.capturedPiece), // shared_ptr handles reference counting
        wasEnPassant(other.wasEnPassant),
        wasPromotion(other.wasPromotion),
        originalType(other.originalType),
        pieceHasMoved(other.pieceHasMoved) {}
    
    // CRITICAL FIX: Add assignment operator
    BoardState& operator=(const BoardState& other) {
        if (this != &other) {
            sideToMove = other.sideToMove;
            whiteCanCastleKingside = other.whiteCanCastleKingside;
            whiteCanCastleQueenside = other.whiteCanCastleQueenside;
            blackCanCastleKingside = other.blackCanCastleKingside;
            blackCanCastleQueenside = other.blackCanCastleQueenside;
            enPassantTarget = other.enPassantTarget;
            halfMoveClock = other.halfMoveClock;
            fullMoveNumber = other.fullMoveNumber;
            capturedPiece = other.capturedPiece;
            wasEnPassant = other.wasEnPassant;
            wasPromotion = other.wasPromotion;
            originalType = other.originalType;
            pieceHasMoved = other.pieceHasMoved;
        }
        return *this;
    }
};

#endif // BOARD_STATE_H