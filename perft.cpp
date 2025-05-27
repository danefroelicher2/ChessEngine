#include "perft.h"
#include "board_state.h"
#include <iostream>
#include <iomanip>
#include <chrono>

PerftTester::PerftResult PerftTester::perft(Board& board, int depth) {
    return perftRecursive(board, depth, true);
}

PerftTester::PerftResult PerftTester::perftRecursive(Board& board, int depth, bool root) {
    PerftResult result;
    
    if (depth == 0) {
        result.nodes = 1;
        return result;
    }
    
    auto legalMoves = board.generateLegalMoves();
    
    for (const auto& move : legalMoves) {
        BoardState previousState;
        
        // Check move characteristics before making the move
        bool isCapture = board.getPieceAt(move.to) != nullptr;
        bool isEnPassant = false;
        bool isCastle = false;
        bool isPromotion = move.promotion != PieceType::NONE;
        
        auto movingPiece = board.getPieceAt(move.from);
        if (movingPiece) {
            // Check for en passant
            if (movingPiece->getType() == PieceType::PAWN && 
                move.to == board.getEnPassantTarget()) {
                isEnPassant = true;
                isCapture = true; // En passant is a capture
            }
            
            // Check for castling
            if (movingPiece->getType() == PieceType::KING) {
                if (abs(move.to.col - move.from.col) == 2) {
                    isCastle = true;
                }
            }
        }
        
        // Make the move
        if (!board.makeMove(move, previousState)) {
            std::cerr << "Error: Invalid move generated: " << move.toString() << std::endl;
            continue;
        }
        
        // Check if the position is in check or checkmate after the move
        bool inCheck = board.isInCheck();
        bool isCheckmate = board.isCheckmate();
        
        // Recursive call
        PerftResult childResult = perftRecursive(board, depth - 1, false);
        
        // Unmake the move
        board.unmakeMove(move, previousState);
        
        // Accumulate results
        result.nodes += childResult.nodes;
        result.captures += childResult.captures;
        result.enPassant += childResult.enPassant;
        result.castles += childResult.castles;
        result.promotions += childResult.promotions;
        result.checks += childResult.checks;
        result.checkmates += childResult.checkmates;
        
        // Count this move's characteristics
        if (isCapture) result.captures += childResult.nodes;
        if (isEnPassant) result.enPassant += childResult.nodes;
        if (isCastle) result.castles += childResult.nodes;
        if (isPromotion) result.promotions += childResult.nodes;
        if (inCheck) result.checks += childResult.nodes;
        if (isCheckmate) result.checkmates += childResult.nodes;
        
        // Print move breakdown at root level
        if (root) {
            std::cout << move.toString() << ": " << childResult.nodes << std::endl;
        }
    }
    
    return result;
}

void PerftTester::perftDivide(Board& board, int depth) {
    std::cout << "\nPerft divide for depth " << depth << ":" << std::endl;
    std::cout << "FEN: " << board.toFEN() << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    PerftResult result = perft(board, depth);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "\nTotal nodes: " << result.nodes << std::endl;
    std::cout << "Captures: " << result.captures << std::endl;
    std::cout << "En passant: " << result.enPassant << std::endl;
    std::cout << "Castles: " << result.castles << std::endl;
    std::cout << "Promotions: " << result.promotions << std::endl;
    std::cout << "Checks: " << result.checks << std::endl;
    std::cout << "Checkmates: " << result.checkmates << std::endl;
    std::cout << "Time: " << duration.count() << "ms" << std::endl;
    
    if (duration.count() > 0) {
        std::cout << "Nodes/sec: " << (result.nodes * 1000) / duration.count() << std::endl;
    }
}

bool PerftTester::testPosition(const std::string& fen, int depth, uint64_t expectedNodes) {
    Board board;
    board.setupFromFEN(fen);
    
    std::cout << "Testing position: " << fen << std::endl;
    std::cout << "Depth: " << depth << ", Expected: " << expectedNodes << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    PerftResult result = perft(board, depth);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    bool passed = (result.nodes == expectedNodes);
    
    std::cout << "Result: " << result.nodes << " nodes in " << duration.count() << "ms - ";
    std::cout << (passed ? "PASS" : "FAIL") << std::endl;
    
    if (!passed) {
        std::cout << "Expected: " << expectedNodes << ", Got: " << result.nodes << std::endl;
        // Run divide to help debug
        std::cout << "Running divide to help debug:" << std::endl;
        perftDivide(board, depth);
    }
    
    std::cout << std::endl;
    return passed;
}

bool PerftTester::runTestSuite() {
    std::cout << "=== Running Perft Test Suite ===" << std::endl;
    
    bool allPassed = true;
    
    // Test 1: Starting position
    allPassed &= testPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20);
    allPassed &= testPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400);
    allPassed &= testPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902);
    allPassed &= testPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281);
    
    // Test 2: Kiwipete position (complex position with many special moves)
    allPassed &= testPosition("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 1, 48);
    allPassed &= testPosition("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2, 2039);
    allPassed &= testPosition("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3, 97862);
    
    // Test 3: Position with en passant
    allPassed &= testPosition("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 1, 20);
    allPassed &= testPosition("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", 2, 400);
    
    // Test 4: Position with castling
    allPassed &= testPosition("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 1, 26);
    allPassed &= testPosition("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", 2, 568);
    
    // Test 5: Promotion position
    allPassed &= testPosition("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 1, 14);
    allPassed &= testPosition("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 2, 191);
    
    std::cout << "=== Test Suite " << (allPassed ? "PASSED" : "FAILED") << " ===" << std::endl;
    return allPassed;
}