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
    if (!initialized) initialize();
    
    uint64_t newKey = currentKey;
    
    // Get the piece being moved (BEFORE the move was made - we need the original position)
    // Note: This assumes the move hasn't been made yet on the board
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece) return newKey;
    
    Color movingColor = movingPiece->getColor();
    int pieceType = static_cast<int>(movingPiece->getType());
    int fromIndex = move.from.row * 8 + move.from.col;
    int toIndex = move.to.row * 8 + move.to.col;
    int colorIndex = (movingColor == Color::WHITE) ? 0 : 1;
    
    // Remove piece from source square
    newKey ^= pieceKeys[pieceType][colorIndex][fromIndex];
    
    // Handle captures (including en passant)
    auto capturedPiece = board.getPieceAt(move.to);
    if (capturedPiece) {
        int capturedType = static_cast<int>(capturedPiece->getType());
        int capturedColorIndex = (capturedPiece->getColor() == Color::WHITE) ? 0 : 1;
        newKey ^= pieceKeys[capturedType][capturedColorIndex][toIndex];
    }
    // Handle en passant capture
    else if (pieceType == static_cast<int>(PieceType::PAWN) && move.to == board.getEnPassantTarget()) {
        int capturedPawnRow = (movingColor == Color::WHITE) ? move.to.row - 1 : move.to.row + 1;
        int capturedPawnIndex = capturedPawnRow * 8 + move.to.col;
        int opponentColorIndex = 1 - colorIndex;
        newKey ^= pieceKeys[static_cast<int>(PieceType::PAWN)][opponentColorIndex][capturedPawnIndex];
    }
    
    // Handle promotion
    if (move.promotion != PieceType::NONE) {
        // Add the promoted piece instead of the pawn
        newKey ^= pieceKeys[static_cast<int>(move.promotion)][colorIndex][toIndex];
    } else {
        // Add the moving piece to the destination square
        newKey ^= pieceKeys[pieceType][colorIndex][toIndex];
    }
    
    // Handle castling rook movement
    if (pieceType == static_cast<int>(PieceType::KING)) {
        // Kingside castling
        if (move.from.col == 4 && move.to.col == 6) {
            int rookFromIndex = move.from.row * 8 + 7;
            int rookToIndex = move.from.row * 8 + 5;
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookFromIndex];
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookToIndex];
        }
        // Queenside castling
        else if (move.from.col == 4 && move.to.col == 2) {
            int rookFromIndex = move.from.row * 8 + 0;
            int rookToIndex = move.from.row * 8 + 3;
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookFromIndex];
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookToIndex];
        }
    }
    
    // Update castling rights (get current state)
    bool oldWhiteKingside = board.getWhiteCanCastleKingside();
    bool oldWhiteQueenside = board.getWhiteCanCastleQueenside();
    bool oldBlackKingside = board.getBlackCanCastleKingside();
    bool oldBlackQueenside = board.getBlackCanCastleQueenside();
    
    // Calculate new castling rights
    bool newWhiteKingside = oldWhiteKingside;
    bool newWhiteQueenside = oldWhiteQueenside;
    bool newBlackKingside = oldBlackKingside;
    bool newBlackQueenside = oldBlackQueenside;
    
    // King moves lose all castling rights for that side
    if (pieceType == static_cast<int>(PieceType::KING)) {
        if (movingColor == Color::WHITE) {
            newWhiteKingside = false;
            newWhiteQueenside = false;
        } else {
            newBlackKingside = false;
            newBlackQueenside = false;
        }
    }
    
    // Rook moves lose castling rights for that side
    if (pieceType == static_cast<int>(PieceType::ROOK)) {
        if (movingColor == Color::WHITE) {
            if (move.from.row == 0 && move.from.col == 0) newWhiteQueenside = false;
            if (move.from.row == 0 && move.from.col == 7) newWhiteKingside = false;
        } else {
            if (move.from.row == 7 && move.from.col == 0) newBlackQueenside = false;
            if (move.from.row == 7 && move.from.col == 7) newBlackKingside = false;
        }
    }
    
    // Rook captures lose castling rights
    if (capturedPiece && capturedPiece->getType() == PieceType::ROOK) {
        if (move.to.row == 0 && move.to.col == 0) newWhiteQueenside = false;
        if (move.to.row == 0 && move.to.col == 7) newWhiteKingside = false;
        if (move.to.row == 7 && move.to.col == 0) newBlackQueenside = false;
        if (move.to.row == 7 && move.to.col == 7) newBlackKingside = false;
    }
    
    // Update hash for changed castling rights
    if (oldWhiteKingside != newWhiteKingside) newKey ^= castlingKeys[0];
    if (oldWhiteQueenside != newWhiteQueenside) newKey ^= castlingKeys[1];
    if (oldBlackKingside != newBlackKingside) newKey ^= castlingKeys[2];
    if (oldBlackQueenside != newBlackQueenside) newKey ^= castlingKeys[3];
    
    // Handle en passant target changes
    Position oldEnPassant = board.getEnPassantTarget();
    if (oldEnPassant.isValid()) {
        newKey ^= enPassantKeys[oldEnPassant.col];
    }
    
    // Set new en passant target for double pawn pushes
    if (pieceType == static_cast<int>(PieceType::PAWN) && abs(move.to.row - move.from.row) == 2) {
        newKey ^= enPassantKeys[move.from.col];
    }
    
    // Toggle side to move
    newKey ^= sideToMoveKey;
    
    return newKey;
}