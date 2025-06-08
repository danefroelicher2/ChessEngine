#include "engine.h"
#include <chrono>
#include <limits>
#include <algorithm>
#include <sstream>

// Initialize piece-square tables
// These tables provide positional bonuses for pieces on specific squares
const int Engine::pawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5, 5, 10, 25, 25, 10, 5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0};

const int Engine::knightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};

const int Engine::bishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 5, 5, 5, 5, -10,
    -10, 0, 5, 0, 0, 5, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};

const int Engine::rookTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0};

const int Engine::queenTable[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20};

const int Engine::kingMiddleGameTable[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20, 0, 0, 0, 0, 20, 20,
    20, 30, 10, 0, 0, 10, 30, 20};

const int Engine::kingEndGameTable[64] = {
    -50, -40, -30, -20, -20, -30, -40, -50,
    -30, -20, -10, 0, 0, -10, -20, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 30, 40, 40, 30, -10, -30,
    -30, -10, 20, 30, 30, 20, -10, -30,
    -30, -30, 0, 0, 0, 0, -30, -30,
    -50, -30, -30, -30, -30, -30, -30, -50};



void Engine::clearKillerMoves()
{
    for (int ply = 0; ply < MAX_PLY; ply++)
    {
        for (int i = 0; i < 4; i++) // 4 killer slots instead of 2
        {
            killerMoves[ply][i] = Move(Position(0, 0), Position(0, 0));
        }
    }
}

// Clear the counter moves
void Engine::clearCounterMoves()
{
    for (int pieceType = 0; pieceType < 6; pieceType++)
    {
        for (int color = 0; color < 2; color++)
        {
            for (int from = 0; from < 64; from++)
            {
                for (int to = 0; to < 64; to++)
                {
                    counterMoves[pieceType][color][from][to] = Move(Position(0, 0), Position(0, 0));
                }
            }
        }
    }
}

void Engine::clearEnhancedTables()
{
    // Clear butterfly history
    for (int from = 0; from < 64; from++) {
        for (int to = 0; to < 64; to++) {
            butterflyHistory[from][to] = 0;
        }
    }
    
    // Clear countermove history
    for (int piece = 0; piece < 6; piece++) {
        for (int to = 0; to < 64; to++) {
            countermoveHistory[piece][to] = Move(Position(0, 0), Position(0, 0));
        }
    }
    
    // Clear SEE cache
    clearSEECache();
    
    // Reset null move tracking
    for (int i = 0; i < MAX_PLY; i++) {
        nullMoveAllowed[i] = true;
    }
}

// Clear SEE Cache
void Engine::clearSEECache()
{
    seeCache.clear();
}

// Generate capture moves
void Engine::generateCaptureMoves(const Board& board, std::vector<Move>& captures) const
{
    captures.clear();
    
    Color sideToMove = board.getSideToMove();
    
    // Loop through all squares to find our pieces
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position from(row, col);
            auto piece = board.getPieceAt(from);
            
            if (!piece || piece->getColor() != sideToMove) continue;
            
            // Generate captures for each piece type
            switch (piece->getType()) {
                case PieceType::PAWN:
                    generatePawnCaptures(board, from, captures);
                    break;
                    
                case PieceType::KNIGHT:
                    generateKnightCaptures(board, from, captures);
                    break;
                    
                case PieceType::BISHOP:
                    generateBishopCaptures(board, from, captures);
                    break;
                    
                case PieceType::ROOK:
                    generateRookCaptures(board, from, captures);
                    break;
                    
                case PieceType::QUEEN:
                    generateQueenCaptures(board, from, captures);
                    break;
                    
                case PieceType::KING:
                    generateKingCaptures(board, from, captures);
                    break;
                    
                default:
                    break;
            }
        }
    }
}

void Engine::generatePawnCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::PAWN) return;
    
    Color color = piece->getColor();
    int direction = (color == Color::WHITE) ? 1 : -1;
    
    // Diagonal captures
    for (int dCol : {-1, 1}) {
        Position to(from.row + direction, from.col + dCol);
        
        if (!to.isValid()) continue;
        
        auto target = board.getPieceAt(to);
        
        // Regular capture
        if (target && target->getColor() != color) {
            // Check for promotion
            if (to.row == 0 || to.row == 7) {
                captures.emplace_back(from, to, PieceType::QUEEN);
                captures.emplace_back(from, to, PieceType::ROOK);
                captures.emplace_back(from, to, PieceType::BISHOP);
                captures.emplace_back(from, to, PieceType::KNIGHT);
            } else {
                captures.emplace_back(from, to);
            }
        }
        // En passant capture
        else if (to == board.getEnPassantTarget()) {
            captures.emplace_back(from, to);
        }
    }
}

void Engine::generateKnightCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::KNIGHT) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> knightMoves = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    for (const auto& offset : knightMoves) {
        Position to(from.row + offset.first, from.col + offset.second);
        
        if (!to.isValid()) continue;
        
        auto target = board.getPieceAt(to);
        if (target && target->getColor() != color) {
            captures.emplace_back(from, to);
        }
    }
}

void Engine::generateBishopCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::BISHOP) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> directions = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    
    for (const auto& dir : directions) {
        for (int distance = 1; distance < 8; distance++) {
            Position to(from.row + dir.first * distance, from.col + dir.second * distance);
            
            if (!to.isValid()) break;
            
            auto target = board.getPieceAt(to);
            if (target) {
                if (target->getColor() != color) {
                    captures.emplace_back(from, to);
                }
                break; // Can't go further
            }
        }
    }
}

void Engine::generateRookCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::ROOK) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (const auto& dir : directions) {
        for (int distance = 1; distance < 8; distance++) {
            Position to(from.row + dir.first * distance, from.col + dir.second * distance);
            
            if (!to.isValid()) break;
            
            auto target = board.getPieceAt(to);
            if (target) {
                if (target->getColor() != color) {
                    captures.emplace_back(from, to);
                }
                break; // Can't go further
            }
        }
    }
}

void Engine::generateQueenCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    // Queen moves like both rook and bishop
    generateRookCaptures(board, from, captures);
    generateBishopCaptures(board, from, captures);
}

void Engine::generateKingCaptures(const Board& board, Position from, std::vector<Move>& captures) const
{
    auto piece = board.getPieceAt(from);
    if (!piece || piece->getType() != PieceType::KING) return;
    
    Color color = piece->getColor();
    const std::vector<std::pair<int, int>> directions = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    for (const auto& dir : directions) {
        Position to(from.row + dir.first, from.col + dir.second);
        
        if (!to.isValid()) continue;
        
        auto target = board.getPieceAt(to);
        if (target && target->getColor() != color) {
            captures.emplace_back(from, to);
        }
    }
}

void Engine::generateCheckEvasions(const Board& board, std::vector<Move>& evasions) const
{
    evasions.clear();
    
    Color sideToMove = board.getSideToMove();
    
    // Find the king's actual position
    Position kingPos;
    bool foundKing = false;
    for (int row = 0; row < 8 && !foundKing; row++) {
        for (int col = 0; col < 8 && !foundKing; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::KING && piece->getColor() == sideToMove) {
                kingPos = Position(row, col);
                foundKing = true;
            }
        }
    }
    
    if (!foundKing) return;
    
    // 1. Generate king moves (always try to move the king out of check)
    const std::vector<std::pair<int, int>> kingMoves = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},           {0, 1},
        {1, -1},  {1, 0},  {1, 1}
    };
    
    for (const auto& offset : kingMoves) {
        Position to(kingPos.row + offset.first, kingPos.col + offset.second);
        if (to.isValid()) {
            auto target = board.getPieceAt(to);
            if (!target || target->getColor() != sideToMove) {
                evasions.emplace_back(kingPos, to);
            }
        }
    }
    
    // 2. Try to block or capture the attacking piece
    // For now, generate all legal moves as fallback (this could be optimized further)
    auto allMoves = board.generateLegalMoves();
    for (const auto& move : allMoves) {
        // Skip king moves (already added above)
        auto piece = board.getPieceAt(move.from);
        if (piece && piece->getType() != PieceType::KING) {
            evasions.push_back(move);
        }
    }
}

void Engine::generatePromotions(const Board& board, std::vector<Move>& promotions) const
{
    Color sideToMove = board.getSideToMove();
    int promotionRank = (sideToMove == Color::WHITE) ? 6 : 1; // 7th rank for promotion
    
    for (int col = 0; col < 8; col++) {
        Position pawnPos(promotionRank, col);
        auto piece = board.getPieceAt(pawnPos);
        
        if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == sideToMove) {
            int direction = (sideToMove == Color::WHITE) ? 1 : -1;
            Position to(pawnPos.row + direction, pawnPos.col);
            
            // Forward promotion
            if (to.isValid() && !board.getPieceAt(to)) {
                promotions.emplace_back(pawnPos, to, PieceType::QUEEN);
                // Only add queen promotions in quiescence (most important)
            }
            
            // Capture promotions
            for (int dCol : {-1, 1}) {
                Position captureTo(pawnPos.row + direction, pawnPos.col + dCol);
                if (captureTo.isValid()) {
                    auto target = board.getPieceAt(captureTo);
                    if (target && target->getColor() != sideToMove) {
                        promotions.emplace_back(pawnPos, captureTo, PieceType::QUEEN);
                    }
                }
            }
        }
    }
}

// Clear the history table
void Engine::clearHistoryTable()
{
    for (int color = 0; color < 2; color++)
    {
        for (int from = 0; from < 64; from++)
        {
            for (int to = 0; to < 64; to++)
            {
                historyTable[color][from][to] = 0;
            }
        }
    }
}

// Fixed LMR reduction calculation
int Engine::calculateLMRReduction(int depth, int moveIndex, bool foundPV, bool isCapture,
                                  bool isCheck, bool isKillerMove) const
{
    // Don't reduce if depth is too shallow
    if (depth < LMR_MIN_DEPTH)
    {
        return 0;
    }

    // Don't reduce the first few moves
    if (moveIndex < LMR_MIN_MOVE_INDEX)
    {
        return 0;
    }

    // Don't reduce the PV move (first move when foundPV is false)
    if (!foundPV)
    {
        return 0; // This is the PV move, don't reduce
    }

    // Don't reduce captures, checks, or killer moves
    if (isCapture || isCheck || isKillerMove)
    {
        return 0;
    }

    // Calculate base reduction using logarithmic formula
    double logDepth = std::log(static_cast<double>(depth));
    double logMoveIndex = std::log(static_cast<double>(moveIndex + 1));

    double reduction = LMR_BASE_REDUCTION +
                       (logDepth * LMR_DEPTH_FACTOR) +
                       (logMoveIndex * LMR_MOVE_FACTOR);

    // Convert to integer and clamp between 1 and 3
    int intReduction = static_cast<int>(std::round(reduction));
    intReduction = std::max(1, std::min(3, intReduction));

    return intReduction;
}

// NEW: Enhanced LMR implementation
int Engine::calculateAdvancedLMRReduction(int depth, int moveIndex, bool foundPV, bool isCapture,
                                         bool isCheck, bool isKillerMove, const Board& board, 
                                         const Move& move, int ply) const
{
    // Don't reduce if depth is too shallow
    if (depth < LMR_MIN_DEPTH) {
        return 0;
    }

    // Don't reduce the first few moves
    if (moveIndex < LMR_MIN_MOVE_INDEX) {
        return 0;
    }

    // Don't reduce the PV move
    if (!foundPV) {
        return 0;
    }

    // Don't reduce captures, checks, or killer moves
    if (isCapture || isCheck || isKillerMove) {
        return 0;
    }

    // Calculate multi-factor reduction
    double logDepth = std::log(static_cast<double>(depth));
    double logMoveIndex = std::log(static_cast<double>(moveIndex + 1));

    // Base reduction calculation
    double reduction = LMR_BASE_REDUCTION +
                      (logDepth * LMR_DEPTH_FACTOR) +
                      (logMoveIndex * LMR_MOVE_FACTOR);

    // Position-aware adjustments
    int tacticalBonus = getTacticalPositionBonus(board);
    int kingSafetyBonus = getKingSafetyBonus(board, move);
    
    double positionFactor = (tacticalBonus + kingSafetyBonus) * LMR_POSITION_FACTOR;
    
    // Reduce less in tactical positions
    if (tacticalBonus > 0) {
        reduction *= 0.7; // 30% less reduction
    }
    
    // Reduce more in quiet endgames
    if (isEndgame(board) && tacticalBonus == 0) {
        reduction *= 1.3; // 30% more reduction
    }
    
    // Time pressure - reduce more aggressively
    if (timeManaged && shouldStopSearch()) {
        reduction *= 1.2;
    }

    // Apply position factor
    reduction += positionFactor;

    // Convert to integer and apply limits
    int intReduction = static_cast<int>(std::round(reduction));
    intReduction = std::max(MIN_LMR_REDUCTION, std::min(MAX_LMR_REDUCTION, intReduction));

    return intReduction;
}

bool Engine::shouldDoGradualReSearch(int lmrScore, int alpha, int beta, int depth) const
{
    // Always re-search if we beat alpha
    if (lmrScore > alpha) {
        return true;
    }
    
    // For deep searches, be more conservative about re-searching
    if (depth >= 8 && lmrScore < alpha - 100) {
        return false; // Don't re-search if score is much worse
    }
    
    // Re-search if close to alpha (might be a tactical shot)
    return (lmrScore >= alpha - 50);
}

