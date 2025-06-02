#ifndef ENGINE_H
#define ENGINE_H

#include "main.h"
#include "game.h"
#include "transposition.h"
#include "zobrist.h"
#include <mutex>
#include <atomic>
#include <unordered_map>

// Maximum search depth - adjust if needed
#define MAX_PLY 64
#define MAX_QSEARCH_DEPTH 8

class Engine
{
private:
    // CORE ENGINE DATA
    int maxDepth;
    Game &game;
    TranspositionTable transpositionTable;
    Zobrist zobristHasher;

    // PRINCIPAL VARIATION (PV) STORAGE
    std::vector<Move> principalVariation;
    std::vector<std::vector<Move>> pvTable; // Stores PV for each depth

    // ENHANCED: KILLER MOVE TABLES - 4 slots instead of 2
    Move killerMoves[MAX_PLY][4];

    // COUNTER MOVE HEURISTIC TABLE
    Move counterMoves[6][2][64][64]; // [piece_type][color][from_square][to_square]

    // HISTORY HEURISTIC TABLE
    int historyTable[2][64][64]; // [color][from_square][to_square]

    // ENHANCED MOVE ORDERING STRUCTURES
    int butterflyHistory[64][64];                    // [from_square][to_square]
    Move countermoveHistory[6][64];                  // [piece_type][to_square]
    mutable std::unordered_map<uint64_t, int> seeCache;      // SEE cache for performance

    // NULL MOVE PRUNING TRACKING
    bool nullMoveAllowed[MAX_PLY];  // Track null move usage per ply

    // SEARCH STATISTICS
    long nodesSearched;
    std::chrono::time_point<std::chrono::high_resolution_clock> searchStartTime;

    // NEW: Extension tracking and limiting
    static const int MAX_EXTENSIONS_PER_PLY = 2;     // Maximum extensions per ply
    static const int MAX_TOTAL_EXTENSIONS = 16;      // Maximum total extensions per search path
    static const int SINGULAR_MARGIN_BASE = 64;      // Base margin for singular move detection
    static const int CHECK_EXTENSION_LIMIT = 8;      // Limit check extensions per search
    
    // Extension counters (per search path)
    int extensionsUsed[MAX_PLY];                     // Extensions used at each ply
    int totalExtensionsInPath;                       // Total extensions in current search path

    // NEW: Pruning Integration Tracking
    mutable int pruningUsedAtPly[MAX_PLY];          // Track pruning techniques used per ply
    mutable long pruningStats[5];                    // Stats: [null_move, razoring, futility, lmr, conflicts]

    // TIME MANAGEMENT VARIABLES
    int timeAllocated; // time in milliseconds allocated for this move
    int timeBuffer;    // safety buffer to avoid timeout
    bool timeManaged;  // whether to use time management
    mutable std::mutex timeMutex;
    std::atomic<bool> searchShouldStop{false};
    std::atomic<bool> timeManagementActive{false};

    // SEARCH INSTABILITY DETECTION
    bool positionIsUnstable;
    int unstableExtensionPercent; // Additional percentage of time for unstable positions

public:
    Engine(Game &g, int depth = 3, int ttSizeMB = 64, bool useTimeManagement = false)
        : game(g), maxDepth(depth), transpositionTable(ttSizeMB), nodesSearched(0),
          timeAllocated(0), timeBuffer(100), timeManaged(useTimeManagement),
          positionIsUnstable(false), unstableExtensionPercent(50)
    {
        // Initialize PV table
        pvTable.resize(MAX_PLY);
        for (int i = 0; i < MAX_PLY; i++)
        {
            pvTable[i].clear();
        }

     // Initialize tables
        clearKillerMoves();
        clearHistoryTable();
        clearCounterMoves();
        clearEnhancedTables();
        
        // NEW: Initialize extension tracking
        totalExtensionsInPath = 0;
        for (int i = 0; i < MAX_PLY; i++) {
            extensionsUsed[i] = 0;
        }
    }

    void setTimeAllocation(int timeInMs)
    {
        timeAllocated = timeInMs;
        timeManaged = true;
    }

    // Set time for this move in milliseconds
    void setTimeForMove(int timeMs)
    {
        timeAllocated = timeMs;
        timeManaged = true;
    }

