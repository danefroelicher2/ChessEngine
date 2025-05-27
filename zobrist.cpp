#include "zobrist.h"
#include "board.h"
#include <random>

// Constructor - automatically initializes
Zobrist::Zobrist() : initialized(false) {
    initialize();
}

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
    if (!initialized) initialize();
    
    uint64_t newKey = currentKey;
    
    // Step 1: Toggle side to move (this happens for every move)
    newKey ^= sideToMoveKey;
    
    // Step 2: Remove old castling rights from hash
    if (board.getWhiteCanCastleKingside()) newKey ^= castlingKeys[0];
    if (board.getWhiteCanCastleQueenside()) newKey ^= castlingKeys[1];
    if (board.getBlackCanCastleKingside()) newKey ^= castlingKeys[2];
    if (board.getBlackCanCastleQueenside()) newKey ^= castlingKeys[3];
    
    // Step 3: Remove old en passant from hash
    Position oldEP = board.getEnPassantTarget();
    if (oldEP.isValid()) {
        newKey ^= enPassantKeys[oldEP.col];
    }
    
    // Step 4: Handle piece movement
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece) return newKey; // Should not happen
    
    Color movingColor = movingPiece->getColor();
    int pieceType = static_cast<int>(movingPiece->getType());
    int fromIndex = move.from.row * 8 + move.from.col;
    int toIndex = move.to.row * 8 + move.to.col;
    int colorIndex = (movingColor == Color::WHITE) ? 0 : 1;
    
    // Remove piece from source square
    newKey ^= pieceKeys[pieceType][colorIndex][fromIndex];
    
    // Step 5: Handle captures (including en passant)
    auto capturedPiece = board.getPieceAt(move.to);
    if (capturedPiece) {
        // Regular capture
        int capturedType = static_cast<int>(capturedPiece->getType());
        int capturedColorIndex = (capturedPiece->getColor() == Color::WHITE) ? 0 : 1;
        newKey ^= pieceKeys[capturedType][capturedColorIndex][toIndex];
    } else if (pieceType == static_cast<int>(PieceType::PAWN) && move.to == board.getEnPassantTarget()) {
        // En passant capture - remove the captured pawn
        int capturedPawnRow = (movingColor == Color::WHITE) ? move.to.row - 1 : move.to.row + 1;
        int capturedPawnIndex = capturedPawnRow * 8 + move.to.col;
        int opponentColorIndex = 1 - colorIndex;
        newKey ^= pieceKeys[static_cast<int>(PieceType::PAWN)][opponentColorIndex][capturedPawnIndex];
    }
    
    // Step 6: Add piece to destination (handle promotion)
    if (move.promotion != PieceType::NONE) {
        // Pawn promotion - add promoted piece
        newKey ^= pieceKeys[static_cast<int>(move.promotion)][colorIndex][toIndex];
    } else {
        // Regular move - add same piece to destination
        newKey ^= pieceKeys[pieceType][colorIndex][toIndex];
    }
    
    // Step 7: Handle castling rook movement
    if (pieceType == static_cast<int>(PieceType::KING)) {
        if (move.from.col == 4 && move.to.col == 6) { // Kingside castling
            int rookFromIndex = move.from.row * 8 + 7;
            int rookToIndex = move.from.row * 8 + 5;
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookFromIndex];
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookToIndex];
        } else if (move.from.col == 4 && move.to.col == 2) { // Queenside castling
            int rookFromIndex = move.from.row * 8 + 0;
            int rookToIndex = move.from.row * 8 + 3;
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookFromIndex];
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookToIndex];
        }
    }
    
    // Step 8: Calculate new castling rights (after the move)
    bool newWK = board.getWhiteCanCastleKingside();
    bool newWQ = board.getWhiteCanCastleQueenside();
    bool newBK = board.getBlackCanCastleKingside();
    bool newBQ = board.getBlackCanCastleQueenside();
    
    // Update castling rights based on this move
    if (pieceType == static_cast<int>(PieceType::KING)) {
        // King move removes all castling rights for that color
        if (movingColor == Color::WHITE) {
            newWK = newWQ = false;
        } else {
            newBK = newBQ = false;
        }
    } else if (pieceType == static_cast<int>(PieceType::ROOK)) {
        // Rook move from corner squares
        if (movingColor == Color::WHITE && move.from.row == 0) {
            if (move.from.col == 0) newWQ = false;      // a1 rook
            else if (move.from.col == 7) newWK = false; // h1 rook
        } else if (movingColor == Color::BLACK && move.from.row == 7) {
            if (move.from.col == 0) newBQ = false;      // a8 rook
            else if (move.from.col == 7) newBK = false; // h8 rook
        }
    }
    
    // Rook capture affects castling rights
    if (capturedPiece && capturedPiece->getType() == PieceType::ROOK) {
        if (move.to.row == 0) {
            if (move.to.col == 0) newWQ = false;      // a1 rook captured
            else if (move.to.col == 7) newWK = false; // h1 rook captured
        } else if (move.to.row == 7) {
            if (move.to.col == 0) newBQ = false;      // a8 rook captured
            else if (move.to.col == 7) newBK = false; // h8 rook captured
        }
    }
    
    // Add new castling rights to hash
    if (newWK) newKey ^= castlingKeys[0];
    if (newWQ) newKey ^= castlingKeys[1];
    if (newBK) newKey ^= castlingKeys[2];
    if (newBQ) newKey ^= castlingKeys[3];
    
    // Step 9: Calculate and add new en passant target
    Position newEP;
    if (pieceType == static_cast<int>(PieceType::PAWN) && abs(move.to.row - move.from.row) == 2) {
        // Double pawn move creates en passant target
        int epRow = (movingColor == Color::WHITE) ? move.from.row + 1 : move.from.row - 1;
        newEP = Position(epRow, move.from.col);
    }
    
    // Add new en passant to hash
    if (newEP.isValid()) {
        newKey ^= enPassantKeys[newEP.col];
    }
    
    return newKey;
}