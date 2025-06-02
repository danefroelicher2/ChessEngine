#ifndef PERFT_H
#define PERFT_H

#include "main.h"
#include "board.h"

class PerftTester
{
public:
    struct PerftResult
    {
        uint64_t nodes;
        uint64_t captures;
        uint64_t enPassant;
        uint64_t castles;
        uint64_t promotions;
        uint64_t checks;
        uint64_t checkmates;

        PerftResult() : nodes(0), captures(0), enPassant(0), castles(0),
                        promotions(0), checks(0), checkmates(0) {}
    };

    struct PerftBenchmark {
        std::string position;
        std::string description;
        int depth;
        uint64_t expectedNodes;
        int maxTimeMs;  
        int minNodesPerSec; 
        
        PerftBenchmark(const std::string& pos, const std::string& desc, int d, 
                      uint64_t nodes, int maxTime, int minNPS = 0)
            : position(pos), description(desc), depth(d), expectedNodes(nodes), 
              maxTimeMs(maxTime), minNodesPerSec(minNPS) {}
    };
    
    struct BenchmarkResult {
        std::string testName;
        bool correctness;      
        bool performance;      
        uint64_t actualNodes;
        uint64_t expectedNodes;
        int timeMs;
        int nodesPerSec;
        std::string errorMessage;
        
        BenchmarkResult() : correctness(false), performance(false), actualNodes(0), 
                           expectedNodes(0), timeMs(0), nodesPerSec(0) {}
    };
    

    static bool runBenchmarkSuite();
    static BenchmarkResult runSingleBenchmark(const PerftBenchmark& benchmark);
    static bool runPerformanceRegression();
    static void printBenchmarkReport(const std::vector<BenchmarkResult>& results);
    static PerftResult perft(Board &board, int depth);
    static void perftDivide(Board &board, int depth);
    static bool runTestSuite();
    static bool testPosition(const std::string &fen, int depth, uint64_t expectedNodes);
    static bool testEnPassant();
    static bool testCastling();
    static bool testPromotion();
    static bool runEnhancedTestSuite();

private:
    static PerftResult perftRecursive(Board &board, int depth, bool root = false);
};

#endif // PERFT_H