int Engine::getTacticalPositionBonus(const Board& board) const
{
    int bonus = 0;
    
    // Count available captures
    std::vector<Move> captures;
    generateCaptureMoves(board, captures);
    
    if (captures.size() > 3) {
        bonus += 2; // Many captures = tactical
    }
    
    // Check if king is under pressure
    if (board.isInCheck()) {
        bonus += 3; // In check = very tactical
    }
    
    // Look for pieces attacking near the king
    Color sideToMove = board.getSideToMove();
    Color opponentColor = (sideToMove == Color::WHITE) ? Color::BLACK : Color::WHITE;
    
    // Find opponent king
    Position opponentKingPos;
    bool foundKing = false;
    for (int row = 0; row < 8 && !foundKing; row++) {
        for (int col = 0; col < 8 && !foundKing; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::KING && piece->getColor() == opponentColor) {
                opponentKingPos = Position(row, col);
                foundKing = true;
            }
        }
    }
    
    if (foundKing) {
        // Count pieces attacking near opponent king
        int attacksNearKing = 0;
        for (int dRow = -2; dRow <= 2; dRow++) {
            for (int dCol = -2; dCol <= 2; dCol++) {
                Position nearKing(opponentKingPos.row + dRow, opponentKingPos.col + dCol);
                if (nearKing.isValid() && board.isSquareAttacked(nearKing, sideToMove)) {
                    attacksNearKing++;
                }
            }
        }
        
        if (attacksNearKing > 3) {
            bonus += 2; // King under pressure
        }
    }
    
    return bonus;
}

int Engine::getKingSafetyBonus(const Board& board, const Move& move) const
{
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece) return 0;
    
    // Moving pieces away from our king = dangerous
    Color ourColor = movingPiece->getColor();
    
    // Find our king
    Position ourKingPos;
    bool foundKing = false;
    for (int row = 0; row < 8 && !foundKing; row++) {
        for (int col = 0; col < 8 && !foundKing; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::KING && piece->getColor() == ourColor) {
                ourKingPos = Position(row, col);
                foundKing = true;
            }
        }
    }
    
    if (!foundKing) return 0;
    
    // Calculate distance from king before and after move
    int distBefore = abs(move.from.row - ourKingPos.row) + abs(move.from.col - ourKingPos.col);
    int distAfter = abs(move.to.row - ourKingPos.row) + abs(move.to.col - ourKingPos.col);
    
    // Moving away from king in critical positions = reduce less
    if (distAfter > distBefore && distBefore <= 3) {
        return 2; // Don't reduce moves away from king as much
    }
    
    return 0;
}


// Extension calculation methods
int Engine::calculateExtensions(const Move& move, const Board& board, int depth, int ply, 
                                bool isPVNode, bool inCheck, int moveNumber) const
{
    // Prevent extension explosion
    if (totalExtensionsInPath >= MAX_TOTAL_EXTENSIONS || 
        extensionsUsed[ply] >= MAX_EXTENSIONS_PER_PLY) {
        return 0;
    }
    
    int totalExtension = 0;
    
    // 1. Check Extension (refined)
    if (inCheck) {
        int checkExt = getCheckExtension(board, ply);
        totalExtension = std::max(totalExtension, checkExt);
    }
    
    // 2. Singular Move Extension (new)
    if (isPVNode && depth >= 6 && moveNumber == 0) {
        int singularExt = getSingularExtension(board, move, depth, -99999, 99999, ply);
        totalExtension = std::max(totalExtension, singularExt);
    }
    
    // 3. Recapture Extension
    if (move.to.isValid()) {
        auto capturedPiece = board.getPieceAt(move.to);
        if (capturedPiece && totalExtensionsInPath < MAX_TOTAL_EXTENSIONS / 2) {
            totalExtension = std::max(totalExtension, 1);
        }
    }
    
    // 4. Pawn Push to 7th Rank Extension
    auto movingPiece = board.getPieceAt(move.from);
    if (movingPiece && movingPiece->getType() == PieceType::PAWN) {
        Color pawnColor = movingPiece->getColor();
        int promotionRank = (pawnColor == Color::WHITE) ? 6 : 1;
        
        if (move.to.row == promotionRank && totalExtensionsInPath < MAX_TOTAL_EXTENSIONS / 2) {
            totalExtension = std::max(totalExtension, 1);
        }
    }
    
    return std::min(totalExtension, 1);
}

int Engine::getCheckExtension(const Board& board, int ply) const
{
    if (ply > 20 || extensionsUsed[ply] > 0) {
        return 0;
    }
    
    auto legalMoves = board.generateLegalMoves();
    
    if (legalMoves.size() == 1) {
        return 1;
    }
    
    if (legalMoves.size() <= 3 && totalExtensionsInPath < MAX_TOTAL_EXTENSIONS / 2) {
        return 1;
    }
    
    return 0;
}

int Engine::getSingularExtension(const Board& board, const Move& move, int depth,
                                int alpha, int beta, int ply) const
{
    if (totalExtensionsInPath >= MAX_TOTAL_EXTENSIONS / 2) {
        return 0;
    }
    
    auto legalMoves = board.generateLegalMoves();
    
    if (legalMoves.size() == 1) {
        return 1;
    }
    
    if (legalMoves.size() <= 3) {
        return 1;
    }
    
    return 0;
}

// Store PV at a specific depth

// Store PV at a specific depth
void Engine::storePV(int depth, const std::vector<Move> &pv)
{
    if (depth >= 0 && depth < MAX_PLY)
    {
        pvTable[depth] = pv;
    }
}

// Check if a move is in the PV at a specific depth and ply
bool Engine::isPVMove(const Move &move, int depth, int ply) const
{
    if (depth < 0 || depth >= MAX_PLY || ply >= static_cast<int>(pvTable[depth].size()))
    {
        return false;
    }

    const Move &pvMove = pvTable[depth][ply];
    return (pvMove.from.row == move.from.row &&
            pvMove.from.col == move.from.col &&
            pvMove.to.row == move.to.row &&
            pvMove.to.col == move.to.col);
}

// Get the best move for the current position
Move Engine::getBestMove()
{
    // Reset search statistics
    resetStats();
    // Reset search flags at start of new search
    searchShouldStop.store(false);
    timeManagementActive.store(timeManaged);
    searchStartTime = std::chrono::high_resolution_clock::now();

    // Get a copy of the board
    Board board = game.getBoard();

    // Increment transposition table age
    transpositionTable.incrementAge();

    // Initialize Zobrist hashing
    uint64_t hashKey = zobristHasher.generateHashKey(board);

   // Reset null move tracking for new search
    for (int i = 0; i < MAX_PLY; i++) {
        nullMoveAllowed[i] = true;
    }

   // NEW: Reset extension counters
    totalExtensionsInPath = 0;
    for (int i = 0; i < MAX_PLY; i++) {
        extensionsUsed[i] = 0;
    }

    // NEW: Reset pruning tracking
    for (int i = 0; i < MAX_PLY; i++) {
        pruningUsedAtPly[i] = 0;
    }
    for (int i = 0; i < 5; i++) {
        pruningStats[i] = 0;
    }

    // Use iterative deepening to find the best move
    return iterativeDeepeningSearch(board, maxDepth, hashKey);
}

// Iterative deepening search
Move Engine::iterativeDeepeningSearch(Board &board, int maxDepth, uint64_t hashKey)
{
    principalVariation.clear();
    Move bestMove(Position(0, 0), Position(0, 0));
    Move previousBestMove(Position(0, 0), Position(0, 0));
    int bestScore = 0;
    int previousScore = 0;

    // For time management
    long nodesPrevious = 0;

    // For aspiration windows
    int windowSize = 50;

    // For instability detection
    int bestMoveChanges = 0;
    int scoreSwings = 0;

    // Initialize PV table for each depth
    for (int i = 0; i < MAX_PLY; i++)
    {
        pvTable[i].clear();
    }

    // Iterative deepening loop
    for (int depth = 1; depth <= maxDepth && !searchShouldStop.load(); depth++)
    {
        std::vector<Move> pv;

        // Record nodes before this iteration
        nodesPrevious = nodesSearched;

        // Store previous iteration's results
        previousBestMove = bestMove;
        previousScore = bestScore;

        // Color is set to true for maximizing player (WHITE), false for minimizing player (BLACK)
        bool maximizingPlayer = board.getSideToMove() == Color::WHITE;

        int alpha, beta, delta = windowSize;
        int score;

        // For depth 1, use full window
        if (depth == 1)
        {
            alpha = -100000;
            beta = 100000;
           score = pvSearch(board, depth, alpha, beta, maximizingPlayer, pv, hashKey, 0, Move(Position(0, 0), Position(0, 0)));
        }
        else
        {
            // Use aspiration windows for deeper searches
            alpha = bestScore - delta;
            beta = bestScore + delta;

            // Try with narrow window first
            while (true)
            {
               score = pvSearch(board, depth, alpha, beta, maximizingPlayer, pv, hashKey, 0, Move(Position(0, 0), Position(0, 0)));

                // If the score falls within our window, we're good
                if (score > alpha && score < beta)
                {
                    break;
                }

                // If we failed low (score <= alpha), widen the window
                if (score <= alpha)
                {
                    alpha = std::max(-100000, alpha - delta);
                    delta *= 2; // Increase window size
                }
                // If we failed high (score >= beta), widen the window
                else if (score >= beta)
                {
                    beta = std::min(100000, beta + delta);
                    delta *= 2; // Increase window size
                }

                // If window is already full, break
                if (alpha <= -99000 && beta >= 99000)
                {
                    break;
                }
            }
        }

        // Store the best move and score if we got valid results
        if (!pv.empty())
        {
            bestMove = pv[0];
            bestScore = score;
            principalVariation = pv;

            // Store this iteration's PV
            storePV(depth, pv);
        }

        // Instability detection
        if (depth >= 2)
        {
            // Check if best move changed
            if (bestMove.from.row != previousBestMove.from.row ||
                bestMove.from.col != previousBestMove.from.col ||
                bestMove.to.row != previousBestMove.to.row ||
                bestMove.to.col != previousBestMove.to.col)
            {
                bestMoveChanges++;
            }

            // Check for significant evaluation swings
            const int SCORE_SWING_THRESHOLD = 50; // Centipawns
            if (std::abs(bestScore - previousScore) > SCORE_SWING_THRESHOLD)
            {
                scoreSwings++;
            }

            // Determine if position is unstable
            bool isUnstable = (bestMoveChanges >= 2 || scoreSwings >= 1) && depth >= 3;

            if (isUnstable && !positionIsUnstable)
            {
                positionIsUnstable = true;
            }
        }

        // Log the progress
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - searchStartTime);

        // Nodes for this iteration
        long nodesThisIteration = nodesSearched - nodesPrevious;

        std::cout << "Depth: " << depth
                  << ", Score: " << score
                  << ", Nodes: " << nodesSearched
                  << ", Time: " << duration.count() << "ms";

        if (duration.count() > 0)
        {
            std::cout << ", NPS: " << static_cast<long>(nodesSearched * 1000.0 / duration.count());
        }

        std::cout << ", PV: " << getPVString() << std::endl;

        if (timeManaged && timeAllocated > 0)
        {
            int timeUsed = duration.count();
            
            // Dynamic buffer calculation
            int dynamicBuffer = std::max(50, std::min(500, timeAllocated * 5 / 100));

            // Calculate adjusted time allocation based on position stability
            int adjustedTimeAllocation = timeAllocated;
            if (positionIsUnstable)
            {
                adjustedTimeAllocation += (timeAllocated * unstableExtensionPercent) / 100;
            }

            // Stop if we've used most of our time
            if (timeUsed >= adjustedTimeAllocation - dynamicBuffer)
            {
                break;
            }

            // Improved time estimation for next iteration
            if (nodesThisIteration > 0 && depth > 1)
            {
                // More conservative branching factor for better time estimation
                double branchingFactor = (depth <= 4) ? 3.5 : 4.5;
                long estimatedNodesNext = static_cast<long>(nodesThisIteration * branchingFactor);
                double estimatedTimeNext = (double)timeUsed * estimatedNodesNext / nodesThisIteration;

                // Be more conservative with time estimation
                int conservativeBuffer = dynamicBuffer + (dynamicBuffer / 2);
                
                if (timeUsed + estimatedTimeNext + conservativeBuffer > adjustedTimeAllocation)
                {
                    break;
                }
            }
        }
    }

    return bestMove;
}

// Get the principal variation as a string
std::string Engine::getPVString() const
{
    std::stringstream ss;

    for (size_t i = 0; i < principalVariation.size(); i++)
    {
        ss << principalVariation[i].toString() << " ";
    }

    return ss.str();
}

// Get depth adjustment for move ordering and late move reductions
int Engine::getDepthAdjustment(const Move &move, const Board &board, bool isPVMove, int moveIndex) const
{
    // For PV moves, no reduction
    if (isPVMove)
    {
        return 0;
    }

    // Losing captures shouldn't get full depth
    auto capturedPiece = board.getPieceAt(move.to);
    if (capturedPiece && seeCapture(board, move) < 0)
    {
        return -1;
    }

    // First few moves should get full depth
    if (moveIndex < 3)
    {
        return 0;
    }

    // Late moves get progressively less depth
    int reduction = 0;

    // Base reduction on move index
    if (moveIndex >= 3)
    {
        reduction = 1;
    }
    if (moveIndex >= 6)
    {
        reduction = 2;
    }
    if (moveIndex >= 12)
    {
        reduction = 3;
    }

    // Don't reduce too much at low depths
    reduction = std::min(reduction, 2);

    return -reduction;
}

