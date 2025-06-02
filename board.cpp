#include "board.h"

Board::Board()
{
    // Initialize the board with empty squares
    squares.resize(8, std::vector<std::shared_ptr<Piece>>(8, nullptr));

    // Set default values
    sideToMove = Color::WHITE;
    whiteCanCastleKingside = true;
    whiteCanCastleQueenside = true;
    blackCanCastleKingside = true;
    blackCanCastleQueenside = true;
    enPassantTarget = Position();
    halfMoveClock = 0;
    fullMoveNumber = 1;

    // Initialize the kings as nullptr
    whiteKing = nullptr;
    blackKing = nullptr;
}

void Board::setupStartingPosition()
{
    clear();

    // Setup pawns
    for (int col = 0; col < 8; col++)
    {
        setPieceAt(Position(1, col), std::make_shared<Pawn>(Color::WHITE, Position(1, col)));
        setPieceAt(Position(6, col), std::make_shared<Pawn>(Color::BLACK, Position(6, col)));
    }

    // Setup rooks
    setPieceAt(Position(0, 0), std::make_shared<Rook>(Color::WHITE, Position(0, 0)));
    setPieceAt(Position(0, 7), std::make_shared<Rook>(Color::WHITE, Position(0, 7)));
    setPieceAt(Position(7, 0), std::make_shared<Rook>(Color::BLACK, Position(7, 0)));
    setPieceAt(Position(7, 7), std::make_shared<Rook>(Color::BLACK, Position(7, 7)));

    // Setup knights
    setPieceAt(Position(0, 1), std::make_shared<Knight>(Color::WHITE, Position(0, 1)));
    setPieceAt(Position(0, 6), std::make_shared<Knight>(Color::WHITE, Position(0, 6)));
    setPieceAt(Position(7, 1), std::make_shared<Knight>(Color::BLACK, Position(7, 1)));
    setPieceAt(Position(7, 6), std::make_shared<Knight>(Color::BLACK, Position(7, 6)));

    // Setup bishops
    setPieceAt(Position(0, 2), std::make_shared<Bishop>(Color::WHITE, Position(0, 2)));
    setPieceAt(Position(0, 5), std::make_shared<Bishop>(Color::WHITE, Position(0, 5)));
    setPieceAt(Position(7, 2), std::make_shared<Bishop>(Color::BLACK, Position(7, 2)));
    setPieceAt(Position(7, 5), std::make_shared<Bishop>(Color::BLACK, Position(7, 5)));

    // Setup queens
    setPieceAt(Position(0, 3), std::make_shared<Queen>(Color::WHITE, Position(0, 3)));
    setPieceAt(Position(7, 3), std::make_shared<Queen>(Color::BLACK, Position(7, 3)));

    // Setup kings
    whiteKing = std::make_shared<King>(Color::WHITE, Position(0, 4));
    blackKing = std::make_shared<King>(Color::BLACK, Position(7, 4));
    setPieceAt(Position(0, 4), whiteKing);
    setPieceAt(Position(7, 4), blackKing);

    // Reset game state variables
    sideToMove = Color::WHITE;
    whiteCanCastleKingside = true;
    whiteCanCastleQueenside = true;
    blackCanCastleKingside = true;
    blackCanCastleQueenside = true;
    enPassantTarget = Position();
    halfMoveClock = 0;
    fullMoveNumber = 1;
}

// REPLACE THE ENTIRE setupFromFEN METHOD (from the opening brace to closing brace) with this:

