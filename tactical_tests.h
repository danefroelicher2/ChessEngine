#ifndef TACTICAL_TESTS_H
#define TACTICAL_TESTS_H

#include "common.h"
#include "game.h"
#include "engine.h"

class TacticalTester {
public:
    struct TestPosition {
        std::string fen;
        std::string description;
        std::vector<std::string> bestMoves; // Multiple moves might be equally good
        int maxDepth;
        int timeLimit; // milliseconds
    };
    
    // Run a suite of tactical tests
    static bool runTestSuite(Engine& engine);
    
    // Test a single position
    static bool testPosition(Engine& engine, const TestPosition& test);
    
private:
    static std::vector<TestPosition> getBasicTests();
    static std::vector<TestPosition> getTacticalTests();
};

#endif // TACTICAL_TESTS_H