// Static Exchange Evaluation (SEE)
int Engine::seeCapture(const Board &board, const Move &move) const
{
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece)
    {
        return 0; // Invalid move - no piece at source
    }

    auto capturedPiece = board.getPieceAt(move.to);
    if (!capturedPiece)
    {
        // Check for en passant
        if (movingPiece->getType() == PieceType::PAWN &&
            move.to == board.getEnPassantTarget())
        {
            return PAWN_VALUE; // En passant captures a pawn
        }
        return 0; // Not a capture
    }

    // Use SEE cache for performance
    uint64_t moveKey = ((uint64_t)move.from.row << 24) | 
                       ((uint64_t)move.from.col << 16) | 
                       ((uint64_t)move.to.row << 8) | 
                       ((uint64_t)move.to.col);
    
    auto it = seeCache.find(moveKey);
    if (it != seeCache.end()) {
        return it->second;
    }

    // Create a temporary board with the capture made
    Board tempBoard = board;
    tempBoard.setPieceAt(move.from, nullptr);
    tempBoard.setPieceAt(move.to, movingPiece);

    int captureValue = getPieceValue(capturedPiece->getType());
    int attackerValue = getPieceValue(movingPiece->getType());

    // Calculate what happens if the opponent recaptures
    int opponentResponse = see(tempBoard, move.to, movingPiece->getColor(), attackerValue);

    int result = captureValue - opponentResponse;
    
    // Cache the result
    seeCache[moveKey] = result;
    
    return result;
}

// Static Exchange Evaluation - simulates a sequence of captures on a square
int Engine::see(const Board &board, const Position &square, Color side, int captureValue) const
{
    // Find all attackers of the given square, sorted by piece value (least valuable first)
    std::vector<std::pair<int, Position>> attackers;

    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);

            if (piece && piece->getColor() != side)
            {
                // Check if this piece can attack the target square
                auto moves = piece->getLegalMoves(board);

                for (const auto &move : moves)
                {
                    if (move.to == square)
                    {
                        int pieceValue = getPieceValue(piece->getType());
                        attackers.emplace_back(pieceValue, pos);
                        break; // Only need to know it can attack, not how many ways
                    }
                }
            }
        }
    }

    // If no attackers, the previous capture stands
    if (attackers.empty())
    {
        return 0;
    }

    // Sort attackers by piece value (least valuable first for optimal play)
    std::sort(attackers.begin(), attackers.end());

    // Take the least valuable attacker
    int attackerValue = attackers[0].first;

    // Create a temporary board to simulate the capture
    Board tempBoard = board;

    // Remove the attacking piece and place it on the target square
    tempBoard.setPieceAt(attackers[0].second, nullptr);
    auto attackingPiece = board.getPieceAt(attackers[0].second);
    tempBoard.setPieceAt(square, attackingPiece);

    // Recursively calculate the score if the opponent recaptures
    // Note: we flip the side and negate the result
    int opponentResponse = see(tempBoard, square, (side == Color::WHITE) ? Color::BLACK : Color::WHITE, attackerValue);

    // The score is: what we capture minus what the opponent gets back
    return std::max(0, captureValue - opponentResponse);
}

// Get the approximate value of a piece for SEE
int Engine::getPieceValue(PieceType type) const
{
    switch (type)
    {
    case PieceType::PAWN:
        return PAWN_VALUE;
    case PieceType::KNIGHT:
        return KNIGHT_VALUE;
    case PieceType::BISHOP:
        return BISHOP_VALUE;
    case PieceType::ROOK:
        return ROOK_VALUE;
    case PieceType::QUEEN:
        return QUEEN_VALUE;
    case PieceType::KING:
        return KING_VALUE;
    default:
        return 0;
    }
}

// MVV-LVA (Most Valuable Victim - Least Valuable Aggressor) scoring for move ordering
int Engine::getMVVLVAScore(PieceType attacker, PieceType victim) const
{
    // A simple 6x6 matrix for MVV-LVA
    // Victim (Most Valuable) on top, Attacker (Least Valuable) on left side
    static const int mvvLvaScores[6][6] = {
        // Pawn, Knight, Bishop, Rook, Queen, King (victim)
        {105, 205, 305, 405, 505, 605}, // Pawn (attacker)
        {104, 204, 304, 404, 504, 604}, // Knight
        {103, 203, 303, 403, 503, 603}, // Bishop
        {102, 202, 302, 402, 502, 602}, // Rook
        {101, 201, 301, 401, 501, 601}, // Queen
        {100, 200, 300, 400, 500, 600}  // King
    };

    return mvvLvaScores[static_cast<int>(attacker)][static_cast<int>(victim)];
}

// Store Enhanced Killer Move
void Engine::storeEnhancedKillerMove(const Move &move, int ply)
{
    // Comprehensive bounds checking
    if (ply < 0 || ply >= MAX_PLY) {
        return;
    }
    
    // Additional safety check for move validity
    if (!move.from.isValid() || !move.to.isValid()) {
        return;
    }

    // Don't store if it's already the first killer move
    if (killerMoves[ply][0].from.row == move.from.row &&
        killerMoves[ply][0].from.col == move.from.col &&
        killerMoves[ply][0].to.row == move.to.row &&
        killerMoves[ply][0].to.col == move.to.col)
    {
        return;
    }

    // Shift killer moves to make room for the new one (4 slots)
    for (int i = 3; i > 0; i--) {
        killerMoves[ply][i] = killerMoves[ply][i-1];
    }
    killerMoves[ply][0] = move;
}

// Store killer move
void Engine::storeKillerMove(const Move &move, int ply)
{
    storeEnhancedKillerMove(move, ply);
}

// Check if move is killer move
bool Engine::isKillerMove(const Move &move, int ply) const
{
    // Comprehensive bounds checking
    if (ply < 0 || ply >= MAX_PLY) {
        return false;
    }
    
    // Additional safety check for move validity
    if (!move.from.isValid() || !move.to.isValid()) {
        return false;
    }

    // Check all 4 killer move slots
    for (int i = 0; i < 4; i++) {
        if (killerMoves[ply][i].from.row == move.from.row &&
            killerMoves[ply][i].from.col == move.from.col &&
            killerMoves[ply][i].to.row == move.to.row &&
            killerMoves[ply][i].to.col == move.to.col) {
            return true;
        }
    }
    return false;
}

// Store counter move
void Engine::storeCounterMove(const Move &lastMove, const Move &counterMove)
{
    if (!lastMove.from.isValid() || !lastMove.to.isValid())
        return;

    // Get the current board state AFTER the move was made
    auto pieceAtDestination = game.getBoard().getPieceAt(lastMove.to);
    if (!pieceAtDestination)
        return;

    // The counter move should be indexed by the OPPONENT's move
    int opponentPieceType = static_cast<int>(pieceAtDestination->getType());
    int opponentColor = (pieceAtDestination->getColor() == Color::WHITE) ? 0 : 1;
    int fromIdx = lastMove.from.row * 8 + lastMove.from.col;
    int toIdx = lastMove.to.row * 8 + lastMove.to.col;

    // Store the counter move
    counterMoves[opponentPieceType][opponentColor][fromIdx][toIdx] = counterMove;
}

// Get counter move
Move Engine::getCounterMove(const Move &lastMove) const
{
    if (!lastMove.from.isValid() || !lastMove.to.isValid())
        return Move(Position(0, 0), Position(0, 0));

    // Get the piece that made the last move (opponent's piece)
    auto piece = game.getBoard().getPieceAt(lastMove.to);
    if (!piece)
        return Move(Position(0, 0), Position(0, 0));

    // Use the same indexing as storage
    int pieceType = static_cast<int>(piece->getType());
    int color = (piece->getColor() == Color::WHITE) ? 0 : 1;
    int fromIdx = lastMove.from.row * 8 + lastMove.from.col;
    int toIdx = lastMove.to.row * 8 + lastMove.to.col;

    return counterMoves[pieceType][color][fromIdx][toIdx];
}

// Enhanced Move Ordering Methods
void Engine::updateButterflyHistory(const Move &move, int depth, Color color)
{
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;
    
    // Calculate bonus based on depth - but cap it to prevent overflow
    int bonus = std::min(depth * depth, 512);
    
    // Check for potential overflow BEFORE adding
    const int MAX_BUTTERFLY_SCORE = 8192;
    const int OVERFLOW_THRESHOLD = MAX_BUTTERFLY_SCORE - 1024;
    
    // If we're approaching overflow, scale down ALL entries first
    if (butterflyHistory[fromIdx][toIdx] > OVERFLOW_THRESHOLD)
    {
        for (int from = 0; from < 64; from++)
        {
            for (int to = 0; to < 64; to++)
            {
                butterflyHistory[from][to] = (butterflyHistory[from][to] * 3) / 4;
            }
        }
    }
    
    // Now safely add the bonus
    butterflyHistory[fromIdx][toIdx] += bonus;
    
    // Aging of other moves
    const int AGING_DENOMINATOR = 64;
    
    for (int from = 0; from < 64; from++)
    {
        for (int to = 0; to < 64; to++)
        {
            if (from != fromIdx || to != toIdx)
            {
                butterflyHistory[from][to] -= butterflyHistory[from][to] / AGING_DENOMINATOR;
                if (butterflyHistory[from][to] < 0)
                {
                    butterflyHistory[from][to] = 0;
                }
            }
        }
    }
    
    // Final safety check
    if (butterflyHistory[fromIdx][toIdx] > MAX_BUTTERFLY_SCORE)
    {
        butterflyHistory[fromIdx][toIdx] = MAX_BUTTERFLY_SCORE;
    }
}

void Engine::storeCountermoveHistory(const Move &lastMove, const Move &counterMove)
{
    if (!lastMove.to.isValid() || !counterMove.from.isValid())
        return;
    
    auto lastPiece = game.getBoard().getPieceAt(lastMove.to);
    if (!lastPiece)
        return;
    
    int pieceType = static_cast<int>(lastPiece->getType());
    int toSquare = lastMove.to.row * 8 + lastMove.to.col;
    
    countermoveHistory[pieceType][toSquare] = counterMove;
}

Move Engine::getCountermoveHistory(const Move &lastMove) const
{
    if (!lastMove.to.isValid())
        return Move(Position(0, 0), Position(0, 0));
    
    auto lastPiece = game.getBoard().getPieceAt(lastMove.to);
    if (!lastPiece)
        return Move(Position(0, 0), Position(0, 0));
    
    int pieceType = static_cast<int>(lastPiece->getType());
    int toSquare = lastMove.to.row * 8 + lastMove.to.col;
    
    return countermoveHistory[pieceType][toSquare];
}

int Engine::getButterflyScore(const Move &move) const
{
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;
    return butterflyHistory[fromIdx][toIdx];
}

// Enhanced Move Scoring
int Engine::getEnhancedMoveScore(const Move& move, const Board& board, const Move& ttMove,
                                int ply, Color sideToMove, const Move& lastMove) const
{
    // Check for valid moving piece first
    auto movingPiece = board.getPieceAt(move.from);
    if (!movingPiece) {
        return -999999; // Invalid move - heavily penalize
    }

    // 1. Transposition table move (highest priority)
    if (ttMove.from.isValid() && ttMove.to.isValid() &&
        ttMove.from.row == move.from.row && ttMove.from.col == move.from.col &&
        ttMove.to.row == move.to.row && ttMove.to.col == move.to.col)
    {
        return 10000000; // Highest priority
    }

    // 2. Principal variation moves from ALL depths
    for (int d = maxDepth; d >= 1; d--)
    {
        if (isPVMove(move, d, ply))
        {
            return 9000000 + d * 1000; // Very high priority, deeper searches get bonus
        }
    }

    // 3. Winning captures (positive SEE) - ordered by victim value then SEE score
    auto capturedPiece = board.getPieceAt(move.to);
    if (capturedPiece) {
        int seeScore = seeCapture(board, move);
        
        if (seeScore > 0) {
            // Good captures: prioritize by victim value, then by SEE score
            int victimValue = getPieceValue(capturedPiece->getType());
            int attackerValue = getPieceValue(movingPiece->getType());
            
            // MVV-LVA: Most Valuable Victim - Least Valuable Attacker
            int mvvLvaScore = (victimValue * 100) - (attackerValue / 10);
            
            return 8000000 + mvvLvaScore + seeScore;
        }
        // Even captures: still prioritize over non-captures but below good ones
        else if (seeScore == 0) {
            int victimValue = getPieceValue(capturedPiece->getType());
            return 7000000 + victimValue;
        }
        // Bad captures: lowest priority among captures
        else {
            return 6000000 + seeScore; // seeScore is negative here
        }
    }

    // 4. Enhanced Counter moves
    if (lastMove.from.isValid() && lastMove.to.isValid())
    {
        // Try both counter move systems
        Move counter = getCounterMove(lastMove);
        Move counterHist = getCountermoveHistory(lastMove);
        
        if ((counter.from.isValid() && counter.to.isValid() &&
            counter.from.row == move.from.row && counter.from.col == move.from.col &&
            counter.to.row == move.to.row && counter.to.col == move.to.col) ||
            (counterHist.from.isValid() && counterHist.to.isValid() &&
            counterHist.from.row == move.from.row && counterHist.from.col == move.from.col &&
            counterHist.to.row == move.to.row && counterHist.to.col == move.to.col))
        {
            return 5000000;
        }
    }

    // 5. Enhanced Killer moves (4 slots, ranked by recency)
    if (isKillerMove(move, ply))
    {
        // Give higher scores to more recent killer moves
        for (int i = 0; i < 4; i++) {
            if (ply < MAX_PLY && 
                killerMoves[ply][i].from.row == move.from.row &&
                killerMoves[ply][i].from.col == move.from.col &&
                killerMoves[ply][i].to.row == move.to.row &&
                killerMoves[ply][i].to.col == move.to.col)
            {
                return 4000000 + (4 - i) * 25; // More recent = higher score
            }
        }
    }

    // 6. Enhanced History scoring (combine traditional + butterfly)
    int traditionHistoryScore = getHistoryScore(move, sideToMove);
    int butterflyScore = getButterflyScore(move);
    int combinedHistoryScore = traditionHistoryScore + (butterflyScore / 2);
    
    // 7. Basic positional scoring for remaining moves
    int positionalScore = 0;
    
    // Bonus for moves toward the center
    int centerDistance = abs(move.to.row - 3.5) + abs(move.to.col - 3.5);
    positionalScore += (7 - centerDistance) * 10;
    
    // Bonus for advancing pawns
    if (movingPiece->getType() == PieceType::PAWN) {
        if (sideToMove == Color::WHITE) {
            positionalScore += move.to.row * 20;
        } else {
            positionalScore += (7 - move.to.row) * 20;
        }
    }
    
    return combinedHistoryScore + positionalScore;
}

