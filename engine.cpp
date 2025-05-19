// If it's a good capture (positive SEE)
            if (seeScore > 0) {
                return 4000000 + seeScore;
            } 
            // Still prioritize captures, but lower than good captures
            else {
                return 3000000 + getMVVLVAScore(movingPiece->getType(), capturedPiece->getType());
            }
        }
    }
    
    // 4. Counter move
    if (lastMove.from.isValid() && lastMove.to.isValid()) {
        Move counter = getCounterMove(lastMove);
        if (counter.from.isValid() && counter.to.isValid() &&
            counter.from.row == move.from.row && counter.from.col == move.from.col &&
            counter.to.row == move.to.row && counter.to.col == move.to.col) {
            return 2500000;
        }
    }
    
    // 5. Killer moves
    if (isKillerMove(move, ply)) {
        // First killer move gets slightly higher score than second
        if (killerMoves[ply][0].from.row == move.from.row && 
            killerMoves[ply][0].from.col == move.from.col && 
            killerMoves[ply][0].to.row == move.to.row && 
            killerMoves[ply][0].to.col == move.to.col) {
            return 2000100;
        }
        return 2000000;
    }
    
    // 6. History heuristic for non-captures
    return getHistoryScore(move, sideToMove);
}

// Principal Variation Search (PVS) - a more efficient version of alpha-beta for PV nodes
int Engine::pvSearch(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, 
                    std::vector<Move>& pv, uint64_t hashKey, int ply, Move lastMove) {
    // Track nodes searched
    nodesSearched++;
    
    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(), Position());
    int score;
    
    pv.clear();
    
    // Probe the transposition table
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, ttMove)) {
        return score; // Return cached result if available (but don't use TT at root)
    }
    
    // If we've reached the maximum depth or the game is over, evaluate the position
    if (depth == 0 || board.isCheckmate() || board.isStalemate()) {
        return evaluatePosition(board);
    }
    
    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty()) {
        if (board.isInCheck()) {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        } else {
            // Stalemate
            return 0;
        }
    }
    
    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto& move : legalMoves) {
        int moveScore = getMoveScore(move, board, ttMove, principalVariation, ply, board.getSideToMove(), lastMove);
        
        // Early pruning of very bad captures
        if (depth >= 3) {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece) {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2) {
                    continue;
                }
            }
        }
        
        scoredMoves.push_back(std::make_pair(moveScore, move));
    }
    
    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(), 
              [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) {
                  return a.first > b.first;
              });
    
    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];
    bool foundPV = false;
    
    // This will be used to store the principal variation
    std::vector<Move> childPV;
    
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        
        for (size_t i = 0; i < scoredMoves.size(); i++) {
            const Move& move = scoredMoves[i].second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval;
            
            // Full window search for first move, null window for others
            if (foundPV) {
                // Try a null window search first
                eval = -pvSearch(tempBoard, depth - 1, -alpha - 1, -alpha, false, childPV, newHashKey, ply + 1, move);
                
                // If we might fail high, do a full window search
                if (eval > alpha && eval < beta) {
                    childPV.clear();
                    eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
                }
            } else {
                // First move gets a full window search
                eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, false, childPV, newHashKey, ply + 1, move);
            }
            
            // Update the best move if this move is better
            if (eval > maxEval) {
                maxEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
                
                foundPV = true;
            }
            
            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::BETA; // Fail high
                break;
            }
        }
        
        // Store result in transposition table
        if (maxEval > originalAlpha && maxEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);
        
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        
        for (size_t i = 0; i < scoredMoves.size(); i++) {
            const Move& move = scoredMoves[i].second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval;
            
            // Full window search for first move, null window for others
            if (foundPV) {
                // Try a null window search first
                eval = -pvSearch(tempBoard, depth - 1, -alpha - 1, -alpha, true, childPV, newHashKey, ply + 1, move);
                
                // If we might fail high, do a full window search
                if (eval > alpha && eval < beta) {
                    childPV.clear();
                    eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
                }
            } else {
                // First move gets a full window search
                eval = -pvSearch(tempBoard, depth - 1, -beta, -alpha, true, childPV, newHashKey, ply + 1, move);
            }
            
            // Update the best move if this move is better
            if (eval < minEval) {
                minEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
                
                foundPV = true;
            }
            
            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::ALPHA; // Fail low
                break;
            }
        }
        
        // Store result in transposition table
        if (minEval > originalAlpha && minEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);
        
        return minEval;
    }
}