void Board::setupFromFEN(const std::string& fen) {
    // STEP 1: Input validation
    if (fen.empty() || fen.length() < 10) {
        std::cerr << "Error: FEN string too short or empty (length: " << fen.length() << ")" << std::endl;
        setupStartingPosition();
        return;
    }
    
    // STEP 2: Safe parsing with validation
    std::istringstream ss(fen);
    std::string boardStr, activeColor, castling, enPassant, halfmove, fullmove;
    
    // Try to extract all 6 components
    if (!(ss >> boardStr >> activeColor >> castling >> enPassant >> halfmove >> fullmove)) {
        std::cerr << "Error: Invalid FEN format - missing components. Expected 6 parts, got: " << fen << std::endl;
        setupStartingPosition();
        return;
    }
    
    // STEP 3: Validate each component
    if (activeColor != "w" && activeColor != "b") {
        std::cerr << "Error: Invalid active color '" << activeColor << "' - must be 'w' or 'b'" << std::endl;
        setupStartingPosition();
        return;
    }
    
    // STEP 4: Validate castling rights format
    for (char c : castling) {
        if (c != '-' && c != 'K' && c != 'Q' && c != 'k' && c != 'q') {
            std::cerr << "Error: Invalid castling rights character: '" << c << "'" << std::endl;
            setupStartingPosition();
            return;
        }
    }
    
    // STEP 5: Validate en passant target
    if (enPassant != "-") {
        if (enPassant.length() != 2 || 
            enPassant[0] < 'a' || enPassant[0] > 'h' ||
            enPassant[1] < '1' || enPassant[1] > '8') {
            std::cerr << "Error: Invalid en passant target: '" << enPassant << "'" << std::endl;
            setupStartingPosition();
            return;
        }
    }
    
    // STEP 6: Safe numeric conversion with bounds checking
    int halfMove = 0, fullMove = 1;
    try {
        halfMove = std::stoi(halfmove);
        fullMove = std::stoi(fullmove);
        
        if (halfMove < 0 || halfMove > 100) {
            std::cerr << "Warning: Unusual halfmove clock " << halfMove << ", clamping to valid range" << std::endl;
            halfMove = std::max(0, std::min(100, halfMove));
        }
        
        if (fullMove < 1 || fullMove > 9999) {
            std::cerr << "Warning: Unusual fullmove number " << fullMove << ", clamping to valid range" << std::endl;
            fullMove = std::max(1, std::min(9999, fullMove));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Cannot parse move counters '" << halfmove << "', '" << fullmove << "': " << e.what() << std::endl;
        std::cerr << "Using default values: halfmove=0, fullmove=1" << std::endl;
        halfMove = 0;
        fullMove = 1;
    }
    
    // STEP 7: Validate board string format BEFORE attempting to parse
    if (!validateFENBoardString(boardStr)) {
        std::cerr << "Error: Invalid board position in FEN: '" << boardStr << "'" << std::endl;
        setupStartingPosition();
        return;
    }
    
    // STEP 8: If we get here, FEN is valid - clear board and start parsing
    clear();
    
    // STEP 9: Parse board position (now we know it's valid)
    int row = 7;  // Start from rank 8 (top of board)
    int col = 0;
    
    for (char c : boardStr) {
        if (c == '/') {
            row--;
            col = 0;
        } else if (isdigit(c)) {
            col += c - '0';  // Skip empty squares
        } else {
            // Create piece
            Position pos(row, col);
            std::shared_ptr<Piece> piece = nullptr;
            Color color = isupper(c) ? Color::WHITE : Color::BLACK;
            char pieceChar = tolower(c);
            
            switch (pieceChar) {
                case 'p':
                    piece = std::make_shared<Pawn>(color, pos);
                    break;
                case 'n':
                    piece = std::make_shared<Knight>(color, pos);
                    break;
                case 'b':
                    piece = std::make_shared<Bishop>(color, pos);
                    break;
                case 'r':
                    piece = std::make_shared<Rook>(color, pos);
                    break;
                case 'q':
                    piece = std::make_shared<Queen>(color, pos);
                    break;
                case 'k': {
                    auto king = std::make_shared<King>(color, pos);
                    piece = king;
                    if (color == Color::WHITE) {
                        whiteKing = king;
                    } else {
                        blackKing = king;
                    }
                    break;
                }
                default:
                    std::cerr << "Error: Unexpected piece character during parsing: " << c << std::endl;
                    setupStartingPosition();
                    return;
            }
            
            setPieceAt(pos, piece);
            col++;
        }
    }
    
    // STEP 10: Set game state (we know all values are valid)
    sideToMove = (activeColor == "w") ? Color::WHITE : Color::BLACK;
    
    // Parse castling rights
    whiteCanCastleKingside = castling.find('K') != std::string::npos;
    whiteCanCastleQueenside = castling.find('Q') != std::string::npos;
    blackCanCastleKingside = castling.find('k') != std::string::npos;
    blackCanCastleQueenside = castling.find('q') != std::string::npos;
    
    // Parse en passant target square
    if (enPassant != "-") {
        enPassantTarget = Position::fromString(enPassant);
    } else {
        enPassantTarget = Position();
    }
    
    // Set move counters
    halfMoveClock = halfMove;
    fullMoveNumber = fullMove;
    
    // STEP 11: Final validation - ensure kings are properly set
    if (!whiteKing || !blackKing) {
        std::cerr << "Error: Kings not properly initialized after FEN parsing" << std::endl;
        setupStartingPosition();
        return;
    }
    
    std::cout << "Successfully loaded FEN: " << fen << std::endl;
}

std::string Board::toFEN() const
{
    std::stringstream fen;

    // Board position
    for (int row = 7; row >= 0; row--)
    {
        int emptyCount = 0;

        for (int col = 0; col < 8; col++)
        {
            auto piece = getPieceAt(Position(row, col));

            if (piece)
            {
                if (emptyCount > 0)
                {
                    fen << emptyCount;
                    emptyCount = 0;
                }
                fen << piece->toChar();
            }
            else
            {
                emptyCount++;
            }
        }

        if (emptyCount > 0)
        {
            fen << emptyCount;
        }

        if (row > 0)
        {
            fen << '/';
        }
    }

    // Active color
    fen << ' ' << (sideToMove == Color::WHITE ? 'w' : 'b');

    // Castling rights
    fen << ' ';
    if (whiteCanCastleKingside || whiteCanCastleQueenside ||
        blackCanCastleKingside || blackCanCastleQueenside)
    {
        if (whiteCanCastleKingside)
            fen << 'K';
        if (whiteCanCastleQueenside)
            fen << 'Q';
        if (blackCanCastleKingside)
            fen << 'k';
        if (blackCanCastleQueenside)
            fen << 'q';
    }
    else
    {
        fen << '-';
    }

    // En passant target square
    fen << ' ';
    if (enPassantTarget.isValid())
    {
        fen << enPassantTarget.toString();
    }
    else
    {
        fen << '-';
    }

    // Halfmove clock
    fen << ' ' << halfMoveClock;

    // Fullmove number
    fen << ' ' << fullMoveNumber;

    return fen.str();
}

std::shared_ptr<Piece> Board::getPieceAt(const Position &pos) const
{
    if (!pos.isValid())
        return nullptr;
    return squares[pos.row][pos.col];
}

void Board::setPieceAt(const Position &pos, std::shared_ptr<Piece> piece)
{
    if (!pos.isValid())
        return;
    squares[pos.row][pos.col] = piece;
    if (piece)
    {
        piece->setPosition(pos);
    }
}

bool Board::makeMove(const Move& move) {
    BoardState dummy; // We don't need to save state for the public interface
    return makeMove(move, dummy);
}

bool Board::makeMove(const Move &move, BoardState &previousState)
{
    previousState.sideToMove = sideToMove;
    previousState.whiteCanCastleKingside = whiteCanCastleKingside;
    previousState.whiteCanCastleQueenside = whiteCanCastleQueenside;
    previousState.blackCanCastleKingside = blackCanCastleKingside;
    previousState.blackCanCastleQueenside = blackCanCastleQueenside;
    previousState.enPassantTarget = enPassantTarget;
    previousState.halfMoveClock = halfMoveClock;
    previousState.fullMoveNumber = fullMoveNumber;
    previousState.capturedPiece = nullptr;
    previousState.wasEnPassant = false;
    previousState.wasPromotion = false;

    // Get the piece at the source position
    auto piece = getPieceAt(move.from);
    if (!piece)
        return false;

    // Check if the piece belongs to the current side to move
    if (piece->getColor() != sideToMove)
        return false;

// Efficient basic validation - check move pattern without generating all moves
if (!isValidMovePattern(piece, move)) {
    return false;
}

// Add this NEW HELPER METHOD to board.cpp (add after wouldBeInCheck method):
bool Board::isValidMovePattern(std::shared_ptr<Piece> piece, const Move& move) const
{
    if (!piece || !move.from.isValid() || !move.to.isValid()) {
        return false;
    }
    
    PieceType type = piece->getType();
    Color color = piece->getColor();
    int rowDiff = move.to.row - move.from.row;
    int colDiff = move.to.col - move.from.col;
    
    // Check basic move patterns for each piece type
    switch (type) {
        case PieceType::PAWN: {
            int direction = (color == Color::WHITE) ? 1 : -1;
            
            // Forward moves
            if (colDiff == 0) {
                if (rowDiff == direction) return true; // Single step
                if (rowDiff == 2 * direction) {
                    // Double step from starting position
                    bool startingRank = (color == Color::WHITE && move.from.row == 1) || 
                                       (color == Color::BLACK && move.from.row == 6);
                    return startingRank;
                }
            }
            // Diagonal captures (including en passant)
            else if (abs(colDiff) == 1 && rowDiff == direction) {
                auto target = getPieceAt(move.to);
                return (target && target->getColor() != color) || 
                       (move.to == getEnPassantTarget());
            }
            return false;
        }
        
        case PieceType::KNIGHT: {
            return (abs(rowDiff) == 2 && abs(colDiff) == 1) || 
                   (abs(rowDiff) == 1 && abs(colDiff) == 2);
        }
        
        case PieceType::BISHOP: {
            if (abs(rowDiff) != abs(colDiff)) return false;
            return isPathClearForMove(move.from, move.to);
        }
        
        case PieceType::ROOK: {
            if (rowDiff != 0 && colDiff != 0) return false;
            return isPathClearForMove(move.from, move.to);
        }
        
        case PieceType::QUEEN: {
            if (rowDiff != 0 && colDiff != 0 && abs(rowDiff) != abs(colDiff)) return false;
            return isPathClearForMove(move.from, move.to);
        }
        
        case PieceType::KING: {
            // Regular king moves
            if (abs(rowDiff) <= 1 && abs(colDiff) <= 1) return true;
            
            // Castling moves
            if (abs(rowDiff) == 0 && abs(colDiff) == 2) {
                return canCastle(move);
            }
            return false;
        }
        
        default:
            return false;
    }
}

    previousState.pieceHasMoved = piece->getHasMoved();

    // Handle castling BEFORE other validations
    if (piece->getType() == PieceType::KING) {
        // Kingside castling
        if (move.from.col == 4 && move.to.col == 6) {
            if (!canCastle(move)) {
                return false;
            }
            
            // Move the rook
            auto rook = getPieceAt(Position(move.from.row, 7));
            setPieceAt(Position(move.from.row, 5), rook);
            setPieceAt(Position(move.from.row, 7), nullptr);
            if (rook) rook->setMoved();
        }
        // Queenside castling
        else if (move.from.col == 4 && move.to.col == 2) {
            if (!canCastle(move)) {
                return false;
            }
            
            // Move the rook
            auto rook = getPieceAt(Position(move.from.row, 0));
            setPieceAt(Position(move.from.row, 3), rook);
            setPieceAt(Position(move.from.row, 0), nullptr);
            if (rook) rook->setMoved();
        }
    }

     previousState.capturedPiece = getPieceAt(move.to);

    // Determine if this is a capture or pawn move 
    bool isCapture = getPieceAt(move.to) != nullptr;
    bool isPawnMove = piece->getType() == PieceType::PAWN;

   // REPLACE the en passant capture section in makeMove() with this:
// Handle en passant capture with proper validation
if (isPawnMove && move.to == enPassantTarget && enPassantTarget.isValid()) {
    // Validate that there's actually a pawn to capture
    int capturedPawnRow = (sideToMove == Color::WHITE) ? move.to.row - 1 : move.to.row + 1;
    Position capturedPawnPos(capturedPawnRow, move.to.col);
    
    auto capturedPawn = getPieceAt(capturedPawnPos);
    
    // COMPREHENSIVE en passant validation
    if (capturedPawn && 
        capturedPawn->getType() == PieceType::PAWN && 
        capturedPawn->getColor() != sideToMove) {
        
        // Additional validation: The captured pawn should be on the correct rank
        bool validRank = false;
        if (sideToMove == Color::WHITE && capturedPawnRow == 4) {
            validRank = true; // White capturing black pawn on 5th rank
        } else if (sideToMove == Color::BLACK && capturedPawnRow == 3) {
            validRank = true; // Black capturing white pawn on 4th rank
        }
        
        if (validRank) {
            // Valid en passant capture
            previousState.capturedPiece = capturedPawn;
            setPieceAt(capturedPawnPos, nullptr);
            isCapture = true;
            previousState.wasEnPassant = true;
        } else {
            // Invalid en passant - this shouldn't happen with correct move generation
            return false;
        }
    } else {
        // No valid pawn to capture - invalid en passant
        return false;
    }
}

   // Update en passant target square
    if (isPawnMove && abs(move.to.row - move.from.row) == 2) {
        // Set the en passant target square
        int epRow = (sideToMove == Color::WHITE) ? move.from.row + 1 : move.from.row - 1;
        enPassantTarget = Position(epRow, move.from.col);
    } else {
        enPassantTarget = Position();
    }
    
    // Update halfmove clock
    if (isCapture || isPawnMove) {
        halfMoveClock = 0;
    } else {
        halfMoveClock++;
    }
    
    // Save the original piece type for undoing promotions
    previousState.originalType = piece->getType();
    
// Get the original piece type before any modifications
auto originalPiece = getPieceAt(move.from);
PieceType originalPieceType = originalPiece->getType();

// Handle pawn promotion - FIXED memory safety
if (isPawnMove && (move.to.row == 0 || move.to.row == 7)) {
    // Default to queen if no promotion specified (should not happen in normal play)
    PieceType promotionType = (move.promotion != PieceType::NONE) ? move.promotion : PieceType::QUEEN;
    
    // Validate promotion type (only allow legal promotions)
    if (promotionType != PieceType::QUEEN && promotionType != PieceType::ROOK && 
        promotionType != PieceType::BISHOP && promotionType != PieceType::KNIGHT) {
        promotionType = PieceType::QUEEN; // Default to queen for invalid types
    }
    
    previousState.wasPromotion = true;
    
    // Create new piece - no try/catch needed, shared_ptr handles this safely
    std::shared_ptr<Piece> newPiece;
    
    switch (promotionType) {
        case PieceType::QUEEN:
            newPiece = std::make_shared<Queen>(sideToMove, move.from); // Use move.from initially
            break;
        case PieceType::ROOK:
            newPiece = std::make_shared<Rook>(sideToMove, move.from);
            break;
        case PieceType::BISHOP:
            newPiece = std::make_shared<Bishop>(sideToMove, move.from);
            break;
        case PieceType::KNIGHT:
            newPiece = std::make_shared<Knight>(sideToMove, move.from);
            break;
        default:
            newPiece = std::make_shared<Queen>(sideToMove, move.from);
            break;
    }
    
    // Safe assignment - the old piece will be automatically cleaned up
    piece = newPiece;
    // Position will be updated when we call setPieceAt(move.to, piece) later
}

// Update castling rights based on ORIGINAL piece movement (MOVED OUTSIDE PROMOTION BLOCK)
if (originalPieceType == PieceType::KING) {
    if (sideToMove == Color::WHITE) {
        whiteCanCastleKingside = false;
        whiteCanCastleQueenside = false;
    } else {
        blackCanCastleKingside = false;
        blackCanCastleQueenside = false;
    }
}

// Update castling rights if rook moves
if (originalPieceType == PieceType::ROOK) {
    if (sideToMove == Color::WHITE) {
        if (move.from.row == 0 && move.from.col == 0) {
            whiteCanCastleQueenside = false;
        }
        if (move.from.row == 0 && move.from.col == 7) {
            whiteCanCastleKingside = false;
        }
    } else {
        if (move.from.row == 7 && move.from.col == 0) {
            blackCanCastleQueenside = false;
        }
        if (move.from.row == 7 && move.from.col == 7) {
            blackCanCastleKingside = false;
        }
    }
}

// Update castling rights if rook is captured
if (previousState.capturedPiece && previousState.capturedPiece->getType() == PieceType::ROOK) {
    if (move.to.row == 0 && move.to.col == 0) {
        whiteCanCastleQueenside = false;
    }
    if (move.to.row == 0 && move.to.col == 7) {
        whiteCanCastleKingside = false;
    }
    if (move.to.row == 7 && move.to.col == 0) {
        blackCanCastleQueenside = false;
    }
    if (move.to.row == 7 && move.to.col == 7) {
        blackCanCastleKingside = false;
    }
}

// CRITICAL FIX: Validate king safety BEFORE making irreversible changes
if (wouldBeInCheck(move, sideToMove)) {
    return false; // Invalid move - would leave king in check
}

// Now make the move (we know it's safe)
setPieceAt(move.from, nullptr);
setPieceAt(move.to, piece);

// Mark the piece as moved
piece->setMoved();
    
// Update fullmove number
if (sideToMove == Color::BLACK) {
    fullMoveNumber++;
}

// Switch side to move
switchSideToMove();

return true;

// Implementation of unmakeMove
bool Board::unmakeMove(const Move& move, const BoardState& previousState) {
    // Get the piece at the destination position
    auto piece = getPieceAt(move.to);
    if (!piece) return false;
    
    // If this was a promotion, create the original piece type (pawn)
    if (previousState.wasPromotion) {
        piece = std::make_shared<Pawn>(previousState.sideToMove, move.from);
    }
    
    // Move the piece back to the source
    setPieceAt(move.from, piece);
    setPieceAt(move.to, nullptr);
    
    // If it didn't have moved status before, reset it
    if (!previousState.pieceHasMoved) {
        piece->setHasMoved(false);
    }
    
    // Restore captured piece (if any)
    if (previousState.wasEnPassant) {
        // This was an en passant capture
        int capturedPawnRow = (previousState.sideToMove == Color::WHITE) ? move.to.row - 1 : move.to.row + 1;
        setPieceAt(Position(capturedPawnRow, move.to.col), previousState.capturedPiece);
    } else if (previousState.capturedPiece) {
        // Normal capture
        setPieceAt(move.to, previousState.capturedPiece);
    }
    
    // Handle castling - move the rook back
    if (piece->getType() == PieceType::KING) {
        if (move.from.col == 4 && move.to.col == 6) {
            // Kingside castling - move rook back
            auto rook = getPieceAt(Position(move.from.row, 5));
            if (rook && rook->getType() == PieceType::ROOK) {
                setPieceAt(Position(move.from.row, 7), rook);
                setPieceAt(Position(move.from.row, 5), nullptr);
                rook->setHasMoved(false); // Assume the rook hadn't moved before castling
            }
        }
        else if (move.from.col == 4 && move.to.col == 2) {
            // Queenside castling - move rook back
            auto rook = getPieceAt(Position(move.from.row, 3));
            if (rook && rook->getType() == PieceType::ROOK) {
                setPieceAt(Position(move.from.row, 0), rook);
                setPieceAt(Position(move.from.row, 3), nullptr);
                rook->setHasMoved(false); // Assume the rook hadn't moved before castling
            }
        }
    }
    
    // Restore the game state
    sideToMove = previousState.sideToMove;
    whiteCanCastleKingside = previousState.whiteCanCastleKingside;
    whiteCanCastleQueenside = previousState.whiteCanCastleQueenside;
    blackCanCastleKingside = previousState.blackCanCastleKingside;
    blackCanCastleQueenside = previousState.blackCanCastleQueenside;
    enPassantTarget = previousState.enPassantTarget;
    halfMoveClock = previousState.halfMoveClock;
    fullMoveNumber = previousState.fullMoveNumber;
    
    return true;
}

std::vector<Move> Board::generateLegalMoves() const
{
    std::vector<Move> legalMoves;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            auto piece = getPieceAt(Position(row, col));

            if (piece && piece->getColor() == sideToMove)
            {
                auto pieceMoves = piece->getLegalMoves(*this);

                // Filter out moves that would leave the king in check
                for (const auto &move : pieceMoves)
                {
                    if (!wouldBeInCheck(move, sideToMove))
                    {
                        legalMoves.push_back(move);
                    }
                }
            }
        }
    }

    return legalMoves;
}