void Engine::updateHistoryScore(const Move &move, int depth, Color color)
{
    int colorIdx = (color == Color::WHITE) ? 0 : 1;
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;

    // Robust history table overflow prevention
    
    // Calculate bonus based on depth - but cap it to prevent overflow
    int bonus = std::min(depth * depth, 1024); // Cap individual bonuses
    
    // Check for potential overflow BEFORE adding
    const int MAX_HISTORY_SCORE = 16384;
    const int OVERFLOW_THRESHOLD = MAX_HISTORY_SCORE - 2048; // Leave some headroom
    
    // If we're approaching overflow, scale down ALL entries first
    if (historyTable[colorIdx][fromIdx][toIdx] > OVERFLOW_THRESHOLD)
    {
        // Aggressive scaling to prevent overflow
        for (int c = 0; c < 2; c++)
        {
            for (int from = 0; from < 64; from++)
            {
                for (int to = 0; to < 64; to++)
                {
                    historyTable[c][from][to] = (historyTable[c][from][to] * 3) / 8; // Reduce by 5/8
                }
            }
        }
    }
    
    // Now safely add the bonus
    historyTable[colorIdx][fromIdx][toIdx] += bonus;
    
    // More aggressive aging of other moves to maintain relative differences
    const int AGING_DENOMINATOR = 32; // More aggressive than 16
    
    for (int from = 0; from < 64; from++)
    {
        for (int to = 0; to < 64; to++)
        {
            if (from != fromIdx || to != toIdx) // Don't age the move that just caused cutoff
            {
                // More aggressive aging: reduce by 1/32nd instead of 1/16th
                historyTable[colorIdx][from][to] -= historyTable[colorIdx][from][to] / AGING_DENOMINATOR;
                
                // Ensure we don't go negative
                if (historyTable[colorIdx][from][to] < 0)
                {
                    historyTable[colorIdx][from][to] = 0;
                }
            }
        }
    }
    
    // Final safety check - should never happen now, but just in case
    if (historyTable[colorIdx][fromIdx][toIdx] > MAX_HISTORY_SCORE)
    {
        historyTable[colorIdx][fromIdx][toIdx] = MAX_HISTORY_SCORE;
    }
}

// Get the history score for a move
int Engine::getHistoryScore(const Move &move, Color color) const
{
    int colorIdx = (color == Color::WHITE) ? 0 : 1;
    int fromIdx = move.from.row * 8 + move.from.col;
    int toIdx = move.to.row * 8 + move.to.col;

    return historyTable[colorIdx][fromIdx][toIdx];
}

// Check if a move is part of the principal variation (deprecated version for compatibility)
bool Engine::isPVMove(const Move &move, const std::vector<Move> &pv, int ply) const
{
    if (pv.size() <= static_cast<size_t>(ply))
        return false;

    const Move &pvMove = pv[ply];
    return (pvMove.from.row == move.from.row &&
            pvMove.from.col == move.from.col &&
            pvMove.to.row == move.to.row &&
            pvMove.to.col == move.to.col);
}

// Legacy getMoveScore for compatibility
int Engine::getMoveScore(const Move &move, const Board &board, const Move &ttMove,
                         const std::vector<Move> &pv, int ply, Color sideToMove,
                         const Move &lastMove) const
{
    // Redirect to enhanced version
    return getEnhancedMoveScore(move, board, ttMove, ply, sideToMove, lastMove);
}

// NULL MOVE PRUNING IMPLEMENTATION
bool Engine::canUseNullMove(const Board& board, int depth, int beta, int ply) const {
    // Basic depth requirement
    if (depth < NULL_MOVE_MIN_DEPTH) {
        return false;
    }
    
    // Check if null move is allowed at this ply
    if (ply >= MAX_PLY || !nullMoveAllowed[ply]) {
        return false;
    }
    
    // Don't use null move when in check
    if (board.isInCheck()) {
        return false;
    }
    
    // Don't use in zugzwang-prone endgames
    if (isZugzwangPosition(board)) {
        return false;
    }
    
   // Static evaluation should be >= beta (position looks good)
    int staticEval = const_cast<Engine*>(this)->evaluatePosition(board);
    if (staticEval < beta) {
        return false;
    }
    
    // Don't use null move if we have very little material
    if (hasOnlyPawnsAndKing(board, board.getSideToMove())) {
        return false;
    }
    
    return true;
}

bool Engine::isZugzwangPosition(const Board& board) const {
    Color sideToMove = board.getSideToMove();
    
    // Count non-pawn, non-king pieces for the side to move
    int majorPieces = 0;
    int minorPieces = 0;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getColor() == sideToMove) {
                switch (piece->getType()) {
                    case PieceType::QUEEN:
                    case PieceType::ROOK:
                        majorPieces++;
                        break;
                    case PieceType::BISHOP:
                    case PieceType::KNIGHT:
                        minorPieces++;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    // Zugzwang-prone: No major pieces and at most one minor piece
    return (majorPieces == 0 && minorPieces <= 1);
}

bool Engine::hasOnlyPawnsAndKing(const Board& board, Color color) const {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getColor() == color) {
                if (piece->getType() != PieceType::PAWN && piece->getType() != PieceType::KING) {
                    return false;
                }
            }
        }
    }
    return true;
}

int Engine::calculateNullMoveReduction(int depth, int staticEval, int beta) const {
    // Base reduction
    int reduction = NULL_MOVE_BASE_REDUCTION;
    
    // Adaptive reduction based on depth
    if (depth >= 6) {
        reduction += (depth / 6);
    }
    
    // Increase reduction if static eval is much better than beta
    if (staticEval >= beta + 200) {
        reduction += 1;
    }
    
    // Don't reduce too much
    reduction = std::min(reduction, depth - 1);
    
    return reduction;
}

