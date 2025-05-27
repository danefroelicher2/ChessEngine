#include "game.h"
#include <map>
#include <sstream>
#include <algorithm>

Game::Game() {
    newGame();
}

void Game::newGame() {
    board.setupStartingPosition();
    moveHistory.clear();
    fenHistory.clear();
    fenHistory.push_back(board.toFEN());
    result = GameResult::IN_PROGRESS;
    endReason = GameEndReason::NONE;
}

void Game::newGameFromFEN(const std::string& fen) {
    board.setupFromFEN(fen);
    moveHistory.clear();
    fenHistory.clear();
    fenHistory.push_back(fen);
    result = GameResult::IN_PROGRESS;
    endReason = GameEndReason::NONE;
}

bool Game::makeMove(const Move& move) {
    if (isGameOver()) {
        return false;
    }
    
    // Store the current FEN position before making the move
    std::string currentFEN = board.toFEN();
    
    // Try to make the move
    if (!board.makeMove(move)) {
        return false;
    }
    
    // Add the move to the history
    moveHistory.push_back(move);
    
    // Add the new position to the FEN history
    fenHistory.push_back(board.toFEN());
    
    // Check for end-of-game conditions
    if (board.isCheckmate()) {
        // The side that just moved (not the current side to move) won
        result = (board.getSideToMove() == Color::WHITE) ? 
                  GameResult::BLACK_WINS : GameResult::WHITE_WINS;
        endReason = GameEndReason::CHECKMATE;
    } else if (board.isStalemate()) {
        result = GameResult::DRAW;
        endReason = GameEndReason::STALEMATE;
    } else if (isInsufficientMaterial()) {
        result = GameResult::DRAW;
        endReason = GameEndReason::INSUFFICIENT_MATERIAL;
    } else if (isFiftyMoveRule()) {
        result = GameResult::DRAW;
        endReason = GameEndReason::FIFTY_MOVE_RULE;
    } else if (isThreefoldRepetition()) {
        result = GameResult::DRAW;
        endReason = GameEndReason::THREEFOLD_REPETITION;
    }
    
    return true;
}

bool Game::makeMove(const std::string& moveStr) {
    // Parse the move string (e.g., "e2e4", "g1f3")
    if (moveStr.length() < 4) {
        return false;
    }
    
    Position from = Position::fromString(moveStr.substr(0, 2));
    Position to = Position::fromString(moveStr.substr(2, 2));
    
    if (!from.isValid() || !to.isValid()) {
        return false;
    }
    
    // Check for promotion
    PieceType promotion = PieceType::NONE;
    if (moveStr.length() > 4) {
        char promChar = moveStr[4];
        switch (promChar) {
            case 'q': promotion = PieceType::QUEEN; break;
            case 'r': promotion = PieceType::ROOK; break;
            case 'b': promotion = PieceType::BISHOP; break;
            case 'n': promotion = PieceType::KNIGHT; break;
            default: break;
        }
    }
    
    Move move(from, to, promotion);
    return makeMove(move);
}

bool Game::undoMove() {
    if (moveHistory.empty() || fenHistory.size() < 2) {
        return false;
    }
    
    // Get the last move that was made
    Move lastMove = moveHistory.back();
    
    // Remove the last move from history
    moveHistory.pop_back();
    fenHistory.pop_back();
    
    // Restore the board to the previous position using FEN
    // (This is simpler for the game interface, engine uses different method)
    board.setupFromFEN(fenHistory.back());
    
    // Reset the game result
    result = GameResult::IN_PROGRESS;
    endReason = GameEndReason::NONE;
    
    return true;
}

