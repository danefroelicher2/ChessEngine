#ifndef GAME_H
#define GAME_H

#include "main.h"
#include "board.h"

enum class GameResult {
    IN_PROGRESS,
    WHITE_WINS,
    BLACK_WINS,
    DRAW
};

enum class GameEndReason {
    NONE,
    CHECKMATE,
    STALEMATE,
    INSUFFICIENT_MATERIAL,
    FIFTY_MOVE_RULE,
    THREEFOLD_REPETITION,
    AGREEMENT
};

class Game {
private:
    Board board;
    std::vector<Move> moveHistory;
    std::vector<std::string> fenHistory;
    GameResult result;
    GameEndReason endReason;
    
public:
    Game();
    
    // Start a new game with the standard starting position
    void newGame();
    
    // Start a game from a FEN position
    void newGameFromFEN(const std::string& fen);
    
    // Make a move in the game
    bool makeMove(const Move& move);
    
    // Make a move in the game using algebraic notation
    bool makeMove(const std::string& moveStr);
    
    // Undo the last move
    bool undoMove();
    
    // Get the current board state
    const Board& getBoard() const { return board; }
    
    // Get the game result
    GameResult getResult() const { return result; }
    
    // Get the reason why the game ended
    GameEndReason getEndReason() const { return endReason; }
    
    // Get the move history
    const std::vector<Move>& getMoveHistory() const { return moveHistory; }
    
    // Get the FEN history
    const std::vector<std::string>& getFENHistory() const { return fenHistory; }
    
    // Check for draw by insufficient material
    bool isInsufficientMaterial() const;
    
    // Check for draw by fifty move rule
    bool isFiftyMoveRule() const;
    
    // Check for draw by threefold repetition
    bool isThreefoldRepetition() const;
    
    // End the game in a draw by agreement
    void endInDrawByAgreement();
    
    // Check if the game is over
    bool isGameOver() const { return result != GameResult::IN_PROGRESS; }
    
    // Print the current game state
    void print() const;
};

#endif // GAME_H