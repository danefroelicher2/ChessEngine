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

void Board::setupFromFEN(const std::string &fen)
{
    clear();

    std::istringstream ss(fen);
    std::string board, activeColor, castling, enPassant, halfmove, fullmove;

    // Extract the 6 parts of the FEN string
    ss >> board >> activeColor >> castling >> enPassant >> halfmove >> fullmove;

    // Parse board position
    int row = 7;
    int col = 0;

    for (char c : board)
    {
        if (c == '/')
        {
            row--;
            col = 0;
        }
        else if (isdigit(c))
        {
            col += c - '0';
        }
        else
        {
            Position pos(row, col);
            std::shared_ptr<Piece> piece = nullptr;
            Color color = isupper(c) ? Color::WHITE : Color::BLACK;
            char pieceChar = tolower(c);

            switch (pieceChar)
            {
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
            case 'k':
            {
                auto king = std::make_shared<King>(color, pos);
                piece = king;
                if (color == Color::WHITE)
                {
                    whiteKing = king;
                }
                else
                {
                    blackKing = king;
                }
                break;
            }
            }

            setPieceAt(pos, piece);
            col++;
        }
    }

    // Parse active color
    sideToMove = (activeColor == "w") ? Color::WHITE : Color::BLACK;

    // Parse castling rights
    whiteCanCastleKingside = castling.find('K') != std::string::npos;
    whiteCanCastleQueenside = castling.find('Q') != std::string::npos;
    blackCanCastleKingside = castling.find('k') != std::string::npos;
    blackCanCastleQueenside = castling.find('q') != std::string::npos;

    // Parse en passant target square
    if (enPassant != "-")
    {
        enPassantTarget = Position::fromString(enPassant);
    }
    else
    {
        enPassantTarget = Position();
    }

    // Parse halfmove clock
    halfMoveClock = std::stoi(halfmove);

    // Parse fullmove number
    fullMoveNumber = std::stoi(fullmove);
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

// Basic validation - check if piece can potentially make this move
auto potentialMoves = piece->getLegalMoves(*this);
bool moveFound = false;
for (const auto& m : potentialMoves) {
    if (m.from.row == move.from.row && m.from.col == move.from.col && 
        m.to.row == move.to.row && m.to.col == move.to.col &&
        m.promotion == move.promotion) {
        moveFound = true;
        break;
    }
}
if (!moveFound) {
    return false;
}

    // Check if the move would leave the king in check
    if (wouldBeInCheck(move, sideToMove))
    {
        return false;
    }

    previousState.pieceHasMoved = piece->getHasMoved();

      // Handle castling
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

    // Handle en passant capture
   if (isPawnMove && move.to == enPassantTarget) {
        // Remove the captured pawn
        int capturedPawnRow = (sideToMove == Color::WHITE) ? move.to.row - 1 : move.to.row + 1;
        previousState.capturedPiece = getPieceAt(Position(capturedPawnRow, move.to.col));
        setPieceAt(Position(capturedPawnRow, move.to.col), nullptr);
        isCapture = true;
        previousState.wasEnPassant = true;
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

// Handle pawn promotion
if (isPawnMove && (move.to.row == 0 || move.to.row == 7) && move.promotion != PieceType::NONE) {
    previousState.wasPromotion = true;
    
    switch (move.promotion) {
        case PieceType::QUEEN:
            piece = std::make_shared<Queen>(sideToMove, move.to);
            break;
        case PieceType::ROOK:
            piece = std::make_shared<Rook>(sideToMove, move.to);
            break;
        case PieceType::BISHOP:
            piece = std::make_shared<Bishop>(sideToMove, move.to);
            break;
        case PieceType::KNIGHT:
            piece = std::make_shared<Knight>(sideToMove, move.to);
            break;
        default:
            // Default to queen if no promotion specified
            piece = std::make_shared<Queen>(sideToMove, move.to);
            break;
    }
    
    // Update king pointers if promoting to king (shouldn't happen in normal chess)
    if (move.promotion == PieceType::KING) {
        auto king = std::make_shared<King>(sideToMove, move.to);
        piece = king;
        if (sideToMove == Color::WHITE) {
            whiteKing = king;
        } else {
            blackKing = king;
        }
    }
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

// Make the move (this should come AFTER promotion handling)
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

bool Board::isCheckmate() const
{
    if (!isInCheck())
        return false;

    return generateLegalMoves().empty();
}

bool Board::isStalemate() const
{
    if (isInCheck())
        return false;

    return generateLegalMoves().empty();
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

bool Board::canCastle(const Move &move) const
{
    // Check if the piece is a king
    auto piece = getPieceAt(move.from);
    if (!piece || piece->getType() != PieceType::KING || piece->getHasMoved())
    {
        return false;
    }

    // Check if the king is in check
    if (isInCheck())
    {
        return false;
    }

    // Kingside castling
    if (move.to.col == move.from.col + 2)
    {
        // Check if the king has castling rights
        if (piece->getColor() == Color::WHITE && !whiteCanCastleKingside)
        {
            return false;
        }
        if (piece->getColor() == Color::BLACK && !blackCanCastleKingside)
        {
            return false;
        }

        // Check if the rook is there and hasn't moved
        auto rook = getPieceAt(Position(move.from.row, 7));
        if (!rook || rook->getType() != PieceType::ROOK || rook->getHasMoved())
        {
            return false;
        }

        // Check if the squares between the king and the rook are empty
        if (getPieceAt(Position(move.from.row, 5)) || getPieceAt(Position(move.from.row, 6)))
        {
            return false;
        }

        // Check if the king would move through or end up in check
        if (isSquareAttacked(Position(move.from.row, 5), (piece->getColor() == Color::WHITE) ? Color::BLACK : Color::WHITE) ||
            isSquareAttacked(Position(move.from.row, 6), (piece->getColor() == Color::WHITE) ? Color::BLACK : Color::WHITE))
        {
            return false;
        }

        return true;
    }
    // Queenside castling
    else if (move.to.col == move.from.col - 2)
    {
        // Check if the king has castling rights
        if (piece->getColor() == Color::WHITE && !whiteCanCastleQueenside)
        {
            return false;
        }
        if (piece->getColor() == Color::BLACK && !blackCanCastleQueenside)
        {
            return false;
        }

        // Check if the rook is there and hasn't moved
        auto rook = getPieceAt(Position(move.from.row, 0));
        if (!rook || rook->getType() != PieceType::ROOK || rook->getHasMoved())
        {
            return false;
        }

        // Check if the squares between the king and the rook are empty
        if (getPieceAt(Position(move.from.row, 1)) ||
            getPieceAt(Position(move.from.row, 2)) ||
            getPieceAt(Position(move.from.row, 3)))
        {
            return false;
        }

        // Check if the king would move through or end up in check
        if (isSquareAttacked(Position(move.from.row, 3), (piece->getColor() == Color::WHITE) ? Color::BLACK : Color::WHITE) ||
            isSquareAttacked(Position(move.from.row, 2), (piece->getColor() == Color::WHITE) ? Color::BLACK : Color::WHITE))
        {
            return false;
        }

        return true;
    }

    return false;
}

bool Board::wouldBeInCheck(const Move &move, Color kingColor) const
{
    // Create a copy of the current board
    Board tempBoard = *this;

    // Get the piece at the source position
    auto piece = tempBoard.getPieceAt(move.from);
    if (!piece)
        return false;

    // Make the move on the temporary board
    tempBoard.setPieceAt(move.from, nullptr);

    // Handle en passant capture
    if (piece->getType() == PieceType::PAWN && move.to == enPassantTarget)
    {
        int capturedPawnRow = (piece->getColor() == Color::WHITE) ? move.to.row - 1 : move.to.row + 1;
        tempBoard.setPieceAt(Position(capturedPawnRow, move.to.col), nullptr);
    }

    // Handle castling
    if (piece->getType() == PieceType::KING)
    {
        if (move.from.col == 4 && move.to.col == 6)
        {
            // Kingside castling
            auto rook = tempBoard.getPieceAt(Position(move.from.row, 7));
            tempBoard.setPieceAt(Position(move.from.row, 5), rook);
            tempBoard.setPieceAt(Position(move.from.row, 7), nullptr);
        }
        else if (move.from.col == 4 && move.to.col == 2)
        {
            // Queenside castling
            auto rook = tempBoard.getPieceAt(Position(move.from.row, 0));
            tempBoard.setPieceAt(Position(move.from.row, 3), rook);
            tempBoard.setPieceAt(Position(move.from.row, 0), nullptr);
        }
    }

    // Set the piece at the destination position
    tempBoard.setPieceAt(move.to, piece);

    // Find the king's position
    Position kingPos;

    if (piece->getType() == PieceType::KING && piece->getColor() == kingColor)
    {
        kingPos = move.to;
    }
    else
    {
        // Find the king
        auto king = (kingColor == Color::WHITE) ? tempBoard.whiteKing : tempBoard.blackKing;
        if (!king)
            return false;
        kingPos = king->getPosition();
    }

    // Check if the king is in check after the move
    return tempBoard.isSquareAttacked(kingPos, (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE);
}