    // Enable/disable time management
    void setTimeManagement(bool enabled) { timeManaged = enabled; }

    // Set the search depth
    void setDepth(int depth) { maxDepth = depth; }

    // Set transposition table size
    void setTTSize(int sizeMB) { transpositionTable.resize(sizeMB); }

    // Calculate the best move for the current position
    Move getBestMove();

    // Clear the transposition table
    void clearTT() { transpositionTable.clear(); }

    // Get the principal variation as a string
    std::string getPVString() const;

    // Get the number of nodes searched
    long getNodesSearched() const { return nodesSearched; }

    // Reset search statistics
    void resetStats() { nodesSearched = 0; }
    // NEW: Public evaluation methods for testing
    int evaluatePieceMobility(const Board& board) const;
    int evaluateKingSafety(const Board& board) const;
    int evaluatePawnStructure(const Board& board) const;
    int evaluatePieceCoordination(const Board& board) const;
    int evaluateEndgameFactors(const Board& board) const;
    int countPieceMobility(const Board& board, Color color) const;
    int getPawnIslands(const Board& board, Color color) const;
    bool hasBishopPair(const Board& board, Color color) const;
    bool isEndgame(const Board& board) const;

private:
    // NULL MOVE PRUNING PARAMETERS
    static const int NULL_MOVE_MIN_DEPTH = 3;
    static const int NULL_MOVE_BASE_REDUCTION = 3;
    static const int NULL_MOVE_VERIFICATION_DEPTH = 2;

// LMR PARAMETERS - Enhanced
    static const int LMR_MIN_DEPTH = 3;            // Minimum depth to apply LMR
    static const int LMR_MIN_MOVE_INDEX = 4;       // Start reducing after this many moves
    static const int PV_NODE_THRESHOLD = 2;        // Different rules for PV nodes
    static const int MAX_LMR_REDUCTION = 4;        // Maximum reduction allowed
    static const int MIN_LMR_REDUCTION = 1;        // Minimum reduction
    static const double LMR_BASE_REDUCTION = 0.85; // Base reduction factor
    static const double LMR_DEPTH_FACTOR = 0.6;    // How much depth affects reduction
    static const double LMR_MOVE_FACTOR = 0.4;     // How much move index affects reduction
  static const double LMR_POSITION_FACTOR = 0.3; // Position-dependent factor

    // NEW: Pruning Integration Control
    static const bool ENABLE_NULL_MOVE_PRUNING = true;
    static const bool ENABLE_RAZORING = true;
    static const bool ENABLE_FUTILITY_PRUNING = true;
    static const bool ENABLE_LMR = true;
    static const int PRUNING_CONFLICT_THRESHOLD = 2; // Max pruning techniques per node    

    // PIECE VALUES
    static const int PAWN_VALUE = 100;
    static const int KNIGHT_VALUE = 320;
    static const int BISHOP_VALUE = 330;
    static const int ROOK_VALUE = 500;
    static const int QUEEN_VALUE = 900;
    static const int KING_VALUE = 20000;

    // NEW: EVALUATION WEIGHTS
    static const int MOBILITY_WEIGHT = 4;
    static const int KING_SAFETY_WEIGHT = 15;
    static const int PAWN_STRUCTURE_WEIGHT = 8;
    static const int PIECE_COORDINATION_WEIGHT = 6;
    static const int ENDGAME_WEIGHT = 10;

    // NEW: PAWN STRUCTURE VALUES
    static const int ISOLATED_PAWN_PENALTY = -12;
    static const int DOUBLED_PAWN_PENALTY = -15;
    static const int BACKWARD_PAWN_PENALTY = -8;
    static const int PASSED_PAWN_BONUS = 20;
    static const int PAWN_ISLAND_PENALTY = -5;

    // NEW: PIECE COORDINATION VALUES
    static const int BISHOP_PAIR_BONUS = 30;
    static const int KNIGHT_OUTPOST_BONUS = 25;
    static const int ROOK_OPEN_FILE_BONUS = 15;
    static const int ROOK_SEMI_OPEN_FILE_BONUS = 10;

    // NEW: KING SAFETY VALUES
    static const int PAWN_SHELTER_BONUS = 10;
    static const int EXPOSED_KING_PENALTY = -20;
    static const int KING_ATTACKER_PENALTY = -15;

