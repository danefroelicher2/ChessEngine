#include "tactical_tests.h"
#include <iostream>
#include <algorithm>

bool TacticalTester::runTestSuite(Engine& engine) {
    std::cout << "=== Running Tactical Test Suite ===" << std::endl;
    
    auto basicTests = getBasicTests();
    auto tacticalTests = getTacticalTests();
    
    int passed = 0;
    int total = 0;
    
    std::cout << "\n--- Basic Tests ---" << std::endl;
    for (const auto& test : basicTests) {
        if (testPosition(engine, test)) {
            passed++;
        }
        total++;
    }
    
    std::cout << "\n--- Tactical Tests ---" << std::endl;
    for (const auto& test : tacticalTests) {
        if (testPosition(engine, test)) {
            passed++;
        }
        total++;
    }
    
    std::cout << "\n=== Test Results: " << passed << "/" << total << " passed ===" << std::endl;
    return passed == total;
}

bool TacticalTester::testPosition(Engine& engine, const TestPosition& test) {
    std::cout << "\nTesting: " << test.description << std::endl;
    std::cout << "FEN: " << test.fen << std::endl;
    
    // Set up the position
    Game game;
    game.newGameFromFEN(test.fen);
    
    // Create a temporary engine with the game
    Engine testEngine(game, test.maxDepth);
    
    // Get the best move
    auto start = std::chrono::high_resolution_clock::now();
    Move bestMove = testEngine.getBestMove();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::string moveStr = bestMove.toString();
    
    // Check if the move is in the list of acceptable moves
    bool found = std::find(test.bestMoves.begin(), test.bestMoves.end(), moveStr) != test.bestMoves.end();
    
    std::cout << "Engine move: " << moveStr << " (in " << duration.count() << "ms)";
    std::cout << " - " << (found ? "PASS" : "FAIL") << std::endl;
    
    if (!found) {
        std::cout << "Expected one of: ";
        for (size_t i = 0; i < test.bestMoves.size(); i++) {
            std::cout << test.bestMoves[i];
            if (i < test.bestMoves.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }
    
    return found;
}

std::vector<TacticalTester::TestPosition> TacticalTester::getBasicTests() {
    return {
        // Mate in 1 tests
        {
            "8/8/8/8/8/8/6pp/6kK w - - 0 1",
            "Basic mate in 1 - back rank",
            {"h1h2"},
            3,
            1000
        },
        {
            "rnbqkb1r/pppp1ppp/5n2/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 4 4",
            "Scholar's mate",
            {"c4f7"},
            3,
            1000
        },
        // Simple tactics
        {
            "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
            "Defend against Scholar's mate",
            {"g8f6", "f8e7", "d8f6"},
            3,
            2000
        },
        {
            "rnbqkbnr/ppp2ppp/8/3pp3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq d6 0 3",
            "Capture center pawn",
            {"c4d5"},
            3,
            1000
        }
    };
}

std::vector<TacticalTester::TestPosition> TacticalTester::getTacticalTests() {
    return {
        // Forks and pins
        {
            "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/3P1N2/PPP2PPP/RNBQKB1R b KQkq - 0 4",
            "Knight fork opportunity",
            {"f6g4"},
            4,
            3000
        },
        {
            "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 4 5",
            "Pin the knight",
            {"c1g5"},
            4,
            3000
        },
        // Discovered attacks
        {
            "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
            "Discovered attack with check",
            {"f3d4"},
            4,
            3000
        },
        // Simple combinations
        {
            "r2qkb1r/ppp2ppp/2npbn2/4p3/2B1P3/3P1N2/PPP2PPP/RNBQ1RK1 w kq - 0 6",
            "Remove defender",
            {"c4f7"},
            5,
            5000
        }
    };
}