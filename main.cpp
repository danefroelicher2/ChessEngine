#include "main.h"
#include "board.h"
#include "game.h"
#include "engine.h"
#include "ui.h"
#include "uci.h"

int main(int argc, char* argv[]) {
    // Create a new game
    Game game;
    
    // Create the engine with a search depth of 3
    Engine engine(game, 3);
    
    // Check for UCI mode
    bool uciMode = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--uci" || arg == "-u") {
            uciMode = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --uci, -u    Run in UCI mode (for chess GUIs)" << std::endl;
            std::cout << "  --help, -h   Show this help message" << std::endl;
            return 0;
        }
    }
    
    if (uciMode) {
        // Run in UCI mode for chess GUIs
        UCIProtocol uci(game, engine);
        uci.run();
    } else {
        // Run in interactive mode with custom UI
        std::cout << "Starting in interactive mode. Use --uci for UCI mode." << std::endl;
        UI ui(game, engine);
        ui.run();
    }
    
    return 0;
}