    // NEW: MOBILITY VALUES
    static const int MOBILITY_BONUS_KNIGHT = 4;
    static const int MOBILITY_BONUS_BISHOP = 3;
    static const int MOBILITY_BONUS_ROOK = 2;
    static const int MOBILITY_BONUS_QUEEN = 1;

    // PIECE-SQUARE TABLES
    static const int pawnTable[64];
    static const int knightTable[64];
    static const int bishopTable[64];
    static const int rookTable[64];
    static const int queenTable[64];
    static const int kingMiddleGameTable[64];
    static const int kingEndGameTable[64];

    // CORE SEARCH METHODS
    Move iterativeDeepeningSearch(Board &board, int maxDepth, uint64_t hashKey);
    int pvSearch(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                 std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove);
    int alphaBeta(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                  std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove);
    int quiescenceSearch(Board &board, int alpha, int beta, uint64_t hashKey, int ply);

  // EVALUATION METHODS
    int evaluatePosition(const Board &board);
    bool isEndgame(const Board &board) const;

    // NEW: Enhanced Evaluation Methods
    int evaluatePieceMobility(const Board& board) const;
    int evaluateKingSafety(const Board& board) const;
    int evaluatePawnStructure(const Board& board) const;
    int evaluatePieceCoordination(const Board& board) const;
    int evaluateEndgameFactors(const Board& board) const;

    // NEW: Individual Evaluation Components
    int countPieceMobility(const Board& board, Color color) const;
    int evaluateKingSafetyForColor(const Board& board, Color color) const;
    int evaluatePawnsForColor(const Board& board, Color color) const;
    int evaluatePieceActivity(const Board& board, Color color) const;
    int evaluateKingActivity(const Board& board, Color color) const;

    // NEW: Pawn Structure Helpers
    bool isPawnIsolated(const Board& board, Position pawnPos) const;
    bool isPawnDoubled(const Board& board, Position pawnPos) const;
    bool isPawnBackward(const Board& board, Position pawnPos) const;
    bool isPawnPassed(const Board& board, Position pawnPos) const;
    int getPawnIslands(const Board& board, Color color) const;

    // NEW: Piece Coordination Helpers
    bool hasBishopPair(const Board& board, Color color) const;
    bool isKnightOutpost(const Board& board, Position knightPos) const;
    bool isRookOnOpenFile(const Board& board, Position rookPos) const;
    bool isRookOnSemiOpenFile(const Board& board, Position rookPos) const;

    // NEW: King Safety Helpers
    int countPawnShelter(const Board& board, Position kingPos, Color kingColor) const;
    int countKingAttackers(const Board& board, Position kingPos, Color attackerColor) const;
    int evaluateKingZone(const Board& board, Position kingPos, Color kingColor) const;

    // MOVE GENERATION METHODS
    void generateCaptureMoves(const Board& board, std::vector<Move>& captures) const;
    void generatePawnCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
    void generateKnightCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
    void generateBishopCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
    void generateRookCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
    void generateQueenCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
    void generateKingCaptures(const Board& board, Position from, std::vector<Move>& captures) const;
    void generateCheckEvasions(const Board& board, std::vector<Move>& evasions) const;
    void generatePromotions(const Board& board, std::vector<Move>& promotions) const;

    // STATIC EXCHANGE EVALUATION (SEE)
    int seeCapture(const Board &board, const Move &move) const;
    int see(const Board &board, const Position &square, Color side, int capture_value) const;
    int getPieceValue(PieceType type) const;

    // MOVE ORDERING AND SCORING
    int getMoveScore(const Move &move, const Board &board, const Move &ttMove,
                     const std::vector<Move> &pv, int ply, Color sideToMove,
                     const Move &lastMove) const;
    int getEnhancedMoveScore(const Move& move, const Board& board, const Move& ttMove,
                           int ply, Color sideToMove, const Move& lastMove) const;
    int getMVVLVAScore(PieceType attacker, PieceType victim) const;
    int getDepthAdjustment(const Move &move, const Board &board, bool isPVMove, int moveIndex) const;

    // ENHANCED: KILLER MOVE MANAGEMENT
    void storeKillerMove(const Move &move, int ply);
    void storeEnhancedKillerMove(const Move &move, int ply);
    bool isKillerMove(const Move &move, int ply) const;