// Quiescence search for handling captures at leaf nodes
int Engine::quiescenceSearch(Board &board, int alpha, int beta, uint64_t hashKey, int ply)
{
    // Track nodes searched
    nodesSearched++;

    // Check for search termination every ~1000 nodes
    if ((nodesSearched % 1000) == 0 && shouldStopSearch()) {
        return evaluatePosition(board);
    }

    // Maximum recursion depth check for quiescence
    if (ply >= MAX_QSEARCH_DEPTH)
        return evaluatePosition(board);

    // Stand-pat score (evaluate the current position without making any moves)
    int standPat = evaluatePosition(board);

    // Beta cutoff
    if (standPat >= beta)
        return beta;

    // Update alpha if stand-pat score is better
    if (standPat > alpha)
        alpha = standPat;

    std::vector<Move> qMoves;
    bool inCheck = board.isInCheck();

    if (inCheck) {
        // Generate only check evasions - much more efficient than all legal moves
        generateCheckEvasions(board, qMoves);
    } else {
        // Generate only captures and promotions
        generateCaptureMoves(board, qMoves);
        
        // Add queen promotions even if not captures (they're very strong)
        generatePromotions(board, qMoves);
    }

    if (qMoves.empty())
        return standPat;

    // Score and sort the moves
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto &move : qMoves)
    {
        // Score the move
        int moveScore = 0;

        auto movingPiece = board.getPieceAt(move.from);
        auto capturedPiece = board.getPieceAt(move.to);

        bool isCapture = capturedPiece != nullptr ||
                         (movingPiece && movingPiece->getType() == PieceType::PAWN &&
                          move.to == board.getEnPassantTarget());

        // Delta pruning - skip captures that can't improve alpha
        if (isCapture && !inCheck && ply > 0)
        {
            // Get the maximum possible material gain from this capture
            int captureValue = 0;
            if (capturedPiece)
            {
                captureValue = getPieceValue(capturedPiece->getType());
            }
            else if (move.to == board.getEnPassantTarget())
            {
                captureValue = PAWN_VALUE;
            }

            // Add potential promotion bonus
            int promotionBonus = 0;
            if (movingPiece && movingPiece->getType() == PieceType::PAWN &&
                (move.to.row == 0 || move.to.row == 7))
            {
                promotionBonus = QUEEN_VALUE - PAWN_VALUE;
            }

            // Delta margin - a buffer to account for positional gains
            const int DELTA_MARGIN = 200;

            // Skip if even the maximum possible gain can't improve alpha
            if (standPat + captureValue + promotionBonus + DELTA_MARGIN <= alpha)
            {
                continue; // Skip this capture - it can't improve alpha
            }

            // Additional futility pruning for bad captures
            if (capturedPiece && seeCapture(board, move) < -50)
            {
                continue; // Skip obviously bad captures
            }
        }

        if (capturedPiece)
        {
            // MVV-LVA scoring for captures
            if (movingPiece) {
                moveScore = 10000000 + getMVVLVAScore(movingPiece->getType(), capturedPiece->getType());

                // Static Exchange Evaluation (SEE)
                int seeScore = seeCapture(board, move);
                if (seeScore < 0)
                {
                    // Skip bad captures at deeper ply depths
                    if (ply > 2 && !inCheck)
                        continue;

                    // Penalize bad captures, but still consider them
                    moveScore += seeScore;
                }
            }
        }
        else if (movingPiece && movingPiece->getType() == PieceType::PAWN &&
                 move.to == board.getEnPassantTarget())
        {
            // En passant capture
            moveScore = 10000000 + getMVVLVAScore(movingPiece->getType(), PieceType::PAWN);
        }
        else
        {
            // Non-capture move (likely a check)
            // Score lower than captures but still consider
            moveScore = 1000000;
        }

        scoredMoves.push_back(std::make_pair(moveScore, move));
    }

    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const std::pair<int, Move> &a, const std::pair<int, Move> &b)
              {
                  return a.first > b.first;
              });

    // Make each move and recursively search
    for (const auto &scoredMove : scoredMoves)
    {
        const Move &move = scoredMove.second;

        // Save board state for unmaking move
        BoardState previousState;

        // Calculate new hash key BEFORE making the move
        uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

        // Make the move
        if (!board.makeMove(move, previousState))
            continue;

        // Recursively search
        int score = -quiescenceSearch(board, -beta, -alpha, newHashKey, ply + 1);

        // Unmake the move
        board.unmakeMove(move, previousState);

        // Beta cutoff
        if (score >= beta)
            return beta;

        // Update alpha
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

// Principal Variation Search (PVS) with NULL MOVE PRUNING
int Engine::pvSearch(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                     std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove)
{
    // Track nodes searched
    nodesSearched++;

    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(0, 0), Position(0, 0));
    int score;

    pv.clear();

 // 1. PRIORITY: Probe the transposition table (ALWAYS FIRST)
    Move tempTTMove(Position(0, 0), Position(0, 0));
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, tempTTMove))
    {
        return score; // Return cached result if available (but don't use TT at root)
    }

    // Reset pruning tracking for this node
    if (ply >= 0 && ply < MAX_PLY) {
        pruningUsedAtPly[ply] = 0;
    }

    // 2. PRIORITY: NULL MOVE PRUNING (Highest reduction potential)
    // Add null move pruning right after TT probe but before move generation
    if (depth >= NULL_MOVE_MIN_DEPTH && 
        ply > 0 && // Don't use at root
        canUseNullMove(board, depth, beta, ply)) {
        
        // Disable null move for the next ply to prevent double null moves
        nullMoveAllowed[ply + 1] = false;
        
        // Calculate reduction
        int staticEval = evaluatePosition(board);
        int reduction = calculateNullMoveReduction(depth, staticEval, beta);
        
        // Make null move (switch sides)
        board.switchSideToMove();
        
        // Calculate new hash key for null move
        uint64_t nullHashKey = zobristHasher.generateHashKey(board);
        
        // Search with reduced depth and negated window
        std::vector<Move> nullPV;
        int nullScore = -pvSearch(board, depth - 1 - reduction, -beta, -beta + 1, 
                                 !maximizingPlayer, nullPV, nullHashKey, ply + 1, Move(Position(0, 0), Position(0, 0)));
        
        // Unmake null move
        board.switchSideToMove();
        
        // Re-enable null move for next iteration
        nullMoveAllowed[ply + 1] = true;
        
       // If null move caused beta cutoff, prune this branch
        if (nullScore >= beta) {
            trackPruningUsage("null_move", depth, ply);
            
            // Verification search for high values to avoid zugzwang
            if (depth >= NULL_MOVE_VERIFICATION_DEPTH && nullScore >= beta + 300) {
                std::vector<Move> verifyPV;
                int verifyScore = pvSearch(board, depth - NULL_MOVE_VERIFICATION_DEPTH, 
                                         beta - 1, beta, maximizingPlayer, verifyPV, hashKey, ply, lastMove);
                if (verifyScore >= beta) {
                    return beta; // Confirmed cutoff
                }
            } else {
                return beta; // Trust the null move cutoff
            }
        }
    }

   // Check for search termination conditions
    if (ply >= MAX_PLY)
    {
        return evaluatePosition(board);
    }

    // 3. PRIORITY: RAZORING - Prune hopeless subtrees (Medium reduction)
    if (ENABLE_RAZORING && depth >= 1 && depth <= 4 && ply > 0 && 
        shouldAllowMultiplePruning(depth, ply, board.isInCheck())) {
        int staticEval = evaluatePosition(board);
        
        if (canUseRazoring(depth, alpha, staticEval, board.isInCheck())) {
            // Try reduced depth search first
            int reducedDepth = depth - 1 - (depth > 2 ? 1 : 0); // Reduce by 1-2 ply
            
            std::vector<Move> razorPV;
            int razorScore = pvSearch(board, reducedDepth, alpha - 1, alpha, maximizingPlayer, 
                                    razorPV, hashKey, ply, lastMove);
            
            // If reduced search still fails low, prune this branch
            if (razorScore < alpha) {
                // Verification search for positions that barely fail
                if (razorScore >= alpha - 100) {
                    // Close call - do verification search at original depth
                    std::vector<Move> verifyPV;
                    int verifyScore = pvSearch(board, depth - 1, alpha - 1, alpha, maximizingPlayer,
                                             verifyPV, hashKey, ply, lastMove);
                   if (verifyScore < alpha) {
                    trackPruningUsage("razoring", depth, ply);
                    return razorScore; // Confirmed pruning
                }
            } else {
                trackPruningUsage("razoring", depth, ply);
                return razorScore; // Confident pruning
            }
            }
        }
    }

    // If we've reached the maximum depth, use quiescence search
    if (depth <= 0)
    {
        return quiescenceSearch(board, alpha, beta, hashKey, ply);
    }

    // Check if we should extend the search depth
    int extension = 0;

    // 1. Check extension - extend search when in check
    if (board.isInCheck())
    {
        extension = 1;
    }

    // If the game is over, return the evaluation
    if (board.isCheckmate() || board.isStalemate())
    {
        return evaluatePosition(board);
    }

    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();

    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty())
    {
        if (board.isInCheck())
        {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        }
        else
        {
            // Stalemate
            return 0;
        }
    }

    // 2. Singular Move Extension - if only one legal move, extend
    if (legalMoves.size() == 1 && depth >= 2)
    {
        extension = std::max(extension, 1);
    }

    // Score moves using enhanced move ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto &move : legalMoves)
    {
       Move validTTMove = (ttMove.from.isValid() && ttMove.to.isValid()) ? ttMove : Move(Position(0, 0), Position(0, 0));
        int moveScore = getEnhancedMoveScore(move, board, validTTMove, ply, board.getSideToMove(), lastMove);

        // Early pruning of very bad captures
        if (depth >= 3)
        {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece)
            {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2)
                {
                    continue;
                }
            }
        }

        scoredMoves.push_back(std::make_pair(moveScore, move));
    }

    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const std::pair<int, Move> &a, const std::pair<int, Move> &b)
              {
                  return a.first > b.first;
              });

    // Clear SEE cache periodically
    if ((nodesSearched % 5000) == 0) {
        clearSEECache();
    }

    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(0, 0), Position(0, 0)) : legalMoves[0];
    bool foundPV = false;

    // This will be used to store the principal variation
    std::vector<Move> childPV;

    if (maximizingPlayer)
    {
        int maxEval = std::numeric_limits<int>::min();

        for (size_t i = 0; i < scoredMoves.size(); i++)
        {
            const Move &move = scoredMoves[i].second;

       // NEW: Futility Pruning Section
            int currentEval = evaluatePosition(board);
            bool isCapture = board.getPieceAt(move.to) != nullptr;
            
          // 4. PRIORITY: FUTILITY PRUNING (Local move skipping)
            if (ENABLE_FUTILITY_PRUNING && shouldAllowMultiplePruning(depth, ply, board.isInCheck())) {
                // 4a. Static Futility Pruning (for quiet moves)
                if (!foundPV && depth <= 3 && !isCapture && i >= 3) {
                    if (canUseFutilityPruning(depth, alpha, beta, currentEval, board.isInCheck())) {
                        trackPruningUsage("futility", depth, ply);
                        continue;
                    }
                }
                
               // 4b. Reverse Futility Pruning (stand-pat)
                if (!foundPV && depth <= 2 && !board.isInCheck()) {
                    if (canUseReverseFutilityPruning(depth, currentEval, beta)) {
                        trackPruningUsage("futility", depth, ply);
                        return currentEval;
                    }
                }
                
                // 4c. Delta Pruning for captures
                if (isCapture && canUseDeltaPruning(currentEval, alpha, move, board)) {
                    trackPruningUsage("futility", depth, ply);
                    continue;
                }
            }

            bool isPVMoveCheck = false;
            for (int d = 1; d <= maxDepth; d++)
            {
                if (isPVMove(move, d, ply))
                {
                    isPVMoveCheck = true;
                    break;
                }
            }

            // Note: isCapture already declared above in futility pruning section
            bool isKillerMoveCheck = isKillerMove(move, ply);

            // Save board state for unmaking move
            BoardState previousState;

            // Make the move
            if (!board.makeMove(move, previousState))
                continue;

            bool isCheckMove = board.isInCheck();

            // Calculate depth adjustment
            int moveExtension = extension;

            // Recapture Extension
            if (lastMove.to.isValid() && move.to == lastMove.to)
            {
                moveExtension = std::max(moveExtension, 1);
            }

            // Pawn Push Extension
            auto piece = board.getPieceAt(move.to); // Use move.to since piece is now there
            if (piece && piece->getType() == PieceType::PAWN)
            {
                int destRow = (board.getSideToMove() == Color::BLACK) ? 6 : 1; // 7th rank (flipped because we switched sides)
                if (move.to.row == destRow)
                {
                    moveExtension = std::max(moveExtension, 1);
                }
            }
// 5. PRIORITY: LMR (Per-move reduction - lowest priority)
            int lmrReduction = 0;
            if (ENABLE_LMR) {
                lmrReduction = calculateAdvancedLMRReduction(depth, i, foundPV, isCapture, isCheckMove, 
                                                           isKillerMoveCheck, board, move, ply);
                if (lmrReduction > 0) {
                    trackPruningUsage("lmr", depth, ply);
                }
            }

            // Final depth after adjustments
            int newDepth = depth - 1 + moveExtension - lmrReduction;
            newDepth = std::max(0, newDepth);

            // Calculate the new hash key after the move
            uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

            // Recursively evaluate the position
            childPV.clear();
            int eval;

            if (foundPV)
            {
                // For non-PV moves, try LMR first if applicable
                if (lmrReduction > 0)
                {
                    // Correct null window search
                    eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);

                    // Enhanced gradual re-search strategy
                    if (shouldDoGradualReSearch(eval, alpha, beta, depth))
                    {
                        if (eval > alpha) {
                            // First try with reduced reduction
                            int intermediateDepth = depth - 1 + moveExtension - std::max(1, lmrReduction / 2);
                            childPV.clear();
                            eval = -pvSearch(board, intermediateDepth, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);
                            
                            // If still good, do full depth search
                            if (eval > alpha) {
                                newDepth = depth - 1 + moveExtension; // Full depth
                                childPV.clear();
                                eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);
                            }
                        }
                    }
                }
                else
                {
                    // No reduction, do null window search
                    eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);
                }

                // If we get a fail-high, re-search with full window
                if (eval > alpha && eval < beta)
                {
                    childPV.clear();
                    eval = -pvSearch(board, newDepth, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
                }
            }
            else
            {
                // First move gets a full window search
                eval = -pvSearch(board, newDepth, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
                foundPV = true;
            }

            // Unmake the move
            board.unmakeMove(move, previousState);

            // Update the best move if this move is better
            if (eval > maxEval)
            {
                maxEval = eval;
                localBestMove = move;

                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }

            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
            {
                // Store enhanced killer moves and history
                if (!isCapture)
                {
                    storeEnhancedKillerMove(move, ply);
                    updateHistoryScore(move, depth, Color::WHITE);
                    updateButterflyHistory(move, depth, Color::WHITE);

    if (lastMove.from.isValid() && lastMove.to.isValid())
                    {
                        storeCounterMove(lastMove, move);
                        storeCountermoveHistory(lastMove, move);
                    }
                }

                nodeType = NodeType::BETA;
                break;
            }
        }

        // Store result in transposition table
        if (maxEval > originalAlpha && maxEval < beta)
        {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);

        return maxEval;
    }
    else
    {
        // Minimizing player
        int minEval = std::numeric_limits<int>::max();

        for (size_t i = 0; i < scoredMoves.size(); i++)
        {
            const Move &move = scoredMoves[i].second;

            // NEW: Futility Pruning Section
            int currentMoveEval = evaluatePosition(board);
            bool isCapture = board.getPieceAt(move.to) != nullptr;
            
            // 1. Static Futility Pruning (for quiet moves)
            if (!foundPV && depth <= 3 && !isCapture && i >= 3) {
                if (canUseFutilityPruning(depth, alpha, beta, currentMoveEval, board.isInCheck())) {
                    continue;
                }
            }
            
            // 2. Reverse Futility Pruning (stand-pat)
            if (!foundPV && depth <= 2 && !board.isInCheck()) {
                if (canUseReverseFutilityPruning(depth, currentMoveEval, beta)) {
                    return currentMoveEval;
                }
            }
            
            // 3. Delta Pruning for captures
            if (isCapture && canUseDeltaPruning(currentMoveEval, alpha, move, board)) {
                continue;
            }

          // Determine move characteristics for LMR
            bool isPVMoveCheck = false;
            for (int d = 1; d <= maxDepth; d++)
            {
                if (isPVMove(move, d, ply))
                {
                    isPVMoveCheck = true;
                    break;
                }
            }

            // Note: isCapture already declared above in futility pruning section
            bool isKillerMoveCheck = isKillerMove(move, ply);

            // Save board state for unmaking move
            BoardState previousState;

            // Make the move
            if (!board.makeMove(move, previousState))
                continue;

            bool isCheckMove = board.isInCheck();

            // Calculate depth adjustment
            int moveExtension = extension;

            // Recapture Extension
            if (lastMove.to.isValid() && move.to == lastMove.to)
            {
                moveExtension = std::max(moveExtension, 1);
            }

            // Pawn Push Extension
            auto piece = board.getPieceAt(move.to);
            if (piece && piece->getType() == PieceType::PAWN)
            {
                int destRow = (board.getSideToMove() == Color::BLACK) ? 6 : 1; // 7th rank
                if (move.to.row == destRow)
                {
                    moveExtension = std::max(moveExtension, 1);
                }
            }

          // Calculate LMR reduction using enhanced method
            int lmrReduction = calculateAdvancedLMRReduction(depth, i, foundPV, isCapture, isCheckMove, 
                                                           isKillerMoveCheck, board, move, ply);

            // Final depth after adjustments
            int newDepth = depth - 1 + moveExtension - lmrReduction;
            newDepth = std::max(0, newDepth);

            // Calculate the new hash key after the move
            uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

            // Recursively evaluate the position
            childPV.clear();
            int eval;

            if (foundPV)
            {
                // For non-PV moves, try LMR first if applicable
                if (lmrReduction > 0)
                {
                    // Correct null window for minimizing player
                    eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);

                    // If LMR search fails high, re-search at full depth
                    if (eval < beta)
                    {
                        newDepth = depth - 1 + moveExtension; // Full depth
                        childPV.clear();
                        eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);
                    }
                }
                else
                {
                    // No reduction, do null window search
                    eval = -pvSearch(board, newDepth, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);
                }

                // If we get a fail-high, re-search with full window
                if (eval < beta && eval > alpha)
                {
                    childPV.clear();
                    eval = -pvSearch(board, newDepth, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
                }
            }
            else
            {
                // First move gets a full window search
                eval = -pvSearch(board, newDepth, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
                foundPV = true;
            }

            // Unmake the move
            board.unmakeMove(move, previousState);

            // Update the best move if this move is better
            if (eval < minEval)
            {
                minEval = eval;
                localBestMove = move;

                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }

            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha)
            {
                // Store enhanced killer moves and history
                if (!isCapture)
                {
                    storeEnhancedKillerMove(move, ply);
                    updateHistoryScore(move, depth, Color::BLACK);
                    updateButterflyHistory(move, depth, Color::BLACK);

        if (lastMove.from.isValid() && lastMove.to.isValid())
                    {
                        storeCounterMove(lastMove, move);
                        storeCountermoveHistory(lastMove, move);
                    }
                }

                nodeType = NodeType::ALPHA;
                break;
            }
        }

        // Store result in transposition table
        if (minEval > originalAlpha && minEval < beta)
        {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);

        return minEval;
    }
}

