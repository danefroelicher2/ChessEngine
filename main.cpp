#include "main.h"
#include "board.h"
#include "game.h"
#include "engine.h"
#include "ui.h"

int main() {
    // Create a new game
    Game game;
    
    // Create the engine with a search depth of 3
    Engine engine(game, 3);
    
    // Create and run the UI
    UI ui(game, engine);
    ui.run();
    
    return 0;
}