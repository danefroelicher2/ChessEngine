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
    
    // Toggle side to move first
    newKey ^= sideToMoveKey;
    
    // Note: This method should be called BEFORE making the move on the board
    // So we can still access the original piece positions
    
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece) return newKey;
    
    Color movingColor = movingPiece->getColor();
    int pieceType = static_cast<int>(movingPiece->getType());
    int fromIndex = move.from.row * 8 + move.from.col;
    int toIndex = move.to.row * 8 + move.to.col;
    int colorIndex = (movingColor == Color::WHITE) ? 0 : 1;
    
    // Remove piece from source square
    newKey ^= pieceKeys[pieceType][colorIndex][fromIndex];
    
    // Handle captures
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
    
    // Add piece to destination (or promoted piece)
    if (move.promotion != PieceType::NONE) {
        newKey ^= pieceKeys[static_cast<int>(move.promotion)][colorIndex][toIndex];
    } else {
        newKey ^= pieceKeys[pieceType][colorIndex][toIndex];
    }
    
    // Handle castling rook movement
    if (pieceType == static_cast<int>(PieceType::KING)) {
        if (move.from.col == 4 && move.to.col == 6) { // Kingside
            int rookFromIndex = move.from.row * 8 + 7;
            int rookToIndex = move.from.row * 8 + 5;
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookFromIndex];
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookToIndex];
        }
        else if (move.from.col == 4 && move.to.col == 2) { // Queenside
            int rookFromIndex = move.from.row * 8 + 0;
            int rookToIndex = move.from.row * 8 + 3;
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookFromIndex];
            newKey ^= pieceKeys[static_cast<int>(PieceType::ROOK)][colorIndex][rookToIndex];
        }
    }
    
    // Handle castling rights changes efficiently (XOR only changed rights)
    bool oldWK = board.getWhiteCanCastleKingside();
    bool oldWQ = board.getWhiteCanCastleQueenside();
    bool oldBK = board.getBlackCanCastleKingside();
    bool oldBQ = board.getBlackCanCastleQueenside();
    
    // Calculate new castling rights
    bool newWK = oldWK, newWQ = oldWQ, newBK = oldBK, newBQ = oldBQ;
    
    // Update based on move
    if (pieceType == static_cast<int>(PieceType::KING)) {
        if (movingColor == Color::WHITE) {
            newWK = newWQ = false;
        } else {
            newBK = newBQ = false;
        }
    }
    else if (pieceType == static_cast<int>(PieceType::ROOK)) {
        if (movingColor == Color::WHITE) {
            if (move.from.row == 0 && move.from.col == 0) newWQ = false;
            if (move.from.row == 0 && move.from.col == 7) newWK = false;
        } else {
            if (move.from.row == 7 && move.from.col == 0) newBQ = false;
            if (move.from.row == 7 && move.from.col == 7) newBK = false;
        }
    }
    
    if (capturedPiece && capturedPiece->getType() == PieceType::ROOK) {
        if (move.to.row == 0 && move.to.col == 0) newWQ = false;
        if (move.to.row == 0 && move.to.col == 7) newWK = false;
        if (move.to.row == 7 && move.to.col == 0) newBQ = false;
        if (move.to.row == 7 && move.to.col == 7) newBK = false;
    }
    
    // XOR only the changed castling rights
    if (oldWK != newWK) newKey ^= castlingKeys[0];
    if (oldWQ != newWQ) newKey ^= castlingKeys[1];
    if (oldBK != newBK) newKey ^= castlingKeys[2];
    if (oldBQ != newBQ) newKey ^= castlingKeys[3];
    
    // Handle en passant changes
    Position oldEnPassant = board.getEnPassantTarget();
    if (oldEnPassant.isValid()) {
        newKey ^= enPassantKeys[oldEnPassant.col];
    }
    
    // Add new en passant if double pawn push
    if (pieceType == static_cast<int>(PieceType::PAWN) && abs(move.to.row - move.from.row) == 2) {
        newKey ^= enPassantKeys[move.from.col];
    }
    
    return newKey;
}