// Regular alpha-beta search (kept for reference/fallback)
int Engine::alphaBeta(Board &board, int depth, int alpha, int beta, bool maximizingPlayer,
                      std::vector<Move> &pv, uint64_t hashKey, int ply, Move lastMove)
{
    // Track nodes searched
    nodesSearched++;

    // Check transposition table for this position
    int originalAlpha = alpha;
   Move ttMove(Position(0, 0), Position(0, 0));
    int score;

    pv.clear();

// Probe the transposition table
    Move tempTTMove(Position(0, 0), Position(0, 0));
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, tempTTMove))
    {
        return score; // Return cached result if available (but don't use TT at root)
    }

    // If we've reached the maximum depth, use quiescence search
    if (depth == 0)
    {
        return quiescenceSearch(board, alpha, beta, hashKey, ply);
    }

    // If the game is over, return the evaluation
    if (board.isCheckmate() || board.isStalemate())
    {
        return evaluatePosition(board);
    }

    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();

    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty())
    {
        if (board.isInCheck())
        {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        }
        else
        {
            // Stalemate
            return 0;
        }
    }

    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto &move : legalMoves)
    {
       Move validTTMove = (ttMove.from.isValid() && ttMove.to.isValid()) ? ttMove : Move(Position(0, 0), Position(0, 0));
        int moveScore = getEnhancedMoveScore(move, board, validTTMove, ply, board.getSideToMove(), lastMove);

        // Early pruning of very bad captures
        if (depth >= 3)
        {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece)
            {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2)
                {
                    continue;
                }
            }
        }

        scoredMoves.push_back(std::make_pair(moveScore, move));
    }

    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const std::pair<int, Move> &a, const std::pair<int, Move> &b)
              {
                  return a.first > b.first;
              });

    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(0, 0), Position(0, 0)) : legalMoves[0];

    // This will be used to store the principal variation
    std::vector<Move> childPV;

    if (maximizingPlayer)
    {
        int maxEval = std::numeric_limits<int>::min();

        for (const auto &scoredMove : scoredMoves)
        {
            const Move &move = scoredMove.second;

            // Save board state for unmaking move
            BoardState previousState;

            // Calculate new hash key BEFORE making the move
            uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

            // Make the move
            if (!board.makeMove(move, previousState))
                continue;

            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(board, depth - 1, alpha, beta, false, childPV, newHashKey, ply + 1, move);

            // Unmake the move
            board.unmakeMove(move, previousState);

            // Update the best move if this move is better
            if (eval > maxEval)
            {
                maxEval = eval;
                localBestMove = move;

                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }

            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
            {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr)
                {
                    // Update killer moves table
                    storeKillerMove(move, ply);

                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());

                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid())
                    {
                        storeCounterMove(lastMove, move);
                    }
                }

                nodeType = NodeType::BETA; // Fail high
                break;
            }
        }

        // Store result in transposition table
        if (maxEval > originalAlpha && maxEval < beta)
        {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);

        return maxEval;
    }
    else
    {
        int minEval = std::numeric_limits<int>::max();

        for (const auto &scoredMove : scoredMoves)
        {
            const Move &move = scoredMove.second;

            // Save board state for unmaking move
            BoardState previousState;

            // Calculate new hash key BEFORE making the move
            uint64_t newHashKey = zobristHasher.updateHashKey(hashKey, move, board);

            // Make the move
            if (!board.makeMove(move, previousState))
                continue;

            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(board, depth - 1, alpha, beta, true, childPV, newHashKey, ply + 1, move);

            // Unmake the move
            board.unmakeMove(move, previousState);

            // Update the best move if this move is better
            if (eval < minEval)
            {
                minEval = eval;
                localBestMove = move;

                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }

            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha)
            {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr)
                {
                    // Update killer moves table
                    storeKillerMove(move, ply);

                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());

                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid())
                    {
                        storeCounterMove(lastMove, move);
                    }
                }

                nodeType = NodeType::ALPHA; // Fail low
                break;
            }
        }

        // Store result in transposition table
        if (minEval > originalAlpha && minEval < beta)
        {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);

        return minEval;
    }
}

// Evaluation function
int Engine::evaluatePosition(const Board &board)
{
    int whiteScore = 0;
    int blackScore = 0;
    bool isEndgamePhase = isEndgame(board);

    // MATERIAL AND POSITIONAL EVALUATION (existing code)
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);

            if (!piece)
                continue;

            // Base piece value
            int pieceValue = 0;
            int positionalValue = 0;
            int tableIndex = row * 8 + col;
            int blackTableIndex = (7 - row) * 8 + col;

            switch (piece->getType())
            {
            case PieceType::PAWN:
                pieceValue = PAWN_VALUE;
                positionalValue = pawnTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;
            case PieceType::KNIGHT:
                pieceValue = KNIGHT_VALUE;
                positionalValue = knightTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;
            case PieceType::BISHOP:
                pieceValue = BISHOP_VALUE;
                positionalValue = bishopTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;
            case PieceType::ROOK:
                pieceValue = ROOK_VALUE;
                positionalValue = rookTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;
            case PieceType::QUEEN:
                pieceValue = QUEEN_VALUE;
                positionalValue = queenTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                break;
            case PieceType::KING:
                pieceValue = KING_VALUE;
                if (isEndgamePhase)
                {
                    positionalValue = kingEndGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                }
                else
                {
                    positionalValue = kingMiddleGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                }
                break;
            default:
                break;
            }

            if (piece->getColor() == Color::WHITE)
            {
                whiteScore += pieceValue + positionalValue;
            }
            else
            {
                blackScore += pieceValue + positionalValue;
            }
        }
    }

    // NEW: ENHANCED EVALUATION COMPONENTS
    
    // 1. Piece Mobility
    int mobilityScore = evaluatePieceMobility(board);
    
    // 2. King Safety
    int kingSafetyScore = evaluateKingSafety(board);
    
    // 3. Pawn Structure
    int pawnStructureScore = evaluatePawnStructure(board);
    
    // 4. Piece Coordination
    int coordinationScore = evaluatePieceCoordination(board);
    
    // 5. Endgame Factors
    int endgameScore = 0;
    if (isEndgamePhase) {
        endgameScore = evaluateEndgameFactors(board);
    }

    // Check for checkmate and stalemate
    if (board.isCheckmate())
    {
        if (board.getSideToMove() == Color::WHITE)
        {
            return -100000;
        }
        else
        {
            return 100000;
        }
    }
    else if (board.isStalemate())
    {
        return 0;
    }

    // Calculate total score
    int materialScore = whiteScore - blackScore;
    int positionalScore = mobilityScore + kingSafetyScore + pawnStructureScore + coordinationScore + endgameScore;
    
    int totalScore = materialScore + positionalScore;

    // Return from current side's perspective
    return board.getSideToMove() == Color::WHITE ? totalScore : -totalScore;
}

bool Engine::shouldStopSearch() const
{
    // Thread-safe time checking
    if (!timeManaged || timeAllocated <= 0) {
        return searchShouldStop.load();
    }

    // Use atomic flag for immediate response
    if (searchShouldStop.load()) {
        return true;
    }

    // Thread-safe time calculation
    std::lock_guard<std::mutex> lock(timeMutex);
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - searchStartTime);

    int calculatedBuffer = std::max(50, std::min(500, timeAllocated * 5 / 100));
    int safeTimeLimit = timeAllocated - calculatedBuffer;

    bool shouldStop = elapsed.count() >= safeTimeLimit;
    
  if (shouldStop) {
        const_cast<std::atomic<bool>&>(searchShouldStop).store(true);
    }
    
    return shouldStop;
}

bool Engine::isEndgame(const Board &board) const
{
    int pieceCount = 0;
    bool whiteQueenPresent = false;
    bool blackQueenPresent = false;

    // Count the number of pieces and check if queens are present
    for (int row = 0; row < 8; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            auto piece = board.getPieceAt(Position(row, col));

            if (piece && piece->getType() != PieceType::KING && piece->getType() != PieceType::PAWN)
            {
                pieceCount++;

                if (piece->getType() == PieceType::QUEEN)
                {
                    if (piece->getColor() == Color::WHITE)
                    {
                        whiteQueenPresent = true;
                    }
                    else
                    {
                        blackQueenPresent = true;
                    }
                }
            }
        }
    }

    // Consider it an endgame if:
    // 1. Both queens are missing, or
    // 2. There are few minor pieces left
    return (!whiteQueenPresent && !blackQueenPresent) || (pieceCount <= 6);
}

// NEW: Futility pruning methods
bool Engine::canUseFutilityPruning(int depth, int alpha, int beta, int eval, bool inCheck) const
{
    if (depth > 3 || inCheck || (beta - alpha) > 1) {
        return false;
    }
    
    int margin = getFutilityMargin(depth, game.getBoard());
    return (eval + margin < alpha);
}

int Engine::getFutilityMargin(int depth, const Board& board) const
{
    const int FUTILITY_MARGIN_BASE = 200;
    const int FUTILITY_MARGIN_MULTIPLIER = 200;
    
    int margin = FUTILITY_MARGIN_BASE + (depth * FUTILITY_MARGIN_MULTIPLIER);
    
    int materialCount = 0;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() != PieceType::PAWN && piece->getType() != PieceType::KING) {
                materialCount++;
            }
        }
    }
    
    margin += materialCount * 25;
    return margin;
}

bool Engine::canUseReverseFutilityPruning(int depth, int eval, int beta) const
{
    if (depth > 2) {
        return false;
    }
    
    const int REVERSE_FUTILITY_MARGIN = 120;
    return (eval - REVERSE_FUTILITY_MARGIN > beta);
}

bool Engine::canUseDeltaPruning(int eval, int alpha, const Move& move, const Board& board) const
{
    auto capturedPiece = board.getPieceAt(move.to);
    if (!capturedPiece) {
        return false;
    }
    
    int captureValue = getPieceValue(capturedPiece->getType());
    
    int promotionBonus = 0;
    auto movingPiece = board.getPieceAt(move.from);
    if (movingPiece && movingPiece->getType() == PieceType::PAWN) {
        if (move.to.row == 0 || move.to.row == 7) {
            promotionBonus = QUEEN_VALUE - PAWN_VALUE;
        }
    }
    
    const int DELTA_PRUNING_MARGIN = 50;
    return (eval + captureValue + promotionBonus + DELTA_PRUNING_MARGIN <= alpha);
}

// NEW: Razoring implementation
bool Engine::canUseRazoring(int depth, int alpha, int eval, bool inCheck) const
{
    // Only use razoring at shallow depths
    if (depth < 1 || depth > 4) {
        return false;
    }
    
    // Never razor when in check
    if (inCheck) {
        return false;
    }
    
    // Don't razor near promotion ranks
    if (hasNearPromotionPawns(game.getBoard(), game.getBoard().getSideToMove())) {
        return false;
    }
    
    // Don't razor with material imbalance (tactical potential)
    if (hasMaterialImbalance(game.getBoard())) {
        return false;
    }
    
    // Check if evaluation is hopeless
    int razoringMargin = getRazoringMargin(depth, game.getBoard());
    return (eval + razoringMargin < alpha);
}

int Engine::getRazoringMargin(int depth, const Board& board) const
{
    // Base razoring margin: 300 + 50 * depth
    int baseMargin = 300 + (50 * depth);
    
    // Increase margin in tactical positions
    int tacticalBonus = 0;
    
    // Count available captures and checks
    std::vector<Move> captures;
    generateCaptureMoves(board, captures);
    
    if (captures.size() > 3) {
        tacticalBonus += 100; // More captures = more tactical
    }
    
    // Increase margin in endgame (evaluation is more precise)
    if (isEndgame(board)) {
        tacticalBonus += 150;
    }
    
    // Increase margin under time pressure
    if (timeManaged && shouldStopSearch()) {
        tacticalBonus += 200;
    }
    
    return baseMargin + tacticalBonus;
}

bool Engine::hasNearPromotionPawns(const Board& board, Color color) const
{
    int promotionRank = (color == Color::WHITE) ? 6 : 1; // 7th rank
    int nearPromotionRank = (color == Color::WHITE) ? 5 : 2; // 6th rank
    
    for (int col = 0; col < 8; col++) {
        // Check 7th rank
        auto piece1 = board.getPieceAt(Position(promotionRank, col));
        if (piece1 && piece1->getType() == PieceType::PAWN && piece1->getColor() == color) {
            return true;
        }
        
        // Check 6th rank
        auto piece2 = board.getPieceAt(Position(nearPromotionRank, col));
        if (piece2 && piece2->getType() == PieceType::PAWN && piece2->getColor() == color) {
            return true;
        }
    }
    
    return false;
}

bool Engine::hasMaterialImbalance(const Board& board) const
{
    int whiteMinor = 0, blackMinor = 0;
    int whiteMajor = 0, blackMajor = 0;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (!piece) continue;
            
            if (piece->getColor() == Color::WHITE) {
                if (piece->getType() == PieceType::KNIGHT || piece->getType() == PieceType::BISHOP) {
                    whiteMinor++;
                } else if (piece->getType() == PieceType::ROOK || piece->getType() == PieceType::QUEEN) {
                    whiteMajor++;
                }
            } else {
                if (piece->getType() == PieceType::KNIGHT || piece->getType() == PieceType::BISHOP) {
                    blackMinor++;
                } else if (piece->getType() == PieceType::ROOK || piece->getType() == PieceType::QUEEN) {
                    blackMajor++;
                }
            }
        }
    }
    
   return (abs(whiteMinor - blackMinor) > 1) || (abs(whiteMajor - blackMajor) > 0);
}

