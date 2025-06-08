#include <iostream>
#include <string>

// Only include what we absolutely need to test
#include "game.h"
#include "engine.h"

int main(int argc, char* argv[]) {
    std::cout << "Chess Engine Starting..." << std::endl;
    std::cout.flush(); // Force output
    
    try {
        std::cout << "Step 1: Creating Game..." << std::endl;
        std::cout.flush();
        Game game;
        std::cout << "Game created successfully!" << std::endl;
        std::cout.flush();
        
        std::cout << "Step 2: Creating Engine..." << std::endl;
        std::cout.flush();
        Engine engine(game, 3);
        std::cout << "Engine created successfully!" << std::endl;
        std::cout.flush();
        
        std::cout << "Step 3: Starting main loop..." << std::endl;
        std::cout.flush();
        
        std::string input;
        while (true) {
            std::cout << "chess> ";
            std::cout.flush();
            
            if (!std::getline(std::cin, input)) {
                std::cout << "Input stream ended" << std::endl;
                break;
            }
            
            if (input == "quit" || input == "exit") {
                std::cout << "Goodbye!" << std::endl;
                break;
            }
            else if (input == "test") {
                std::cout << "Engine test successful!" << std::endl;
            }
            else if (input == "move") {
                std::cout << "Getting best move..." << std::endl;
                try {
                    Move bestMove = engine.getBestMove();
                    std::cout << "Best move: " << bestMove.toString() << std::endl;
                } catch (const std::exception& e) {
                    std::cout << "Error getting move: " << e.what() << std::endl;
                } catch (...) {
                    std::cout << "Unknown error getting move" << std::endl;
                }
            }
            else if (input == "help") {
                std::cout << "Commands: test, move, quit" << std::endl;
            }
            else if (input.empty()) {
                continue;
            }
            else {
                std::cout << "Unknown command: '" << input << "'. Try 'help'" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
        std::cout.flush();
        return 1;
    } catch (...) {
        std::cout << "Unknown exception caught!" << std::endl;
        std::cout.flush();
        return 1;
    }
    
    std::cout << "Program ending normally" << std::endl;
    return 0;
}