#include <iostream>
#include "game.h"
#include "engine.h"

int main() {
    try {
        std::cout << "Creating Game..." << std::endl;
        Game game;
        std::cout << "Creating Engine..." << std::endl;
        Engine engine(game, 3);
        std::cout << "Success!" << std::endl;
        std::cin.get();
    } catch (...) {
        std::cout << "Engine constructor failed!" << std::endl;
        std::cin.get();
    }
    return 0;
}