// Pruning Integration Methods
bool Engine::shouldAllowMultiplePruning(int depth, int ply, bool inCheck) const
{
    // Never allow multiple aggressive pruning when in check
    if (inCheck) {
        return false;
    }
    
    // At shallow depths, be more conservative
    if (depth <= 2) {
        return false;
    }
    
    // Check how many pruning techniques already used at this ply
    if (ply >= 0 && ply < MAX_PLY) {
        return pruningUsedAtPly[ply] < PRUNING_CONFLICT_THRESHOLD;
    }
    
    return true;
}

int Engine::getPruningPriority(const std::string& pruningType, int depth, int ply) const
{
    // Priority order (higher number = higher priority)
    if (pruningType == "null_move") return 100;      // Highest reduction
    if (pruningType == "razoring") return 80;        // Medium reduction
    if (pruningType == "futility") return 60;        // Local move skipping
    if (pruningType == "lmr") return 40;            // Per-move reduction
    
    return 0;
}

void Engine::trackPruningUsage(const std::string& pruningType, int depth, int ply) const
{
    // Track usage at this ply
    if (ply >= 0 && ply < MAX_PLY) {
        pruningUsedAtPly[ply]++;
    }
    
    // Update global statistics
    if (pruningType == "null_move") pruningStats[0]++;
    else if (pruningType == "razoring") pruningStats[1]++;
    else if (pruningType == "futility") pruningStats[2]++;
    else if (pruningType == "lmr") pruningStats[3]++;
    
    // Track conflicts
    if (ply >= 0 && ply < MAX_PLY && pruningUsedAtPly[ply] > 1) {
        pruningStats[4]++; // Conflict count
    }
}

// NEW: Parameter Tuning Framework Implementation
void Engine::initializeTuningParameters()
{
    tuningParameters.clear();
    
    // Add tunable parameters (using const_cast to modify const members for tuning)
    tuningParameters.push_back({"NULL_MOVE_MIN_DEPTH", const_cast<int*>(&NULL_MOVE_MIN_DEPTH), 2, 5, 1, NULL_MOVE_MIN_DEPTH});
    tuningParameters.push_back({"LMR_MIN_DEPTH", const_cast<int*>(&LMR_MIN_DEPTH), 2, 5, 1, LMR_MIN_DEPTH});
    tuningParameters.push_back({"LMR_MIN_MOVE_INDEX", const_cast<int*>(&LMR_MIN_MOVE_INDEX), 3, 6, 1, LMR_MIN_MOVE_INDEX});
    tuningParameters.push_back({"MAX_EXTENSIONS_PER_PLY", const_cast<int*>(&MAX_EXTENSIONS_PER_PLY), 1, 4, 1, MAX_EXTENSIONS_PER_PLY});
    tuningParameters.push_back({"MAX_TOTAL_EXTENSIONS", const_cast<int*>(&MAX_TOTAL_EXTENSIONS), 10, 25, 5, MAX_TOTAL_EXTENSIONS});
    
    std::cout << "Initialized " << tuningParameters.size() << " tuning parameters" << std::endl;
}

void Engine::runParameterTuning(const std::string& testSuite)
{
    std::cout << "=== AUTOMATED PARAMETER TUNING ===" << std::endl;
    
    initializeTuningParameters();
    
    // Load test positions (EPD format)
    std::vector<std::string> testPositions = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
    };
    
    // Baseline performance
    double baselineScore = evaluateParameterSet(testPositions);
    std::cout << "Baseline performance: " << baselineScore << std::endl;
    
    // Test each parameter
    for (auto& param : tuningParameters) {
        std::cout << "\nTuning parameter: " << param.name << std::endl;
        
        int bestValue = *param.valuePtr;
        double bestScore = baselineScore;
        
        // Test different values
        for (int value = param.minValue; value <= param.maxValue; value += param.step) {
            if (value == param.originalValue) continue; // Skip original value
            
            *param.valuePtr = value;
            double score = evaluateParameterSet(testPositions);
            
            std::cout << "  " << param.name << "=" << value << " -> Score: " << score << std::endl;
            
            if (score > bestScore) {
                bestScore = score;
                bestValue = value;
            }
        }
        
        // Set best value
        *param.valuePtr = bestValue;
        tuningResults[param.name] = bestScore;
        
        std::cout << "  Best " << param.name << ": " << bestValue 
                  << " (improvement: " << (bestScore - baselineScore) << ")" << std::endl;
    }
    
    printTuningResults();
}

bool Engine::runABTest(const std::string& parameterName, int newValue, int testGames)
{
    std::cout << "Running A/B test for " << parameterName << " = " << newValue << std::endl;
    
    // Find parameter
    auto it = std::find_if(tuningParameters.begin(), tuningParameters.end(),
                          [&](const TuningParameter& p) { return p.name == parameterName; });
    
    if (it == tuningParameters.end()) {
        std::cout << "Parameter not found!" << std::endl;
        return false;
    }
    
    int originalValue = *it->valuePtr;
    
    // Test positions
    std::vector<std::string> testPositions = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
    };
    
    // Baseline test
    double baselineScore = evaluateParameterSet(testPositions);
    
    // Modified test
    *it->valuePtr = newValue;
    double newScore = evaluateParameterSet(testPositions);
    
    // Calculate statistical significance (simplified)
    double improvement = newScore - baselineScore;
    double improvementPercent = (improvement / baselineScore) * 100.0;
    
    bool isSignificant = abs(improvementPercent) > 2.0; // 2% threshold
    
    std::cout << "A/B Test Results:" << std::endl;
    std::cout << "  Baseline: " << baselineScore << std::endl;
    std::cout << "  New:      " << newScore << std::endl;
    std::cout << "  Change:   " << improvementPercent << "%" << std::endl;
    std::cout << "  Significant: " << (isSignificant ? "YES" : "NO") << std::endl;
    
    if (!isSignificant || improvement < 0) {
        // Revert to original value
        *it->valuePtr = originalValue;
        return false;
    }
    
    return true;
}

double Engine::evaluateParameterSet(const std::vector<std::string>& testPositions)
{
    double totalScore = 0.0;
    int totalTests = 0;
    
    for (const auto& fen : testPositions) {
        try {
            Game testGame;
            testGame.newGameFromFEN(fen);
            
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Quick depth-limited search
            int oldDepth = maxDepth;
            maxDepth = 4; // Fast evaluation
            
            Move bestMove = getBestMove();
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            maxDepth = oldDepth; // Restore depth
            
            // Score based on nodes per second and move quality
            double nps = (duration.count() > 0) ? (getNodesSearched() * 1000.0 / duration.count()) : 0;
            double score = nps / 1000.0; // Convert to thousands
            
            totalScore += score;
            totalTests++;
            
        } catch (...) {
            // Skip problematic positions
            continue;
        }
    }
    
    return totalTests > 0 ? (totalScore / totalTests) : 0.0;
}

void Engine::printTuningResults() const
{
    std::cout << "\n=== TUNING RESULTS SUMMARY ===" << std::endl;
    
    for (const auto& param : tuningParameters) {
        auto it = tuningResults.find(param.name);
        if (it != tuningResults.end()) {
            std::cout << param.name << ": " << *param.valuePtr 
                      << " (was " << param.originalValue << ") -> Score: " << it->second << std::endl;
        }
    }
    
    std::cout << "\nPruning Statistics:" << std::endl;
    std::cout << "Null Move: " << pruningStats[0] << std::endl;
    std::cout << "Razoring: " << pruningStats[1] << std::endl;
    std::cout << "Futility: " << pruningStats[2] << std::endl;
    std::cout << "LMR: " << pruningStats[3] << std::endl;
    std::cout << "Conflicts: " << pruningStats[4] << std::endl;
}
// ===== ENHANCED EVALUATION IMPLEMENTATIONS =====

int Engine::evaluatePieceMobility(const Board& board) const
{
    int whiteMobility = countPieceMobility(board, Color::WHITE);
    int blackMobility = countPieceMobility(board, Color::BLACK);
    
    return (whiteMobility - blackMobility) * MOBILITY_WEIGHT;
}

int Engine::countPieceMobility(const Board& board, Color color) const
{
    int totalMobility = 0;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);
            
            if (!piece || piece->getColor() != color) continue;
            
            // Count legal moves for this piece
            auto moves = piece->getLegalMoves(board);
            int moveCount = moves.size();
            
            // Weight mobility by piece type
            int mobilityBonus = 0;
            switch (piece->getType()) {
                case PieceType::KNIGHT:
                    mobilityBonus = moveCount * MOBILITY_BONUS_KNIGHT;
                    break;
                case PieceType::BISHOP:
                    mobilityBonus = moveCount * MOBILITY_BONUS_BISHOP;
                    break;
                case PieceType::ROOK:
                    mobilityBonus = moveCount * MOBILITY_BONUS_ROOK;
                    break;
                case PieceType::QUEEN:
                    mobilityBonus = moveCount * MOBILITY_BONUS_QUEEN;
                    break;
                default:
                    // Pawns and Kings get minimal mobility bonus
                    mobilityBonus = moveCount;
                    break;
            }
            
            totalMobility += mobilityBonus;
            
            // Penalty for trapped pieces (very low mobility)
            if (moveCount <= 1 && piece->getType() != PieceType::PAWN && piece->getType() != PieceType::KING) {
                totalMobility -= 25; // Trapped piece penalty
            }
        }
    }
    
    return totalMobility;
}

int Engine::evaluateKingSafety(const Board& board) const
{
    int whiteKingSafety = evaluateKingSafetyForColor(board, Color::WHITE);
    int blackKingSafety = evaluateKingSafetyForColor(board, Color::BLACK);
    
    return (whiteKingSafety - blackKingSafety) * KING_SAFETY_WEIGHT;
}

int Engine::evaluateKingSafetyForColor(const Board& board, Color color) const
{
    // Find the king
    Position kingPos;
    bool foundKing = false;
    
    for (int row = 0; row < 8 && !foundKing; row++) {
        for (int col = 0; col < 8 && !foundKing; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::KING && piece->getColor() == color) {
                kingPos = Position(row, col);
                foundKing = true;
            }
        }
    }
    
    if (!foundKing) return 0;
    
    int safetyScore = 0;
    Color opponentColor = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    
    // 1. Pawn Shelter
    safetyScore += countPawnShelter(board, kingPos, color);
    
    // 2. King Zone Safety
    safetyScore += evaluateKingZone(board, kingPos, color);
    
    // 3. Attacking Pieces Near King
    int attackers = countKingAttackers(board, kingPos, opponentColor);
    safetyScore -= attackers * KING_ATTACKER_PENALTY;
    
    // 4. King Exposure Penalty (in center or advanced position)
    if (isEndgame(board)) {
        // In endgame, king activity is good
        int centerDistance = abs(kingPos.row - 3.5) + abs(kingPos.col - 3.5);
        safetyScore += (7 - centerDistance) * 5; // Bonus for active king
    } else {
        // In middlegame, king should be safe
        if (kingPos.row > 1 && kingPos.row < 6) {
            safetyScore += EXPOSED_KING_PENALTY; // Penalty for exposed king
        }
    }
    
    return safetyScore;
}

int Engine::countPawnShelter(const Board& board, Position kingPos, Color kingColor) const
{
    int shelterScore = 0;
    int direction = (kingColor == Color::WHITE) ? 1 : -1;
    
    // Check pawn shelter in front of king
    for (int dCol = -1; dCol <= 1; dCol++) {
        Position pawnPos(kingPos.row + direction, kingPos.col + dCol);
        Position pawnPos2(kingPos.row + 2 * direction, kingPos.col + dCol);
        
        if (pawnPos.isValid()) {
            auto piece = board.getPieceAt(pawnPos);
            if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == kingColor) {
                shelterScore += PAWN_SHELTER_BONUS;
            }
        }
        
        // Bonus for pawns two squares away
        if (pawnPos2.isValid()) {
            auto piece = board.getPieceAt(pawnPos2);
            if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == kingColor) {
                shelterScore += PAWN_SHELTER_BONUS / 2;
            }
        }
    }
    
    return shelterScore;
}

int Engine::countKingAttackers(const Board& board, Position kingPos, Color attackerColor) const
{
    int attackers = 0;
    
    // Check for attacking pieces in king zone (3x3 around king)
    for (int dRow = -2; dRow <= 2; dRow++) {
        for (int dCol = -2; dCol <= 2; dCol++) {
            Position checkPos(kingPos.row + dRow, kingPos.col + dCol);
            if (!checkPos.isValid()) continue;
            
            if (board.isSquareAttacked(checkPos, attackerColor)) {
                attackers++;
            }
        }
    }
    
    return attackers;
}

