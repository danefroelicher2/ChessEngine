#include "zobrist.h"
#include "board.h"
#include <random>

// Initialize static members
uint64_t Zobrist::pieceKeys[6][2][64];
uint64_t Zobrist::sideToMoveKey;
uint64_t Zobrist::castlingKeys[4];
uint64_t Zobrist::enPassantKeys[8];
bool Zobrist::initialized = false;

void Zobrist::initialize() {
    if (initialized) return;
    
    // Use a good random number generator
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    
    // Generate random numbers for pieces
    for (int pieceType = 0; pieceType < 6; pieceType++) {
        for (int color = 0; color < 2; color++) {
            for (int pos = 0; pos < 64; pos++) {
                pieceKeys[pieceType][color][pos] = dist(gen);
            }
        }
    }
    
    // Generate random number for side to move
    sideToMoveKey = dist(gen);
    
    // Generate random numbers for castling rights
    for (int i = 0; i < 4; i++) {
        castlingKeys[i] = dist(gen);
    }
    
    // Generate random numbers for en passant files
    for (int file = 0; file < 8; file++) {
        enPassantKeys[file] = dist(gen);
    }
    
    initialized = true;
}

uint64_t Zobrist::generateHashKey(const Board& board) {
    if (!initialized) initialize();
    
    uint64_t key = 0;
    
    // Hash pieces
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            std::shared_ptr<Piece> piece = board.getPieceAt(pos);
            
            if (piece) {
                int pieceType = static_cast<int>(piece->getType());
                int colorIndex = (piece->getColor() == Color::WHITE) ? 0 : 1;
                int squareIndex = row * 8 + col;
                
                key ^= pieceKeys[pieceType][colorIndex][squareIndex];
            }
        }
    }
    
    // Hash side to move
    if (board.getSideToMove() == Color::BLACK) {
        key ^= sideToMoveKey;
    }
    
    // Hash castling rights
    if (board.getWhiteCanCastleKingside()) key ^= castlingKeys[0];
    if (board.getWhiteCanCastleQueenside()) key ^= castlingKeys[1];
    if (board.getBlackCanCastleKingside()) key ^= castlingKeys[2];
    if (board.getBlackCanCastleQueenside()) key ^= castlingKeys[3];
    
    // Hash en passant
    Position ep = board.getEnPassantTarget();
    if (ep.isValid()) {
        key ^= enPassantKeys[ep.col];
    }
    
    return key;
}

uint64_t Zobrist::updateHashKey(uint64_t currentKey, const Move& move, const Board& board) {
    // This is a more complex function to implement efficiently
    // For now, we'll just regenerate the hash after the move
    // In a more advanced implementation, you would efficiently update the key
    // based on what changed with the move
    
    // Simple implementation: just regenerate the hash
    // A proper implementation would modify only the changing elements
    
    // Create a copy of the board and make the move
    Board tempBoard = board;
    tempBoard.makeMove(move);
    
    // Return the hash of the new position
    return generateHashKey(tempBoard);
}