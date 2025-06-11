#ifndef EPD_TESTER_H
#define EPD_TESTER_H

#include "common.h"
#include "engine.h"
#include "game.h"
#include <fstream>
#include <sstream>

class EPDTester {
public:
    struct EPDPosition {
        std::string fen;                        // Position in FEN format
        std::vector<std::string> bestMoves;     // bm operations (best moves)
        std::vector<std::string> avoidMoves;    // am operations (avoid moves)
        std::string id;                         // Position description/ID
        int timeLimit;                          // Search time limit in ms
        int depthLimit;                         // Search depth limit
        std::string comment;                    // Additional comments
        
        EPDPosition() : timeLimit(5000), depthLimit(8) {}
    };
    
    struct EPDResult {
        std::string positionId;
        bool foundBestMove;
        bool avoidedBadMove;
        std::string engineMove;
        std::vector<std::string> expectedMoves;
        std::vector<std::string> forbiddenMoves;
        int searchTimeMs;
        int searchDepth;
        std::string errorMessage;
        
        EPDResult() : foundBestMove(false), avoidedBadMove(true), 
                     searchTimeMs(0), searchDepth(0) {}
    };
    
    struct EPDSuiteResult {
        int totalPositions;
        int correctBestMoves;
        int correctAvoidMoves;
        int totalTimeMs;
        std::vector<EPDResult> detailedResults;
        
        EPDSuiteResult() : totalPositions(0), correctBestMoves(0), 
                          correctAvoidMoves(0), totalTimeMs(0) {}
    };
    
    // Main testing methods
    static bool runEPDSuite(const std::string& filename, Engine& engine);
    static bool runBuiltInSuite(Engine& engine);
    static EPDResult testSinglePosition(const EPDPosition& pos, Engine& engine);
    
    // Utility methods
    static EPDPosition parseEPDLine(const std::string& line);
    static std::vector<EPDPosition> loadEPDFile(const std::string& filename);
    static std::vector<EPDPosition> getBuiltInPositions();
    static void printSuiteResults(const EPDSuiteResult& results);
    static bool saveResults(const EPDSuiteResult& results, const std::string& filename);
    
private:
    static std::vector<std::string> splitString(const std::string& str, char delimiter);
    static std::string trim(const std::string& str);
    static bool moveMatches(const std::string& engineMove, const std::string& expectedMove);
};