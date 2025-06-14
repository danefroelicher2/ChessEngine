#include "board.h"
#include "piece_types.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <climits>

Board::Board() : squares(8, std::vector<std::shared_ptr<Piece>>(8, nullptr))
{
    setupStartingPosition();
}

void Board::setupStartingPosition()
{
    // Clear the board
    clear();

    // Set up pawns
    for (int col = 0; col < 8; col++)
    {
        setPieceAt(Position(1, col), std::make_shared<Pawn>(Color::WHITE, Position(1, col)));
        setPieceAt(Position(6, col), std::make_shared<Pawn>(Color::BLACK, Position(6, col)));
    }

    // Set up rooks
    setPieceAt(Position(0, 0), std::make_shared<Rook>(Color::WHITE, Position(0, 0)));
    setPieceAt(Position(0, 7), std::make_shared<Rook>(Color::WHITE, Position(0, 7)));
    setPieceAt(Position(7, 0), std::make_shared<Rook>(Color::BLACK, Position(7, 0)));
    setPieceAt(Position(7, 7), std::make_shared<Rook>(Color::BLACK, Position(7, 7)));

    // Set up knights
    setPieceAt(Position(0, 1), std::make_shared<Knight>(Color::WHITE, Position(0, 1)));
    setPieceAt(Position(0, 6), std::make_shared<Knight>(Color::WHITE, Position(0, 6)));
    setPieceAt(Position(7, 1), std::make_shared<Knight>(Color::BLACK, Position(7, 1)));
    setPieceAt(Position(7, 6), std::make_shared<Knight>(Color::BLACK, Position(7, 6)));

    // Set up bishops
    setPieceAt(Position(0, 2), std::make_shared<Bishop>(Color::WHITE, Position(0, 2)));
    setPieceAt(Position(0, 5), std::make_shared<Bishop>(Color::WHITE, Position(0, 5)));
    setPieceAt(Position(7, 2), std::make_shared<Bishop>(Color::BLACK, Position(7, 2)));
    setPieceAt(Position(7, 5), std::make_shared<Bishop>(Color::BLACK, Position(7, 5)));

    // Set up queens
    setPieceAt(Position(0, 3), std::make_shared<Queen>(Color::WHITE, Position(0, 3)));
    setPieceAt(Position(7, 3), std::make_shared<Queen>(Color::BLACK, Position(7, 3)));

    // Set up kings
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
    
    // Set castling rights
    whiteCanCastleKingside = castling.find('K') != std::string::npos;
    whiteCanCastleQueenside = castling.find('Q') != std::string::npos;
    blackCanCastleKingside = castling.find('k') != std::string::npos;
    blackCanCastleQueenside = castling.find('q') != std::string::npos;
    
    // Set en passant target
    if (enPassant != "-") {
        enPassantTarget = Position::fromString(enPassant);
    } else {
        enPassantTarget = Position();
    }
    
    // Set move counters
    halfMoveClock = halfMove;
    fullMoveNumber = fullMove;
}