int Engine::evaluateKingZone(const Board& board, Position kingPos, Color kingColor) const
{
    int zoneScore = 0;
    Color opponentColor = (kingColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    
    // Penalty for opponent pieces near king
    for (int dRow = -2; dRow <= 2; dRow++) {
        for (int dCol = -2; dCol <= 2; dCol++) {
            Position nearPos(kingPos.row + dRow, kingPos.col + dCol);
            if (!nearPos.isValid()) continue;
            
            auto piece = board.getPieceAt(nearPos);
            if (piece && piece->getColor() == opponentColor) {
                // More penalty for stronger pieces near king
                switch (piece->getType()) {
                    case PieceType::QUEEN:
                        zoneScore -= 8;
                        break;
                    case PieceType::ROOK:
                        zoneScore -= 5;
                        break;
                    case PieceType::BISHOP:
                    case PieceType::KNIGHT:
                        zoneScore -= 3;
                        break;
                    default:
                        zoneScore -= 1;
                        break;
                }
            }
        }
    }
    
    return zoneScore;
}

int Engine::evaluatePawnStructure(const Board& board) const
{
    int whiteScore = evaluatePawnsForColor(board, Color::WHITE);
    int blackScore = evaluatePawnsForColor(board, Color::BLACK);
    
    return (whiteScore - blackScore) * PAWN_STRUCTURE_WEIGHT;
}

int Engine::evaluatePawnsForColor(const Board& board, Color color) const
{
    int pawnScore = 0;
    
    // Evaluate each pawn
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);
            
            if (!piece || piece->getType() != PieceType::PAWN || piece->getColor() != color) {
                continue;
            }
            
            // Check for pawn weaknesses
            if (isPawnIsolated(board, pos)) {
                pawnScore += ISOLATED_PAWN_PENALTY;
            }
            
            if (isPawnDoubled(board, pos)) {
                pawnScore += DOUBLED_PAWN_PENALTY;
            }
            
            if (isPawnBackward(board, pos)) {
                pawnScore += BACKWARD_PAWN_PENALTY;
            }
            
            // Check for pawn strengths
            if (isPawnPassed(board, pos)) {
                pawnScore += PASSED_PAWN_BONUS;
                
                // Bonus increases as pawn advances
                int advancement = (color == Color::WHITE) ? row : (7 - row);
                pawnScore += advancement * 3;
            }
        }
    }
    
    // Penalty for pawn islands
    int islands = getPawnIslands(board, color);
    pawnScore += islands * PAWN_ISLAND_PENALTY;
    
    return pawnScore;
}

bool Engine::isPawnIsolated(const Board& board, Position pawnPos) const
{
    auto pawn = board.getPieceAt(pawnPos);
    if (!pawn || pawn->getType() != PieceType::PAWN) return false;
    
    Color pawnColor = pawn->getColor();
    
    // Check adjacent files for friendly pawns
    for (int dCol = -1; dCol <= 1; dCol += 2) { // Check left and right files
        int checkCol = pawnPos.col + dCol;
        if (checkCol < 0 || checkCol > 7) continue;
        
        // Check entire file for friendly pawns
        for (int row = 0; row < 8; row++) {
            auto piece = board.getPieceAt(Position(row, checkCol));
            if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == pawnColor) {
                return false; // Found friendly pawn on adjacent file
            }
        }
    }
    
    return true; // No friendly pawns on adjacent files
}

bool Engine::isPawnDoubled(const Board& board, Position pawnPos) const
{
    auto pawn = board.getPieceAt(pawnPos);
    if (!pawn || pawn->getType() != PieceType::PAWN) return false;
    
    Color pawnColor = pawn->getColor();
    
    // Check same file for other friendly pawns
    for (int row = 0; row < 8; row++) {
        if (row == pawnPos.row) continue; // Skip self
        
        auto piece = board.getPieceAt(Position(row, pawnPos.col));
        if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == pawnColor) {
            return true; // Found another friendly pawn on same file
        }
    }
    
    return false;
}

bool Engine::isPawnBackward(const Board& board, Position pawnPos) const
{
    auto pawn = board.getPieceAt(pawnPos);
    if (!pawn || pawn->getType() != PieceType::PAWN) return false;
    
    Color pawnColor = pawn->getColor();
    int direction = (pawnColor == Color::WHITE) ? 1 : -1;
    
    // Check if pawn can be defended by adjacent pawns
    for (int dCol = -1; dCol <= 1; dCol += 2) {
        int checkCol = pawnPos.col + dCol;
        if (checkCol < 0 || checkCol > 7) continue;
        
        Position defenderPos(pawnPos.row - direction, checkCol);
        if (defenderPos.isValid()) {
            auto piece = board.getPieceAt(defenderPos);
            if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == pawnColor) {
                return false; // Can be defended
            }
        }
    }
    
    // Check if advance is blocked by enemy pawn
    Position frontPos(pawnPos.row + direction, pawnPos.col);
    if (frontPos.isValid()) {
        auto piece = board.getPieceAt(frontPos);
        if (piece && piece->getColor() != pawnColor) {
            return true; // Blocked and cannot be defended
        }
    }
    
    return false;
}

bool Engine::isPawnPassed(const Board& board, Position pawnPos) const
{
    auto pawn = board.getPieceAt(pawnPos);
    if (!pawn || pawn->getType() != PieceType::PAWN) return false;
    
    Color pawnColor = pawn->getColor();
    Color opponentColor = (pawnColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    int direction = (pawnColor == Color::WHITE) ? 1 : -1;
    
    // Check if any enemy pawns can stop this pawn
    for (int dCol = -1; dCol <= 1; dCol++) {
        int checkCol = pawnPos.col + dCol;
        if (checkCol < 0 || checkCol > 7) continue;
        
        // Check from current row to end of board
        for (int row = pawnPos.row; row >= 0 && row < 8; row += direction) {
            if (row == pawnPos.row && dCol == 0) continue; // Skip self
            
            auto piece = board.getPieceAt(Position(row, checkCol));
            if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == opponentColor) {
                return false; // Enemy pawn can stop/capture
            }
        }
    }
    
    return true;
}

int Engine::getPawnIslands(const Board& board, Color color) const
{
    bool hasPawn[8] = {false}; // Track which files have pawns
    
    // Mark files with pawns
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == color) {
                hasPawn[col] = true;
            }
        }
    }
    
    // Count islands (groups of consecutive files with pawns)
    int islands = 0;
    bool inIsland = false;
    
    for (int col = 0; col < 8; col++) {
        if (hasPawn[col] && !inIsland) {
            islands++;
            inIsland = true;
        } else if (!hasPawn[col] && inIsland) {
            inIsland = false;
        }
    }
    
    return islands;
}

int Engine::evaluatePieceCoordination(const Board& board) const
{
    int whiteScore = evaluatePieceActivity(board, Color::WHITE);
    int blackScore = evaluatePieceActivity(board, Color::BLACK);
    
    return (whiteScore - blackScore) * PIECE_COORDINATION_WEIGHT;
}

int Engine::evaluatePieceActivity(const Board& board, Color color) const
{
    int activityScore = 0;
    
    // Bishop pair bonus
    if (hasBishopPair(board, color)) {
        activityScore += BISHOP_PAIR_BONUS;
    }
    
    // Evaluate each piece for activity
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);
            
            if (!piece || piece->getColor() != color) continue;
            
            switch (piece->getType()) {
                case PieceType::KNIGHT:
                    if (isKnightOutpost(board, pos)) {
                        activityScore += KNIGHT_OUTPOST_BONUS;
                    }
                    break;
                    
                case PieceType::ROOK:
                    if (isRookOnOpenFile(board, pos)) {
                        activityScore += ROOK_OPEN_FILE_BONUS;
                    } else if (isRookOnSemiOpenFile(board, pos)) {
                        activityScore += ROOK_SEMI_OPEN_FILE_BONUS;
                    }
                    break;
                    
                case PieceType::BISHOP:
                    // Bonus for bishops on long diagonals
                    if ((pos.row == pos.col) || (pos.row + pos.col == 7)) {
                        activityScore += 8; // Long diagonal bonus
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    return activityScore;
}

bool Engine::hasBishopPair(const Board& board, Color color) const
{
    int bishopCount = 0;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::BISHOP && piece->getColor() == color) {
                bishopCount++;
            }
        }
    }
    
    return bishopCount >= 2;
}

bool Engine::isKnightOutpost(const Board& board, Position knightPos) const
{
    auto knight = board.getPieceAt(knightPos);
    if (!knight || knight->getType() != PieceType::KNIGHT) return false;
    
    Color knightColor = knight->getColor();
    Color opponentColor = (knightColor == Color::WHITE) ? Color::BLACK : Color::WHITE;
    
    // Must be in opponent's half
    bool inOpponentHalf = (knightColor == Color::WHITE && knightPos.row >= 4) ||
                          (knightColor == Color::BLACK && knightPos.row <= 3);
    
    if (!inOpponentHalf) return false;
    
    // Must be defended by own pawn
    int direction = (knightColor == Color::WHITE) ? -1 : 1;
    
    for (int dCol = -1; dCol <= 1; dCol += 2) {
        Position defenderPos(knightPos.row + direction, knightPos.col + dCol);
        if (defenderPos.isValid()) {
            auto piece = board.getPieceAt(defenderPos);
            if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == knightColor) {
                // Check that it cannot be attacked by enemy pawns
                for (int dCol2 = -1; dCol2 <= 1; dCol2 += 2) {
                    Position attackerPos(knightPos.row - direction, knightPos.col + dCol2);
                    if (attackerPos.isValid()) {
                        auto attacker = board.getPieceAt(attackerPos);
                        if (attacker && attacker->getType() == PieceType::PAWN && attacker->getColor() == opponentColor) {
                            return false;
                        }
                    }
                }
                return true;
            }
        }
    }
    
    return false;
}

bool Engine::isRookOnOpenFile(const Board& board, Position rookPos) const
{
    auto rook = board.getPieceAt(rookPos);
    if (!rook || rook->getType() != PieceType::ROOK) return false;
    
    // Check if file has no pawns
    for (int row = 0; row < 8; row++) {
        auto piece = board.getPieceAt(Position(row, rookPos.col));
        if (piece && piece->getType() == PieceType::PAWN) {
            return false;
        }
    }
    
    return true;
}

bool Engine::isRookOnSemiOpenFile(const Board& board, Position rookPos) const
{
    auto rook = board.getPieceAt(rookPos);
    if (!rook || rook->getType() != PieceType::ROOK) return false;
    
    Color rookColor = rook->getColor();
    bool hasOwnPawn = false;
    bool hasEnemyPawn = false;
    
    // Check file for pawns
    for (int row = 0; row < 8; row++) {
        auto piece = board.getPieceAt(Position(row, rookPos.col));
        if (piece && piece->getType() == PieceType::PAWN) {
            if (piece->getColor() == rookColor) {
                hasOwnPawn = true;
            } else {
                hasEnemyPawn = true;
            }
        }
    }
    
    // Semi-open = no own pawns but has enemy pawns
    return !hasOwnPawn && hasEnemyPawn;
}

int Engine::evaluateEndgameFactors(const Board& board) const
{
    int whiteScore = evaluateKingActivity(board, Color::WHITE);
    int blackScore = evaluateKingActivity(board, Color::BLACK);
    
    return (whiteScore - blackScore) * ENDGAME_WEIGHT;
}

int Engine::evaluateKingActivity(const Board& board, Color color) const
{
    // Find the king
    Position kingPos;
    bool foundKing = false;
    
    for (int row = 0; row < 8 && !foundKing; row++) {
        for (int col = 0; col < 8 && !foundKing; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::KING && piece->getColor() == color) {
                kingPos = Position(row, col);
                foundKing = true;
            }
        }
    }
    
    if (!foundKing) return 0;
    
    int activityScore = 0;
    
    // King activity bonus (closer to center is better in endgame)
    int centerDistance = abs(kingPos.row - 3.5) + abs(kingPos.col - 3.5);
    activityScore += (7 - centerDistance) * 8;
    
    // King mobility
    auto king = board.getPieceAt(kingPos);
    if (king) {
        auto moves = king->getLegalMoves(board);
        activityScore += moves.size() * 3;
    }
    
    // Opposition evaluation (simplified)
    Color opponentColor = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    Position opponentKingPos;
    bool foundOpponentKing = false;
    
    for (int row = 0; row < 8 && !foundOpponentKing; row++) {
        for (int col = 0; col < 8 && !foundOpponentKing; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            if (piece && piece->getType() == PieceType::KING && piece->getColor() == opponentColor) {
                opponentKingPos = Position(row, col);
                foundOpponentKing = true;
            }
        }
    }
    
    if (foundOpponentKing) {
        int kingDistance = abs(kingPos.row - opponentKingPos.row) + abs(kingPos.col - opponentKingPos.col);
        
        // Opposition bonus (having the move when kings face each other)
        if (kingDistance == 2 && 
            ((kingPos.row == opponentKingPos.row && abs(kingPos.col - opponentKingPos.col) == 2) ||
             (kingPos.col == opponentKingPos.col && abs(kingPos.row - opponentKingPos.row) == 2))) {
            if (board.getSideToMove() != color) {
                activityScore += 15; // Opposition bonus
            }
        }
        
        // King proximity to enemy pawns (for king and pawn endgames)
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                auto piece = board.getPieceAt(Position(row, col));
                if (piece && piece->getType() == PieceType::PAWN && piece->getColor() == opponentColor) {
                    int pawnDistance = abs(kingPos.row - row) + abs(kingPos.col - col);
                    activityScore += (8 - pawnDistance) * 2; // Closer to enemy pawns is better
                }
            }
        }
    }
    
    return activityScore;
}