#ifndef BOARD_H
#define BOARD_H

#include "main.h"
#include "piece.h"
#include "piece_types.h"
#include "board_state.h"

class Board {
private:
    std::vector<std::vector<std::shared_ptr<Piece>>> squares;
    Color sideToMove;
    bool whiteCanCastleKingside;
    bool whiteCanCastleQueenside;
    bool blackCanCastleKingside;
    bool blackCanCastleQueenside;
    Position enPassantTarget;
    int halfMoveClock; // for 50-move rule
    int fullMoveNumber;
    std::shared_ptr<King> whiteKing;
    std::shared_ptr<King> blackKing;

public:
    Board();
    
    // Initialize the board with the standard chess starting position
    void setupStartingPosition();
    
    // Initialize the board from a FEN string
    void setupFromFEN(const std::string& fen);
    
    // Get the current FEN representation of the board
    std::string toFEN() const;
    
    // Get a piece at a specific position, or nullptr if empty
    std::shared_ptr<Piece> getPieceAt(const Position& pos) const;
    
    // Set a piece at a specific position
    void setPieceAt(const Position& pos, std::shared_ptr<Piece> piece);
    
 // Method declarations for Board class
bool makeMove(const Move& move);
bool makeMove(const Move& move, BoardState& previousState);
bool unmakeMove(const Move& move, const BoardState& previousState);
    
    // Generate all legal moves for the current side to move
    std::vector<Move> generateLegalMoves() const;
    
    // Check if the current side to move is in check
    bool isInCheck() const;
    
    // Check if the current side to move is in checkmate
    bool isCheckmate() const;
    
    // Check if the game is in stalemate
    bool isStalemate() const;

    // Add to board.h public section:
bool isGameOver() const;

// Add this enum before the Board class declaration in board.h:
enum class GameEndType {
    NOT_ENDED,
    CHECKMATE,
    STALEMATE
};

// Add this method declaration in the public section:
GameEndType getGameEndType() const;
    
    // Get the side to move
    Color getSideToMove() const { return sideToMove; }
    
    // Switch the side to move
    void switchSideToMove() { sideToMove = (sideToMove == Color::WHITE) ? Color::BLACK : Color::WHITE; }
    
    // Check if a square is attacked by a piece of the specified color
    bool isSquareAttacked(const Position& pos, Color attackerColor) const;
    
    // Castling rights accessors
    bool getWhiteCanCastleKingside() const { return whiteCanCastleKingside; }
    bool getWhiteCanCastleQueenside() const { return whiteCanCastleQueenside; }
    bool getBlackCanCastleKingside() const { return blackCanCastleKingside; }
    bool getBlackCanCastleQueenside() const { return blackCanCastleQueenside; }
    
    // En passant target accessor
    Position getEnPassantTarget() const { return enPassantTarget; }
    
    // Print the board to the console
    void print() const;
    
    // Clear the board
    void clear();
    
private:
    // Check if a castling move is legal
    bool canCastle(const Move& move) const;

    // Add this line to board.h private section:
bool isValidMovePattern(std::shared_ptr<Piece> piece, const Move& move) const;

    // Add these to board.h in the private section:
bool canPieceAttackSquareAfterMove(std::shared_ptr<Piece> piece, Position piecePos, 
                                  Position target, const Move& simulatedMove) const;
bool canPieceAttackSquareSimple(PieceType pieceType, Position from, Position to) const;
bool isPathClearForMove(Position from, Position to) const;
    
    // Helper to verify king safety after move
    bool wouldBeInCheck(const Move& move, Color kingColor) const;
};

#endif // BOARD_H