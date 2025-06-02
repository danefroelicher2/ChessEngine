// ADD THIS TO YOUR TESTING CODE (or create test_fen.cpp)

void testFENSafety() {
    std::cout << "=== Testing FEN Parsing Safety ===" << std::endl;
    
    Board board;
    
    // Test cases that should NOT crash (should fall back to starting position)
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"", "Empty string"},
        {"invalid", "Single word"},
        {"abc def ghi", "Too few components"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - abc def", "Invalid move counters"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1", "Missing rank"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1", "Invalid active color"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w XYZ - 0 1", "Invalid castling"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq z9 0 1", "Invalid en passant"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - -5 1", "Negative halfmove"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0", "Zero fullmove"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBN w KQkq - 0 1", "Missing white king"},
        {"rnbqkbn/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Missing black king"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNRK w KQkq - 0 1", "Extra king"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/9 w KQkq - 0 1", "Invalid empty count"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR@ w KQkq - 0 1", "Invalid piece character"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNRR w KQkq - 0 1", "Too many pieces in rank"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1", "Too few ranks"},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/extra w KQkq - 0 1", "Too many ranks"}
    };
    
    int passedTests = 0;
    int totalTests = testCases.size();
    
    for (const auto& testCase : testCases) {
        std::cout << "\nTesting: " << testCase.second << std::endl;
        std::cout << "FEN: \"" << testCase.first << "\"" << std::endl;
        
        try {
            board.setupFromFEN(testCase.first);
            std::cout << "Result: ✓ No crash (fell back to starting position)" << std::endl;
            passedTests++;
        } catch (const std::exception& e) {
            std::cout << "Result: ✗ Exception caught: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Result: ✗ Unknown exception caught" << std::endl;
        }
    }
    
    // Test valid FEN strings (should work normally)
    std::vector<std::pair<std::string, std::string>> validTests = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Starting position"},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", "After e2-e4"},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "Kiwipete position"}
    };
    
    std::cout << "\n=== Testing Valid FEN Strings ===" << std::endl;
    for (const auto& testCase : validTests) {
        std::cout << "\nTesting: " << testCase.second << std::endl;
        
        try {
            board.setupFromFEN(testCase.first);
            std::cout << "Result: ✓ Successfully loaded" << std::endl;
            passedTests++;
            totalTests++;
        } catch (const std::exception& e) {
            std::cout << "Result: ✗ Exception: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n=== FEN Safety Test Results ===" << std::endl;
    std::cout << "Passed: " << passedTests << "/" << totalTests << " tests" << std::endl;
    
    if (passedTests == totalTests) {
        std::cout << "🎉 All FEN safety tests PASSED!" << std::endl;
    } else {
        std::cout << "⚠️  Some tests failed - check implementation" << std::endl;
    }
}

// Add this call to your main() function or test runner:
// testFENSafety();