// Regular alpha-beta search (kept for reference/fallback)
int Engine::alphaBeta(Board& board, int depth, int alpha, int beta, bool maximizingPlayer, 
                    std::vector<Move>& pv, uint64_t hashKey, int ply, Move lastMove) {
    // Track nodes searched
    nodesSearched++;
    
    // Check transposition table for this position
    int originalAlpha = alpha;
    Move ttMove(Position(), Position());
    int score;
    
    pv.clear();
    
    // Probe the transposition table
    if (ply > 0 && transpositionTable.probe(hashKey, depth, alpha, beta, score, ttMove)) {
        return score; // Return cached result if available (but don't use TT at root)
    }
    
    // If we've reached the maximum depth or the game is over, evaluate the position
    if (depth == 0 || board.isCheckmate() || board.isStalemate()) {
        return evaluatePosition(board);
    }
    
    // Generate all legal moves
    std::vector<Move> legalMoves = board.generateLegalMoves();
    
    // If there are no legal moves, either checkmate or stalemate
    if (legalMoves.empty()) {
        if (board.isInCheck()) {
            // Checkmate (worst possible score, adjusted for distance to mate)
            return maximizingPlayer ? -100000 + ply : 100000 - ply;
        } else {
            // Stalemate
            return 0;
        }
    }
    
    // Score each move for ordering
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto& move : legalMoves) {
        int moveScore = getMoveScore(move, board, ttMove, principalVariation, ply, board.getSideToMove(), lastMove);
        
        // Early pruning of very bad captures
        if (depth >= 3) {
            auto capturedPiece = board.getPieceAt(move.to);
            if (capturedPiece) {
                int seeScore = seeCapture(board, move);
                // If SEE indicates a very bad capture, don't even consider this move
                if (seeScore < -PAWN_VALUE * 2) {
                    continue;
                }
            }
        }
        
        scoredMoves.push_back(std::make_pair(moveScore, move));
    }
    
    // Sort moves by score (descending)
    std::sort(scoredMoves.begin(), scoredMoves.end(), 
              [](const std::pair<int, Move>& a, const std::pair<int, Move>& b) {
                  return a.first > b.first;
              });
    
    NodeType nodeType = NodeType::ALPHA;
    Move localBestMove = legalMoves.empty() ? Move(Position(), Position()) : legalMoves[0];
    
    // This will be used to store the principal variation
    std::vector<Move> childPV;
    
    if (maximizingPlayer) {
        int maxEval = std::numeric_limits<int>::min();
        
        for (const auto& scoredMove : scoredMoves) {
            const Move& move = scoredMove.second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, false, childPV, newHashKey, ply + 1, move);
            
            // Update the best move if this move is better
            if (eval > maxEval) {
                maxEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }
            
            // Alpha-beta pruning
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::BETA; // Fail high
                break;
            }
        }
        
        // Store result in transposition table
        if (maxEval > originalAlpha && maxEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, maxEval, nodeType, localBestMove);
        
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        
        for (const auto& scoredMove : scoredMoves) {
            const Move& move = scoredMove.second;
            
            // Make a copy of the board
            Board tempBoard = board;
            
            // Make the move
            tempBoard.makeMove(move);
            
            // Calculate the new hash key after the move
            uint64_t newHashKey = Zobrist::updateHashKey(hashKey, move, board);
            
            // Recursively evaluate the position
            childPV.clear();
            int eval = alphaBeta(tempBoard, depth - 1, alpha, beta, true, childPV, newHashKey, ply + 1, move);
            
            // Update the best move if this move is better
            if (eval < minEval) {
                minEval = eval;
                localBestMove = move;
                
                // Update principal variation
                pv.clear();
                pv.push_back(move);
                pv.insert(pv.end(), childPV.begin(), childPV.end());
            }
            
            // Alpha-beta pruning
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                // Store this move as a killer move if it's not a capture
                if (board.getPieceAt(move.to) == nullptr) {
                    // Update killer moves table
                    storeKillerMove(move, ply);
                    
                    // Update history heuristic
                    updateHistoryScore(move, depth, board.getSideToMove());
                    
                    // Store counter move if we have a previous move
                    if (lastMove.from.isValid() && lastMove.to.isValid()) {
                        storeCounterMove(lastMove, move);
                    }
                }
                
                nodeType = NodeType::ALPHA; // Fail low
                break;
            }
        }
        
        // Store result in transposition table
        if (minEval > originalAlpha && minEval < beta) {
            nodeType = NodeType::EXACT;
        }
        transpositionTable.store(hashKey, depth, minEval, nodeType, localBestMove);
        
        return minEval;
    }
}

// Evaluation function remains unchanged
int Engine::evaluatePosition(const Board& board) {
    // Existing evaluation code...
    int whiteScore = 0;
    int blackScore = 0;
    bool isEndgamePhase = isEndgame(board);
    
    // Loop through all squares on the board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Position pos(row, col);
            auto piece = board.getPieceAt(pos);
            
            if (!piece) continue;
            
            // Base piece value
            int pieceValue = 0;
            
            // Positional value based on piece-square tables
            int positionalValue = 0;
            
            // Calculate the index for the piece-square tables
            int tableIndex = row * 8 + col;
            
            // Adjust the index for black pieces (mirror the board)
            int blackTableIndex = (7 - row) * 8 + col;
            
            switch (piece->getType()) {
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
                    if (isEndgamePhase) {
                        positionalValue = kingEndGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                    } else {
                        positionalValue = kingMiddleGameTable[piece->getColor() == Color::WHITE ? tableIndex : blackTableIndex];
                    }
                    break;
                    
                default:
                    break;
            }
            
            // Add the piece value and positional value to the appropriate side's score
            if (piece->getColor() == Color::WHITE) {
                whiteScore += pieceValue + positionalValue;
            } else {
                blackScore += pieceValue + positionalValue;
            }
        }
    }
    
    // Check for checkmate and stalemate
    if (board.isCheckmate()) {
        if (board.getSideToMove() == Color::WHITE) {
            // Black wins
            return -100000;
        } else {
            // White wins
            return 100000;
        }
    } else if (board.isStalemate()) {
        // Draw
        return 0;
    }
    
    // Calculate the final score from white's perspective
    int score = whiteScore - blackScore;
    
    // Adjust the score based on the side to move
    return board.getSideToMove() == Color::WHITE ? score : -score;
}

bool Engine::isEndgame(const Board& board) const {
    int pieceCount = 0;
    bool whiteQueenPresent = false;
    bool blackQueenPresent = false;
    
    // Count the number of pieces and check if queens are present
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto piece = board.getPieceAt(Position(row, col));
            
            if (piece && piece->getType() != PieceType::KING && piece->getType() != PieceType::PAWN) {
                pieceCount++;
                
                if (piece->getType() == PieceType::QUEEN) {
                    if (piece->getColor() == Color::WHITE) {
                        whiteQueenPresent = true;
                    } else {
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