bool Board::validateFENBoardString(const std::string& boardStr) const {
    int slashCount = 0;
    int squareCount = 0;
    bool hasWhiteKing = false;
    bool hasBlackKing = false;
    int whiteKingCount = 0;
    int blackKingCount = 0;
    
    for (char c : boardStr) {
        if (c == '/') {
            // Check that we had exactly 8 squares in the previous rank
            if (squareCount != 8) {
                std::cerr << "Error: Rank has " << squareCount << " squares, expected 8" << std::endl;
                return false;
            }
            slashCount++;
            squareCount = 0;
        } else if (isdigit(c)) {
            int emptySquares = c - '0';
            if (emptySquares < 1 || emptySquares > 8) {
                std::cerr << "Error: Invalid empty square count: " << emptySquares << std::endl;
                return false;
            }
            squareCount += emptySquares;
            if (squareCount > 8) {
                std::cerr << "Error: Too many squares in rank" << std::endl;
                return false;
            }
        } else if (c == 'p' || c == 'n' || c == 'b' || c == 'r' || c == 'q' || c == 'k' ||
                   c == 'P' || c == 'N' || c == 'B' || c == 'R' || c == 'Q' || c == 'K') {
            squareCount++;
            if (c == 'K') {
                hasWhiteKing = true;
                whiteKingCount++;
            } else if (c == 'k') {
                hasBlackKing = true;
                blackKingCount++;
            }
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

std::string Board::toFEN() const
{
    std::ostringstream fen;

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
    // STEP 1: Comprehensive input validation
    if (!move.from.isValid() || !move.to.isValid()) {
        std::cerr << "Error: Invalid move coordinates - from: " << move.from.toString() 
                  << ", to: " << move.to.toString() << std::endl;
        return false;
    }
    
    // STEP 2: Validate source piece exists
    auto piece = getPieceAt(move.from);
    if (!piece) {
        std::cerr << "Error: No piece at source square " << move.from.toString() << std::endl;
        return false;
    }
    
    // STEP 3: Validate piece ownership
    if (piece->getColor() != sideToMove) {
        std::cerr << "Error: Attempting to move opponent's piece at " << move.from.toString() 
                  << " (piece color: " << (piece->getColor() == Color::WHITE ? "White" : "Black")
                  << ", side to move: " << (sideToMove == Color::WHITE ? "White" : "Black") << ")" << std::endl;
        return false;
    }
    
    // STEP 4: Validate that target square is not occupied by own piece
    auto targetPiece = getPieceAt(move.to);
    if (targetPiece && targetPiece->getColor() == sideToMove) {
        std::cerr << "Error: Cannot capture own piece at " << move.to.toString() << std::endl;
        return false;
    }
    
    // STEP 5: Generate and validate legal moves (only if needed for complex validation)
    try {
        auto legalMoves = generateLegalMoves();
        bool moveIsLegal = false;
        
        // Check if the move is in the list of legal moves
        for (const auto& legalMove : legalMoves) {
            if (legalMove.from == move.from && legalMove.to == move.to && 
                legalMove.promotion == move.promotion) {
                moveIsLegal = true;
                break;
            }
        }
        
        if (!moveIsLegal) {
            std::cerr << "Error: Illegal move attempted: " << move.toString() << std::endl;
            std::cerr << "Legal moves from " << move.from.toString() << ": ";
            
            bool foundAnyFromSquare = false;
            for (const auto& legalMove : legalMoves) {
                if (legalMove.from == move.from) {
                    std::cerr << legalMove.toString() << " ";
                    foundAnyFromSquare = true;
                }
            }
            if (!foundAnyFromSquare) {
                std::cerr << "(none)";
            }
            std::cerr << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: Exception during move validation: " << e.what() << std::endl;
        return false;
    }
    
    // STEP 6: Save previous state before making any changes
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
        
        // Create the promoted piece
        std::shared_ptr<Piece> promotedPiece = nullptr;
        Color pieceColor = piece->getColor();
        
        switch (promotionType) {
            case PieceType::QUEEN:
                promotedPiece = std::make_shared<Queen>(pieceColor, move.to);
                break;
            case PieceType::ROOK:
                promotedPiece = std::make_shared<Rook>(pieceColor, move.to);
                break;
            case PieceType::BISHOP:
                promotedPiece = std::make_shared<Bishop>(pieceColor, move.to);
                break;
            case PieceType::KNIGHT:
                promotedPiece = std::make_shared<Knight>(pieceColor, move.to);
                break;
            default:
                promotedPiece = std::make_shared<Queen>(pieceColor, move.to);
                break;
        }
        
        // Set the promoted piece as the piece to move
        piece = promotedPiece;
    }

    // Update castling rights based on king or rook movement
    if (piece->getType() == PieceType::KING) {
        if (piece->getColor() == Color::WHITE) {
            whiteCanCastleKingside = false;
            whiteCanCastleQueenside = false;
        } else {
            blackCanCastleKingside = false;
            blackCanCastleQueenside = false;
        }
    }
    
    // Update castling rights if rook moves
    if (piece->getType() == PieceType::ROOK) {
        if (piece->getColor() == Color::WHITE) {
            if (move.from == Position(0, 0)) {
                whiteCanCastleQueenside = false;
            } else if (move.from == Position(0, 7)) {
                whiteCanCastleKingside = false;
            }
        } else {
            if (move.from == Position(7, 0)) {
                blackCanCastleQueenside = false;
            } else if (move.from == Position(7, 7)) {
                blackCanCastleKingside = false;
            }
        }
    }
    
    // Update castling rights if rook is captured
    if (targetPiece && targetPiece->getType() == PieceType::ROOK) {
        if (move.to == Position(0, 0)) {
            whiteCanCastleQueenside = false;
        } else if (move.to == Position(0, 7)) {
            whiteCanCastleKingside = false;
        } else if (move.to == Position(7, 0)) {
            blackCanCastleQueenside = false;
        } else if (move.to == Position(7, 7)) {
            blackCanCastleKingside = false;
        }
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
}

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
        Position capturedPawnPos(capturedPawnRow, move.to.col);
        setPieceAt(capturedPawnPos, previousState.capturedPiece);
    } else if (previousState.capturedPiece) {
        // Regular capture
        setPieceAt(move.to, previousState.capturedPiece);
    }
    
    // Handle castling move reversal
    if (piece->getType() == PieceType::KING) {
        // Kingside castling
        if (move.from.col == 4 && move.to.col == 6) {
            auto rook = getPieceAt(Position(move.from.row, 5));
            setPieceAt(Position(move.from.row, 7), rook);
            setPieceAt(Position(move.from.row, 5), nullptr);
            if (rook && !previousState.pieceHasMoved) {
                rook->setHasMoved(false);
            }
        }
        // Queenside castling
        else if (move.from.col == 4 && move.to.col == 2) {
            auto rook = getPieceAt(Position(move.from.row, 3));
            setPieceAt(Position(move.from.row, 0), rook);
            setPieceAt(Position(move.from.row, 3), nullptr);
            if (rook && !previousState.pieceHasMoved) {
                rook->setHasMoved(false);
            }
        }
    }
    
    // Restore all game state
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

std::vector<Move> Board::generateLegalMoves() const {
    std::vector<Move> legalMoves;
    
    // Generate moves for all pieces of the current side
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            auto piece = getPieceAt(pos);
            
            if (piece && piece->getColor() == sideToMove) {
                auto pieceMoves = piece->getLegalMoves(*this);
                
                // Filter out moves that would leave king in check
                for (const auto& move : pieceMoves) {
                    if (!wouldBeInCheck(move, sideToMove)) {
                        legalMoves.push_back(move);
                    }
                }
            }
        }
    }
    
    return legalMoves;
}

bool Board::isInCheck() const {
    // Find our king
    auto king = (sideToMove == Color::WHITE) ? whiteKing : blackKing;
    if (!king) return false;
    
    Position kingPos = king->getPosition();
    Color enemyColor = (sideToMove == Color::WHITE) ? Color::BLACK : Color::WHITE;
    
    return isSquareAttacked(kingPos, enemyColor);
}

bool Board::isCheckmate() const {
    if (!isInCheck()) return false;
    
    // If in check, see if there are any legal moves
    auto legalMoves = generateLegalMoves();
    return legalMoves.empty();
}

bool Board::isStalemate() const {
    if (isInCheck()) return false;
    
    // If not in check, see if there are any legal moves
    auto legalMoves = generateLegalMoves();
    return legalMoves.empty();
}

bool Board::isGameOver() const {
    return isCheckmate() || isStalemate();
}

bool Board::isSquareAttacked(const Position& pos, Color attackerColor) const {
    // Check all squares for pieces of the attacking color
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position piecePos(row, col);
            auto piece = getPieceAt(piecePos);
            
            if (piece && piece->getColor() == attackerColor) {
                // Check if this piece can attack the target square
                // Use direct piece-specific attack patterns instead of getLegalMoves()
                if (canPieceAttackSquare(piece, piecePos, pos)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Board::canCastle(const Move& move) const {
    auto piece = getPieceAt(move.from);
    if (!piece || piece->getType() != PieceType::KING) {
        return false;
    }
    
    Color kingColor = piece->getColor();
    bool isKingside = (move.to.col == 6);
    bool isQueenside = (move.to.col == 2);
    
    if (!isKingside && !isQueenside) {
        return false;
    }
    
    // Check castling rights
    if (kingColor == Color::WHITE) {
        if (isKingside && !whiteCanCastleKingside) return false;
        if (isQueenside && !whiteCanCastleQueenside) return false;
    } else {
        if (isKingside && !blackCanCastleKingside) return false;
        if (isQueenside && !blackCanCastleQueenside) return false;
    }
    
    // Check if king has moved
    if (piece->getHasMoved()) return false;
    
    // Check if rook exists and hasn't moved
    int rookCol = isKingside ? 7 : 0;
    auto rook = getPieceAt(Position(move.from.row, rookCol));
    if (!rook || rook->getType() != PieceType::ROOK || rook->getHasMoved()) {
        return false;
    }
    
    // Check if path is clear
    int startCol = std::min(move.from.col, rookCol);
    int endCol = std::max(move.from.col, rookCol);
    
    for (int col = startCol + 1; col < endCol; col++) {
        if (getPieceAt(Position(move.from.row, col))) {
            return false;
        }
    }
    
    // Check if king is in check
    if (isInCheck()) return false;
    
    // Check if king would pass through or land on an attacked square
    Color enemyColor = (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    int direction = isKingside ? 1 : -1;
    
    for (int i = 1; i <= 2; i++) {
        Position testPos(move.from.row, move.from.col + (i * direction));
        if (isSquareAttacked(testPos, enemyColor)) {
            return false;
        }
    }
    
    return true;
}

bool Board::isValidMovePattern(std::shared_ptr<Piece> piece, const Move& move) const {
    // This is a simplified version - full implementation would check piece-specific movement patterns
    auto legalMoves = piece->getLegalMoves(*this);
    
    for (const auto& legalMove : legalMoves) {
        if (legalMove.from == move.from && legalMove.to == move.to && 
            legalMove.promotion == move.promotion) {
            return true;
        }
    }
    
    return false;
}

bool Board::wouldBeInCheck(const Move& move, Color kingColor) const {
    // Make a copy of the board state to test the move
    Board testBoard = *this;
    BoardState tempState;
    
    // Try to make the move on the test board
    if (!testBoard.makeMove(move, tempState)) {
        return true; // Invalid move, consider it as leaving king in check
    }
    
    // Check if the king is in check after the move
    testBoard.sideToMove = kingColor; // Set to check the correct king
    bool inCheck = testBoard.isInCheck();
    
    // Restore the move
    testBoard.unmakeMove(move, tempState);
    
    return inCheck;
}

void Board::print() const {
    std::cout << "\n  a b c d e f g h\n";
    for (int row = 7; row >= 0; row--) {
        std::cout << (row + 1) << " ";
        for (int col = 0; col < 8; col++) {
            auto piece = getPieceAt(Position(row, col));
            if (piece) {
                std::cout << piece->toChar() << " ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << (row + 1) << "\n";
    }
    std::cout << "  a b c d e f g h\n";
    
    // Print game state info
    std::cout << "\nSide to move: " << (sideToMove == Color::WHITE ? "White" : "Black") << std::endl;
    std::cout << "Castling rights: ";
    if (whiteCanCastleKingside) std::cout << "K";
    if (whiteCanCastleQueenside) std::cout << "Q";
    if (blackCanCastleKingside) std::cout << "k";
    if (blackCanCastleQueenside) std::cout << "q";
    if (!whiteCanCastleKingside && !whiteCanCastleQueenside && 
        !blackCanCastleKingside && !blackCanCastleQueenside) {
        std::cout << "none";
    }
    std::cout << std::endl;
    
    if (enPassantTarget.isValid()) {
        std::cout << "En passant target: " << enPassantTarget.toString() << std::endl;
    }
    
    std::cout << "Halfmove clock: " << halfMoveClock << std::endl;
    std::cout << "Fullmove number: " << fullMoveNumber << std::endl;
}

void Board::clear() {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            squares[row][col] = nullptr;
        }
    }
    whiteKing = nullptr;
    blackKing = nullptr;
}

bool Board::canPieceAttackSquare(std::shared_ptr<Piece> piece, const Position& from, const Position& target) const {
    if (!piece) return false;
    
    PieceType type = piece->getType();
    Color color = piece->getColor();
    
    int rowDiff = target.row - from.row;
    int colDiff = target.col - from.col;
    int absRowDiff = abs(rowDiff);
    int absColDiff = abs(colDiff);
    
    switch (type) {
        case PieceType::PAWN: {
            int direction = (color == Color::WHITE) ? 1 : -1;
            // Pawns attack diagonally
            return (rowDiff == direction && absColDiff == 1);
        }
        
        case PieceType::KNIGHT: {
            // Knight moves in L-shape
            return (absRowDiff == 2 && absColDiff == 1) || (absRowDiff == 1 && absColDiff == 2);
        }
        
        case PieceType::BISHOP: {
            // Bishop moves diagonally
            if (absRowDiff != absColDiff) return false;
            return isPathClear(from, target);
        }
        
        case PieceType::ROOK: {
            // Rook moves horizontally or vertically
            if (rowDiff != 0 && colDiff != 0) return false;
            return isPathClear(from, target);
        }
        
        case PieceType::QUEEN: {
            // Queen combines rook and bishop
            bool straightLine = (rowDiff == 0 || colDiff == 0);
            bool diagonal = (absRowDiff == absColDiff);
            if (!straightLine && !diagonal) return false;
            return isPathClear(from, target);
        }
        
        case PieceType::KING: {
            // King moves one square in any direction
            return (absRowDiff <= 1 && absColDiff <= 1 && (absRowDiff != 0 || absColDiff != 0));
        }
        
        default:
            return false;
    }
}

bool Board::isPathClear(const Position& from, const Position& to) const {
    int rowDir = (to.row > from.row) ? 1 : (to.row < from.row) ? -1 : 0;
    int colDir = (to.col > from.col) ? 1 : (to.col < from.col) ? -1 : 0;
    
    Position current = from;
    current.row += rowDir;
    current.col += colDir;
    
    while (current.row != to.row || current.col != to.col) {
        if (getPieceAt(current) != nullptr) {
            return false; // Path is blocked
        }
        current.row += rowDir;
        current.col += colDir;
    }
    
    return true;
}
