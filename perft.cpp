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
    std::cout << "=== Enhanced Perft Test Suite ===" << std::endl;
    
    bool allPassed = true;
    
    // STEP 1: Define comprehensive test positions
    struct PerftTest {
        std::string fen;
        std::string description;
        std::vector<std::pair<int, uint64_t>> depthResults; // depth -> expected nodes
    };
    
    std::vector<PerftTest> tests = {
        // Starting position - most important test
        {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            "Starting Position",
            {{1, 20}, {2, 400}, {3, 8902}, {4, 197281}}
        },
        
        // Kiwipete (complex tactical position with many special moves)
        {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "Kiwipete Position",
            {{1, 48}, {2, 2039}, {3, 97862}}
        },
        
        // Position 3 (endgame with promotions)
        {
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "Position 3 (Promotions)",
            {{1, 14}, {2, 191}, {3, 2812}}
        },
        
        // Position 4 (avoid illegal en passant)
        {
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "Position 4 (Illegal EP Avoidance)",
            {{1, 6}, {2, 264}, {3, 9467}}
        },
        
        // Position 5 (en passant and promotion combinations)
        {
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "Position 5 (EP + Promotion)",
            {{1, 44}, {2, 1486}}
        },
        
        // Position 6 (complex middle game)
        {
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
            "Position 6 (Complex Middle Game)",
            {{1, 46}, {2, 2079}}
        },
        
        // Castling rights edge cases
        {
            "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
            "Castling Test Position",
            {{1, 26}, {2, 568}}
        },
        
        // En passant specific test
        {
            "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
            "En Passant Test",
            {{1, 31}, {2, 570}}
        }
    };
    
    // STEP 2: Run all test positions
    int totalTests = 0;
    int passedTests = 0;
    
    for (const auto& test : tests) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "Testing: " << test.description << std::endl;
        std::cout << "FEN: " << test.fen << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        Board board;
        try {
            board.setupFromFEN(test.fen);
        } catch (const std::exception& e) {
            std::cout << "ERROR: Failed to setup position: " << e.what() << std::endl;
            allPassed = false;
            continue;
        }
        
        bool testPassed = true;
        
        for (const auto& depthResult : test.depthResults) {
            totalTests++;
            int depth = depthResult.first;
            uint64_t expected = depthResult.second;
            
            std::cout << "\n  Testing depth " << depth << "..." << std::endl;
            
            auto start = std::chrono::high_resolution_clock::now();
            PerftResult result;
            
            try {
                result = perft(board, depth);
            } catch (const std::exception& e) {
                std::cout << "  ERROR: Perft failed: " << e.what() << std::endl;
                testPassed = false;
                continue;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            bool depthPassed = (result.nodes == expected);
            
            std::cout << "    Nodes: " << result.nodes << " (expected " << expected << ")";
            std::cout << " - " << (depthPassed ? "✓ PASS" : "✗ FAIL");
            std::cout << " [" << duration.count() << "ms]" << std::endl;
            
            if (duration.count() > 0) {
                std::cout << "    Speed: " << (result.nodes * 1000 / duration.count()) << " nodes/sec" << std::endl;
            }
            
            if (depthPassed) {
                passedTests++;
            } else {
                std::cout << "    FAILURE DETAILS:" << std::endl;
                std::cout << "      Expected: " << expected << std::endl;
                std::cout << "      Got:      " << result.nodes << std::endl;
                std::cout << "      Diff:     " << (static_cast<int64_t>(result.nodes) - static_cast<int64_t>(expected)) << std::endl;
                
                // Run divide for debugging if depth is reasonable
                if (depth <= 3) {
                    std::cout << "    RUNNING DIVIDE for debugging:" << std::endl;
                    try {
                        perftDivide(board, depth);
                    } catch (const std::exception& e) {
                        std::cout << "    ERROR in divide: " << e.what() << std::endl;
                    }
                }
                testPassed = false;
            }
        }
        
        allPassed &= testPassed;
        std::cout << "\nPosition result: " << (testPassed ? "✓ PASSED" : "✗ FAILED") << std::endl;
    }
    
    // STEP 3: Run special move tests
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "=== SPECIAL MOVE FEATURE TESTS ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    bool specialTestsPassed = true;
    specialTestsPassed &= testEnPassant();
    specialTestsPassed &= testCastling();
    specialTestsPassed &= testPromotion();
    
    allPassed &= specialTestsPassed;
    
    // STEP 4: Final summary
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "=== FINAL PERFT TEST RESULTS ===" << std::endl;
    std::cout << "Position Tests: " << passedTests << "/" << totalTests << std::endl;
    std::cout << "Special Tests:  " << (specialTestsPassed ? "PASSED" : "FAILED") << std::endl;
    std::cout << "Overall Result: " << (allPassed ? "🎉 ALL TESTS PASSED!" : "⚠️  SOME TESTS FAILED") << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    return allPassed;

    bool PerftTester::testEnPassant() {
    std::cout << "\n--- Testing En Passant ---" << std::endl;
    
    bool allPassed = true;
    
    // Test 1: White en passant capture
    std::cout << "Test 1: White en passant capture" << std::endl;
    Board board1;
    board1.setupFromFEN("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
    
    auto moves1 = board1.generateLegalMoves();
    bool foundWhiteEP = false;
    
    for (const auto& move : moves1) {
        if (move.from == Position(4, 4) && move.to == Position(5, 5)) { // e5xf6
            foundWhiteEP = true;
            std::cout << "  ✓ Found white en passant: " << move.toString() << std::endl;
            break;
        }
    }
    
    if (!foundWhiteEP) {
        std::cout << "  ✗ White en passant capture NOT found" << std::endl;
        allPassed = false;
    }
    
    // Test 2: Black en passant capture
    std::cout << "Test 2: Black en passant capture" << std::endl;
    Board board2;
    board2.setupFromFEN("rnbqkbnr/pppp1ppp/8/8/3Pp3/8/PPP1PPPP/RNBQKBNR b KQkq d3 0 2");
    
    auto moves2 = board2.generateLegalMoves();
    bool foundBlackEP = false;
    
    for (const auto& move : moves2) {
        if (move.from == Position(3, 4) && move.to == Position(2, 3)) { // exd3
            foundBlackEP = true;
            std::cout << "  ✓ Found black en passant: " << move.toString() << std::endl;
            break;
        }
    }
    
    if (!foundBlackEP) {
        std::cout << "  ✗ Black en passant capture NOT found" << std::endl;
        allPassed = false;
    }
    
    // Test 3: Invalid en passant (should not be generated)
    std::cout << "Test 3: Invalid en passant (should be blocked)" << std::endl;
    Board board3;
    board3.setupFromFEN("8/8/8/3pP3/8/8/8/4K2k w - d6 0 1");
    
    auto moves3 = board3.generateLegalMoves();
    bool foundInvalidEP = false;
    
    for (const auto& move : moves3) {
        if (move.from == Position(4, 4) && move.to == Position(5, 3)) { // e5xd6
            foundInvalidEP = true;
            break;
        }
    }
    
    if (foundInvalidEP) {
        std::cout << "  ✗ Invalid en passant WAS generated (should be blocked)" << std::endl;
        allPassed = false;
    } else {
        std::cout << "  ✓ Invalid en passant correctly NOT generated" << std::endl;
    }
    
    std::cout << "En Passant Test Result: " << (allPassed ? "✓ PASSED" : "✗ FAILED") << std::endl;
    return allPassed;
}

bool PerftTester::testCastling() {
    std::cout << "\n--- Testing Castling ---" << std::endl;
    
    bool allPassed = true;
    
    // Test 1: All castling rights available
    std::cout << "Test 1: All castling moves available" << std::endl;
    Board board1;
    board1.setupFromFEN("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    auto moves1 = board1.generateLegalMoves();
    bool foundWhiteKingside = false, foundWhiteQueenside = false;
    bool foundBlackKingside = false, foundBlackQueenside = false;
    
    for (const auto& move : moves1) {
        if (move.from == Position(0, 4) && move.to == Position(0, 6)) {
            foundWhiteKingside = true;
            std::cout << "  ✓ Found white kingside castling" << std::endl;
        }
        if (move.from == Position(0, 4) && move.to == Position(0, 2)) {
            foundWhiteQueenside = true;
            std::cout << "  ✓ Found white queenside castling" << std::endl;
        }
    }
    
    // Test black's turn
    board1.switchSideToMove();
    auto blackMoves = board1.generateLegalMoves();
    
    for (const auto& move : blackMoves) {
        if (move.from == Position(7, 4) && move.to == Position(7, 6)) {
            foundBlackKingside = true;
            std::cout << "  ✓ Found black kingside castling" << std::endl;
        }
        if (move.from == Position(7, 4) && move.to == Position(7, 2)) {
            foundBlackQueenside = true;
            std::cout << "  ✓ Found black queenside castling" << std::endl;
        }
    }
    
    if (!foundWhiteKingside || !foundWhiteQueenside || !foundBlackKingside || !foundBlackQueenside) {
        std::cout << "  ✗ Missing castling moves" << std::endl;
        allPassed = false;
    }
    
    // Test 2: Castling blocked by pieces
    std::cout << "Test 2: Castling blocked by pieces" << std::endl;
    Board board2;
    board2.setupFromFEN("r3k1nr/8/8/8/8/8/8/RN2K2R w KQkq - 0 1");
    
    auto moves2 = board2.generateLegalMoves();
    bool foundBlockedCastling = false;
    
    for (const auto& move : moves2) {
        if ((move.from == Position(0, 4) && move.to == Position(0, 6)) ||
            (move.from == Position(0, 4) && move.to == Position(0, 2))) {
            foundBlockedCastling = true;
            break;
        }
    }
    
    if (foundBlockedCastling) {
        std::cout << "  ✗ Castling allowed when blocked by pieces" << std::endl;
        allPassed = false;
    } else {
        std::cout << "  ✓ Castling correctly blocked by pieces" << std::endl;
    }
    
    std::cout << "Castling Test Result: " << (allPassed ? "✓ PASSED" : "✗ FAILED") << std::endl;
    return allPassed;
}

bool PerftTester::testPromotion() {
    std::cout << "\n--- Testing Promotion ---" << std::endl;
    
    bool allPassed = true;
    
    // Test 1: White pawn promotion (forward)
    std::cout << "Test 1: White pawn promotion" << std::endl;
    Board board1;
    board1.setupFromFEN("8/P7/8/8/8/8/8/7k w - - 0 1");
    
    auto moves1 = board1.generateLegalMoves();
    int whitePromotionMoves = 0;
    std::set<PieceType> promotionTypes;
    
    for (const auto& move : moves1) {
        if (move.from == Position(6, 0) && move.to == Position(7, 0)) {
            if (move.promotion != PieceType::NONE) {
                whitePromotionMoves++;
                promotionTypes.insert(move.promotion);
                std::cout << "  ✓ Found promotion to " << 
                    (move.promotion == PieceType::QUEEN ? "Queen" :
                     move.promotion == PieceType::ROOK ? "Rook" :
                     move.promotion == PieceType::BISHOP ? "Bishop" :
                     move.promotion == PieceType::KNIGHT ? "Knight" : "Unknown") << std::endl;
            }
        }
    }
    
    if (whitePromotionMoves != 4) {
        std::cout << "  ✗ Expected 4 promotion moves, found " << whitePromotionMoves << std::endl;
        allPassed = false;
    }
    
    if (promotionTypes.size() != 4) {
        std::cout << "  ✗ Expected 4 different promotion types, found " << promotionTypes.size() << std::endl;
        allPassed = false;
    }
    
    // Test 2: Black pawn promotion (capture)
    std::cout << "Test 2: Black pawn promotion with capture" << std::endl;
    Board board2;
    board2.setupFromFEN("7k/8/8/8/8/8/7p/7R b - - 0 1");
    
    auto moves2 = board2.generateLegalMoves();
    int blackPromotionMoves = 0;
    
    for (const auto& move : moves2) {
        if (move.from == Position(1, 7) && move.to == Position(0, 7)) {
            if (move.promotion != PieceType::NONE) {
                blackPromotionMoves++;
            }
        }
    }
    
    if (blackPromotionMoves != 4) {
        std::cout << "  ✗ Expected 4 black promotion moves, found " << blackPromotionMoves << std::endl;
        allPassed = false;
    } else {
        std::cout << "  ✓ Found all 4 black promotion moves" << std::endl;
    }
    
    // Test 3: Under-promotion should be included
    std::cout << "Test 3: Under-promotion moves included" << std::endl;
    bool foundUnderPromotion = false;
    
    for (const auto& move : moves1) {
        if (move.from == Position(6, 0) && move.to == Position(7, 0)) {
            if (move.promotion == PieceType::KNIGHT || 
                move.promotion == PieceType::BISHOP || 
                move.promotion == PieceType::ROOK) {
                foundUnderPromotion = true;
                break;
            }
        }
    }
    
    if (!foundUnderPromotion) {
        std::cout << "  ✗ Under-promotion moves not found" << std::endl;
        allPassed = false;
    } else {
        std::cout << "  ✓ Under-promotion moves correctly included" << std::endl;
    }
    
    std::cout << "Promotion Test Result: " << (allPassed ? "✓ PASSED" : "✗ FAILED") << std::endl;
    return allPassed;
}
}