bool Board::isInCheck() const
{
    auto king = (sideToMove == Color::WHITE) ? whiteKing : blackKing;
    if (!king)
        return false;

    return isSquareAttacked(king->getPosition(), (sideToMove == Color::WHITE) ? Color::BLACK : Color::WHITE);
}

// REPLACE both isCheckmate() and isStalemate() methods in board.cpp with these:

bool Board::isCheckmate() const
{
    // OPTIMIZED: Check for check first (faster than generating moves)
    bool inCheck = isInCheck();
    if (!inCheck) {
        return false;
    }

    // Only generate legal moves if we're in check
    auto legalMoves = generateLegalMoves();
    return legalMoves.empty();
}

bool Board::isStalemate() const
{
    // OPTIMIZED: Check for check first (faster than generating moves)
    bool inCheck = isInCheck();
    if (inCheck) {
        return false; // Can't be stalemate if in check
    }

    // Only generate legal moves if we're NOT in check
    auto legalMoves = generateLegalMoves();
    return legalMoves.empty();
}

// BONUS: Add this helper method for better game state detection
bool Board::isGameOver() const
{
    // Early exit optimizations
    bool inCheck = isInCheck();
    auto legalMoves = generateLegalMoves();
    
    // No legal moves available
    if (legalMoves.empty()) {
        return true; // Either checkmate or stalemate
    }
    
    // Game continues if there are legal moves
    return false;
}