    // COUNTER MOVE MANAGEMENT
    void storeCounterMove(const Move &lastMove, const Move &counterMove);
    Move getCounterMove(const Move &lastMove) const;
    void storeCountermoveHistory(const Move &lastMove, const Move &counterMove);
    Move getCountermoveHistory(const Move &lastMove) const;

    // HISTORY HEURISTIC MANAGEMENT
    void updateHistoryScore(const Move &move, int depth, Color color);
    int getHistoryScore(const Move &move, Color color) const;
    
    // Extension methods
    int calculateExtensions(const Move& move, const Board& board, int depth, int ply, 
                           bool isPVNode, bool inCheck, int moveNumber) const;
    int getCheckExtension(const Board& board, int ply) const;
   int getSingularExtension(const Board& board, const Move& move, int depth,
                            int alpha, int beta, int ply) const;

    // NEW: Futility pruning methods
    bool canUseFutilityPruning(int depth, int alpha, int beta, int eval, bool inCheck) const;
    int getFutilityMargin(int depth, const Board& board) const;
    bool canUseReverseFutilityPruning(int depth, int eval, int beta) const;
   bool canUseDeltaPruning(int eval, int alpha, const Move& move, const Board& board) const;

    // NEW: Razoring methods
    bool canUseRazoring(int depth, int alpha, int eval, bool inCheck) const;
    int getRazoringMargin(int depth, const Board& board) const;
    bool hasNearPromotionPawns(const Board& board, Color color) const;
  bool hasMaterialImbalance(const Board& board) const;

    // NEW: Pruning Integration Methods
    bool shouldAllowMultiplePruning(int depth, int ply, bool inCheck) const;
    int getPruningPriority(const std::string& pruningType, int depth, int ply) const;
    void trackPruningUsage(const std::string& pruningType, int depth, int ply) const;


    // BUTTERFLY HISTORY MANAGEMENT
    void updateButterflyHistory(const Move &move, int depth, Color color);
    int getButterflyScore(const Move &move) const;

    // PRINCIPAL VARIATION MANAGEMENT
    void storePV(int depth, const std::vector<Move> &pv);
    bool isPVMove(const Move &move, int depth, int ply) const;
    bool isPVMove(const Move &move, const std::vector<Move> &pv, int ply) const;

    // LATE MOVE REDUCTION (LMR) - Enhanced
    int calculateLMRReduction(int depth, int moveIndex, bool foundPV, bool isCapture,
                              bool isCheck, bool isKillerMove) const;
    int calculateAdvancedLMRReduction(int depth, int moveIndex, bool foundPV, bool isCapture,
                                    bool isCheck, bool isKillerMove, const Board& board, 
                                    const Move& move, int ply) const;
    bool shouldDoGradualReSearch(int lmrScore, int alpha, int beta, int depth) const;
    int getTacticalPositionBonus(const Board& board) const;
    int getKingSafetyBonus(const Board& board, const Move& move) const;

    // NULL MOVE PRUNING METHODS
    bool canUseNullMove(const Board& board, int depth, int beta, int ply) const;
    bool isZugzwangPosition(const Board& board) const;
    bool hasOnlyPawnsAndKing(const Board& board, Color color) const;
    int calculateNullMoveReduction(int depth, int staticEval, int beta) const;

    // UTILITY METHODS
    bool shouldStopSearch() const;
    void clearKillerMoves();
    void clearCounterMoves();
    void clearHistoryTable();
    void clearEnhancedTables();
    void clearSEECache();

    // NEW: Parameter Tuning Framework
    struct TuningParameter {
        std::string name;
        int* valuePtr;
        int minValue;
        int maxValue;
        int step;
        int originalValue;
    };
    
    void initializeTuningParameters();
    void runParameterTuning(const std::string& testSuite);
    bool runABTest(const std::string& parameterName, int newValue, int testGames);
    void printTuningResults() const;
    double evaluateParameterSet(const std::vector<std::string>& testPositions);
    
    // Tuning parameter storage
    std::vector<TuningParameter> tuningParameters;
    mutable std::map<std::string, double> tuningResults;
    
};
#endif // ENGINE_H