bool Game::isInsufficientMaterial() const {
    int whiteBishops = 0;
    int whiteKnights = 0;
    int blackBishops = 0;
    int blackKnights = 0;
    int totalPieces = 0;
    bool whiteBishopOnWhite = false;
    bool whiteBishopOnBlack = false;
    bool blackBishopOnWhite = false;
    bool blackBishopOnBlack = false;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            
            if (piece) {
                totalPieces++;
                
                if (piece->getType() == PieceType::PAWN ||
                    piece->getType() == PieceType::ROOK ||
                    piece->getType() == PieceType::QUEEN) {
                    // If there's a pawn, rook, or queen, there is potentially enough material
                    return false;
                }
                
                if (piece->getType() == PieceType::BISHOP) {
                    if (piece->getColor() == Color::WHITE) {
                        whiteBishops++;
                        
                        // Check if the bishop is on a white or black square
                        bool isWhiteSquare = (row + col) % 2 == 0;
                        if (isWhiteSquare) {
                            whiteBishopOnWhite = true;
                        } else {
                            whiteBishopOnBlack = true;
                        }
                    } else {
                        blackBishops++;
                        
                        // Check if the bishop is on a white or black square
                        bool isWhiteSquare = (row + col) % 2 == 0;
                        if (isWhiteSquare) {
                            blackBishopOnWhite = true;
                        } else {
                            blackBishopOnBlack = true;
                        }
                    }
                }
                
                if (piece->getType() == PieceType::KNIGHT) {
                    if (piece->getColor() == Color::WHITE) {
                        whiteKnights++;
                    } else {
                        blackKnights++;
                    }
                }
            }
        }
    }
    
    // King vs King
    if (totalPieces == 2) {
        return true;
    }
    
    // King + Minor piece vs King
    if (totalPieces == 3 && (whiteBishops + whiteKnights + blackBishops + blackKnights == 1)) {
        return true;
    }
    
    // King + Bishop vs King + Bishop (same color squares)
    if (whiteBishops == 1 && blackBishops == 1 && totalPieces == 4) {
        if ((whiteBishopOnWhite && blackBishopOnWhite) || 
            (whiteBishopOnBlack && blackBishopOnBlack)) {
            return true;
        }
    }
    
    return false;
}

bool Game::isFiftyMoveRule() const {
    // Extract the halfmove clock from the current FEN
    std::string fen = board.toFEN();
    std::istringstream ss(fen);
    std::string board, activeColor, castling, enPassant, halfmove, fullmove;
    
    ss >> board >> activeColor >> castling >> enPassant >> halfmove >> fullmove;
    
    int halfMoveClock = std::stoi(halfmove);
    
    // The fifty-move rule applies when the halfmove clock reaches 100 (50 moves by each player)
    return halfMoveClock >= 100;
}

bool Game::isThreefoldRepetition() const {
    // Count the occurrences of each position
    std::map<std::string, int> positionCount;
    
    for (const auto& fen : fenHistory) {
        // Extract the position part of the FEN (ignore the half move clock and full move number)
        std::istringstream ss(fen);
        std::string position, activeColor, castling, enPassant, halfmove, fullmove;
        
        ss >> position >> activeColor >> castling >> enPassant;
        
        // Combine the relevant parts of the FEN
        std::string relevantPart = position + " " + activeColor + " " + castling + " " + enPassant;
        
        // Increment the count for this position
        positionCount[relevantPart]++;
        
        // If this position has occurred three times, return true
        if (positionCount[relevantPart] >= 3) {
            return true;
        }
    }
    
    return false;
}

void Game::endInDrawByAgreement() {
    if (!isGameOver()) {
        result = GameResult::DRAW;
        endReason = GameEndReason::AGREEMENT;
    }
}

void Game::print() const {
    // Print the board
    board.print();
    
    // Print the game status
    std::cout << "Game status: ";
    
    switch (result) {
        case GameResult::IN_PROGRESS:
            std::cout << "In progress" << std::endl;
            break;
        case GameResult::WHITE_WINS:
            std::cout << "White wins";
            
            if (endReason == GameEndReason::CHECKMATE) {
                std::cout << " by checkmate";
            }
            
            std::cout << std::endl;
            break;
        case GameResult::BLACK_WINS:
            std::cout << "Black wins";
            
            if (endReason == GameEndReason::CHECKMATE) {
                std::cout << " by checkmate";
            }
            
            std::cout << std::endl;
            break;
        case GameResult::DRAW:
            std::cout << "Draw";
            
            switch (endReason) {
                case GameEndReason::STALEMATE:
                    std::cout << " by stalemate";
                    break;
                case GameEndReason::INSUFFICIENT_MATERIAL:
                    std::cout << " by insufficient material";
                    break;
                case GameEndReason::FIFTY_MOVE_RULE:
                    std::cout << " by fifty-move rule";
                    break;
                case GameEndReason::THREEFOLD_REPETITION:
                    std::cout << " by threefold repetition";
                    break;
                case GameEndReason::AGREEMENT:
                    std::cout << " by agreement";
                    break;
                default:
                    break;
            }
            
            std::cout << std::endl;
            break;
    }
    
    // Print the move history
    std::cout << "Move history:" << std::endl;
    
    for (size_t i = 0; i < moveHistory.size(); i++) {
        if (i % 2 == 0) {
            std::cout << (i / 2 + 1) << ". ";
        }
        
        std::cout << moveHistory[i].toString();
        
        if (i % 2 == 0) {
            std::cout << " ";
        } else {
            std::cout << std::endl;
        }
    }
    
    if (moveHistory.size() % 2 != 0) {
        std::cout << std::endl;
    }
}