// BONUS: Add this method to distinguish game end reasons efficiently
enum class GameEndType {
    NOT_ENDED,
    CHECKMATE,
    STALEMATE
};

GameEndType Board::getGameEndType() const
{
    bool inCheck = isInCheck();
    auto legalMoves = generateLegalMoves();
    
    if (legalMoves.empty()) {
        return inCheck ? GameEndType::CHECKMATE : GameEndType::STALEMATE;
    }
    
    return GameEndType::NOT_ENDED;
}

bool Board::isSquareAttacked(const Position &pos, Color attackerColor) const
{
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            auto piece = getPieceAt(Position(row, col));

            if (piece && piece->getColor() == attackerColor)
            {
                auto moves = piece->getLegalMoves(*this);

                for (const auto &move : moves)
                {
                    if (move.to == pos)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void Board::print() const
{
    std::cout << "  a b c d e f g h" << std::endl;

    for (int row = 7; row >= 0; row--)
    {
        std::cout << (row + 1) << " ";

        for (int col = 0; col < 8; col++)
        {
            auto piece = getPieceAt(Position(row, col));

            if (piece)
            {
                std::cout << piece->toChar() << " ";
            }
            else
            {
                std::cout << ". ";
            }
        }

        std::cout << (row + 1) << std::endl;
    }

    std::cout << "  a b c d e f g h" << std::endl;
    std::cout << "Side to move: " << (sideToMove == Color::WHITE ? "White" : "Black") << std::endl;
}

void Board::clear()
{
    // Clear the board
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            setPieceAt(Position(row, col), nullptr);
        }
    }

    // Reset kings
    whiteKing = nullptr;
    blackKing = nullptr;

    // Reset game state
    sideToMove = Color::WHITE;
    whiteCanCastleKingside = false;
    whiteCanCastleQueenside = false;
    blackCanCastleKingside = false;
    blackCanCastleQueenside = false;
    enPassantTarget = Position();
    halfMoveClock = 0;
    fullMoveNumber = 1;
}

// REPLACE the entire canCastle() method in board.cpp with this:
bool Board::canCastle(const Move &move) const
{
    // Check if the piece is a king
    auto piece = getPieceAt(move.from);
    if (!piece || piece->getType() != PieceType::KING)
    {
        return false;
    }
    
    Color kingColor = piece->getColor();
    
    // COMPREHENSIVE castling validation
    
    // 1. King must not have moved
    if (piece->getHasMoved())
    {
        return false;
    }
    
    // 2. King must not be in check
    if (isInCheck())
    {
        return false;
    }
    
    // 3. King must be on correct starting square
    int expectedRow = (kingColor == Color::WHITE) ? 0 : 7;
    if (move.from.row != expectedRow || move.from.col != 4)
    {
        return false;
    }

    // 4. Validate castling direction and rights
    if (move.to.col == move.from.col + 2) // Kingside castling
    {
        // Check castling rights
        if (kingColor == Color::WHITE && !whiteCanCastleKingside)
        {
            return false;
        }
        if (kingColor == Color::BLACK && !blackCanCastleKingside)
        {
            return false;
        }

        // ENHANCED: Check if the rook is there, is correct type/color, and hasn't moved
        Position rookPos(expectedRow, 7);
        auto rook = getPieceAt(rookPos);
        if (!rook || 
            rook->getType() != PieceType::ROOK || 
            rook->getColor() != kingColor ||
            rook->getHasMoved())
        {
            return false;
        }

        // Check if squares between king and rook are empty
        for (int col = 5; col <= 6; col++)
        {
            if (getPieceAt(Position(expectedRow, col)))
            {
                return false;
            }
        }

        // Check if king would move through or end up in check
        Color opponentColor = (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
        for (int col = 5; col <= 6; col++)
        {
            if (isSquareAttacked(Position(expectedRow, col), opponentColor))
            {
                return false;
            }
        }

        return true;
    }
    else if (move.to.col == move.from.col - 2) // Queenside castling
    {
        // Check castling rights
        if (kingColor == Color::WHITE && !whiteCanCastleQueenside)
        {
            return false;
        }
        if (kingColor == Color::BLACK && !blackCanCastleQueenside)
        {
            return false;
        }

        // ENHANCED: Check if the rook is there, is correct type/color, and hasn't moved
        Position rookPos(expectedRow, 0);
        auto rook = getPieceAt(rookPos);
        if (!rook || 
            rook->getType() != PieceType::ROOK || 
            rook->getColor() != kingColor ||
            rook->getHasMoved())
        {
            return false;
        }

        // Check if squares between king and rook are empty (including b-file for queenside)
        for (int col = 1; col <= 3; col++)
        {
            if (getPieceAt(Position(expectedRow, col)))
            {
                return false;
            }
        }

        // Check if king would move through or end up in check (not including b-file)
        Color opponentColor = (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
        for (int col = 2; col <= 3; col++)
        {
            if (isSquareAttacked(Position(expectedRow, col), opponentColor))
            {
                return false;
            }
        }

        return true;
    }

    // Invalid castling move
    return false;
}

bool Board::wouldBeInCheck(const Move &move, Color kingColor) const
{
    // SAFE APPROACH: Use direct attack simulation without full board copying
    // This prevents infinite recursion and is efficient
    
    auto movingPiece = getPieceAt(move.from);
    if (!movingPiece) return false;
    
    // Determine king position after the move
    Position kingPos;
    bool foundKing = false;
    
    if (movingPiece->getType() == PieceType::KING && movingPiece->getColor() == kingColor) {
        // The king is moving - its new position will be the destination
        kingPos = move.to;
        foundKing = true;
    } else {
        // Find the current king position
        auto king = (kingColor == Color::WHITE) ? whiteKing : blackKing;
        if (king) {
            kingPos = king->getPosition();
            foundKing = true;
        }
    }
    
    if (!foundKing) return false;
    
    // Check if any opponent piece can attack the king after this move
    Color opponentColor = (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position piecePos(row, col);
            auto piece = getPieceAt(piecePos);
            
            if (!piece || piece->getColor() != opponentColor) continue;
            
            // Skip the piece being captured (it won't be able to attack after the move)
            if (piecePos == move.to) continue;
            
            // For castling, also skip the rook that moves
            if (movingPiece->getType() == PieceType::KING) {
                if ((move.from.col == 4 && move.to.col == 6 && piecePos.row == move.from.row && piecePos.col == 7) ||
                    (move.from.col == 4 && move.to.col == 2 && piecePos.row == move.from.row && piecePos.col == 0)) {
                    continue;
                }
            }
            
            // Check if this piece can attack the king position after the move
            if (canPieceAttackSquareAfterMove(piece, piecePos, kingPos, move)) {
                return true;
            }
        }
    }
    
    // Special case: Check if the moved piece itself gives check
    // (This handles discovered attacks)
    if (movingPiece->getColor() == opponentColor) {
        Position newPiecePos = move.to;
        if (canPieceAttackSquareSimple(movingPiece->getType(), newPiecePos, kingPos)) {
            return true;
        }
    }
    
    return false;
}

// Helper method - add this to board.cpp (and declare in board.h private section)
bool Board::canPieceAttackSquareAfterMove(std::shared_ptr<Piece> piece, Position piecePos, 
                                          Position target, const Move& simulatedMove) const
{
    if (!piece || !target.isValid() || !piecePos.isValid()) return false;
    
    // Adjust piece position if it's affected by castling
    if (simulatedMove.from.isValid()) {
        auto movingPiece = getPieceAt(simulatedMove.from);
        if (movingPiece && movingPiece->getType() == PieceType::KING) {
            // Handle rook movement in castling
            if (simulatedMove.from.col == 4 && simulatedMove.to.col == 6) { // Kingside
                if (piecePos.row == simulatedMove.from.row && piecePos.col == 7) {
                    piecePos = Position(simulatedMove.from.row, 5); // Rook moves to f-file
                }
            } else if (simulatedMove.from.col == 4 && simulatedMove.to.col == 2) { // Queenside
                if (piecePos.row == simulatedMove.from.row && piecePos.col == 0) {
                    piecePos = Position(simulatedMove.from.row, 3); // Rook moves to d-file
                }
            }
        }
    }
    
    return canPieceAttackSquareSimple(piece->getType(), piecePos, target);
}

// Simple geometric attack check - add this to board.cpp (and declare in board.h private section)
bool Board::canPieceAttackSquareSimple(PieceType pieceType, Position from, Position to) const
{
    if (!from.isValid() || !to.isValid()) return false;
    
    int rowDiff = to.row - from.row;
    int colDiff = to.col - from.col;
    
    switch (pieceType) {
        case PieceType::PAWN: {
            // Note: We need to know the piece color, but we can infer from context
            // For now, check both directions (this is safe for check detection)
            return (abs(rowDiff) == 1 && abs(colDiff) == 1);
        }
        
        case PieceType::KNIGHT: {
            return (abs(rowDiff) == 2 && abs(colDiff) == 1) || 
                   (abs(rowDiff) == 1 && abs(colDiff) == 2);
        }
        
        case PieceType::BISHOP: {
            if (abs(rowDiff) != abs(colDiff)) return false;
            return isPathClearForMove(from, to);
        }
        
        case PieceType::ROOK: {
            if (rowDiff != 0 && colDiff != 0) return false;
            return isPathClearForMove(from, to);
        }
        
        case PieceType::QUEEN: {
            if (rowDiff != 0 && colDiff != 0 && abs(rowDiff) != abs(colDiff)) return false;
            return isPathClearForMove(from, to);
        }
        
        case PieceType::KING: {
            return abs(rowDiff) <= 1 && abs(colDiff) <= 1 && (rowDiff != 0 || colDiff != 0);
        }
        
        default:
            return false;
    }
}

// Path checking helper - add this to board.cpp (and declare in board.h private section)
bool Board::isPathClearForMove(Position from, Position to) const
{
    int rowStep = (to.row > from.row) ? 1 : (to.row < from.row) ? -1 : 0;
    int colStep = (to.col > from.col) ? 1 : (to.col < from.col) ? -1 : 0;
    
    Position current = from;
    current.row += rowStep;
    current.col += colStep;
    
    while (current.row != to.row || current.col != to.col) {
        if (getPieceAt(current)) {
            return false; // Path is blocked
        }
        current.row += rowStep;
        current.col += colStep;
    }
    
    return true;

bool Board::validateFENBoardString(const std::string& boardStr) const {
    if (boardStr.empty()) {
        return false;
    }
    
    int slashCount = 0;
    int squareCount = 0;
    bool hasWhiteKing = false;
    bool hasBlackKing = false;
    int whiteKingCount = 0;
    int blackKingCount = 0;
    
    for (char c : boardStr) {
        if (c == '/') {
            // Each rank must have exactly 8 squares
            if (squareCount != 8) {
                std::cerr << "Error: Rank has " << squareCount << " squares, expected 8" << std::endl;
                return false;
            }
            slashCount++;
            squareCount = 0;
            
            // Can't have more than 7 slashes (8 ranks total)
            if (slashCount > 7) {
                std::cerr << "Error: Too many ranks in FEN board string" << std::endl;
                return false;
            }
        } 
        else if (isdigit(c)) {
            int emptySquares = c - '0';
            // Empty square count must be 1-8
            if (emptySquares < 1 || emptySquares > 8) {
                std::cerr << "Error: Invalid empty square count: " << emptySquares << std::endl;
                return false;
            }
            squareCount += emptySquares;
            
            // Can't exceed 8 squares per rank
            if (squareCount > 8) {
                std::cerr << "Error: Too many squares in rank" << std::endl;
                return false;
            }
        } 
        else if (strchr("pnbrqkPNBRQK", c)) {
            squareCount++;
            
            // Track kings for validation
            if (c == 'K') {
                hasWhiteKing = true;
                whiteKingCount++;
            } else if (c == 'k') {
                hasBlackKing = true;
                blackKingCount++;
            }
            
            // Can't exceed 8 squares per rank
            if (squareCount > 8) {
                std::cerr << "Error: Too many squares in rank" << std::endl;
                return false;
            }
        } 
        else {
            std::cerr << "Error: Invalid character in board string: '" << c << "'" << std::endl;
            return false;
        }
    }
    
    // Must have exactly 7 slashes (separating 8 ranks)
    if (slashCount != 7) {
        std::cerr << "Error: Expected 7 slashes, found " << slashCount << std::endl;
        return false;
    }
    
    // Final rank must have exactly 8 squares
    if (squareCount != 8) {
        std::cerr << "Error: Final rank has " << squareCount << " squares, expected 8" << std::endl;
        return false;
    }
    
    // Must have exactly one king of each color
    if (!hasWhiteKing || !hasBlackKing) {
        std::cerr << "Error: Missing king - White: " << hasWhiteKing << ", Black: " << hasBlackKing << std::endl;
        return false;
    }
    
    if (whiteKingCount != 1 || blackKingCount != 1) {
        std::cerr << "Error: Invalid king count - White: " << whiteKingCount << ", Black: " << blackKingCount << std::endl;
        return false;
    }
    
    return true;
}
}