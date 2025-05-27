#ifndef UI_H
#define UI_H

#include "main.h"
#include "game.h"
#include "engine.h"
#include "tactical_tests.h"

class UI
{
private:
    Game &game;
    Engine &engine;
    bool playerIsWhite;
    bool gameActive;

public:
    UI(Game &g, Engine &e) : game(g), engine(e), playerIsWhite(true), gameActive(false) {}

    // Start a new game
    void newGame(bool playerPlaysWhite = true);

    // Start a game from a FEN position
    void newGameFromFEN(const std::string &fen, bool playerPlaysWhite = true);

    // Run the UI
    void run();

private:
    // Print the game state
    void printGame() const;

    // Add this line in the private section
    void runPerftTests();
    void runTacticalTests();

    // Get a move from the player
    Move getPlayerMove() const;

    // Get a move from the engine
    Move getEngineMove();

    // Process a player's command
    bool processCommand(const std::string &command);

    // Check if it's the player's turn
    bool isPlayerTurn() const;

    // Display help information
    void displayHelp() const;
};

#endif // UI_H