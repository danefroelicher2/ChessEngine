#include "uci.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

UCIProtocol::UCIProtocol(Game& g, Engine& e) 
    : game(g), engine(e), debugMode(false), searchActive(false) {
    initializeOptions();
}

void UCIProtocol::initializeOptions() {
    // Hash table size (MB)
    options["Hash"] = UCIOption("Hash", UCIOptionType::SPIN, "64", "1", "2048");
    
    // Search depth
    options["Depth"] = UCIOption("Depth", UCIOptionType::SPIN, "10", "1", "50");
    
    // Evaluation weights
    options["MobilityWeight"] = UCIOption("MobilityWeight", UCIOptionType::SPIN, "4", "0", "20");
    options["KingSafetyWeight"] = UCIOption("KingSafetyWeight", UCIOptionType::SPIN, "15", "0", "50");
    options["PawnStructureWeight"] = UCIOption("PawnStructureWeight", UCIOptionType::SPIN, "8", "0", "30");
    options["PieceCoordinationWeight"] = UCIOption("PieceCoordinationWeight", UCIOptionType::SPIN, "6", "0", "25");
    options["EndgameWeight"] = UCIOption("EndgameWeight", UCIOptionType::SPIN, "10", "0", "40");
    
    // Time management
    options["TimeManagement"] = UCIOption("TimeManagement", UCIOptionType::CHECK, "true");
    
    // Debug mode
    options["Debug"] = UCIOption("Debug", UCIOptionType::CHECK, "false");
    
    // Clear hash button
    options["Clear Hash"] = UCIOption("Clear Hash", UCIOptionType::BUTTON, "");
}

void UCIProtocol::run() {
    std::string command;
    
    while (std::getline(std::cin, command)) {
        processCommand(command);
    }
}

void UCIProtocol::processCommand(const std::string& command) {
    if (command.empty()) return;
    
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    
    if (cmd == "uci") {
        handleUCI();
    } else if (cmd == "isready") {
        handleIsReady();
    } else if (cmd == "ucinewgame") {
        handleUCINewGame();
    } else if (cmd == "position") {
        handlePosition(command);
    } else if (cmd == "go") {
        handleGo(command);
    } else if (cmd == "stop") {
        handleStop();
    } else if (cmd == "setoption") {
        handleSetOption(command);
    } else if (cmd == "quit") {
        handleQuit();
    } else if (debugMode) {
        std::cout << "info string Unknown command: " << command << std::endl;
    }
}

void UCIProtocol::handleUCI() {
    // Send engine identification
    std::cout << "id name YourChessEngine 1.0" << std::endl;
    std::cout << "id author YourName" << std::endl;
    
    // Send all options
    for (const auto& [name, option] : options) {
        sendOption(option);
    }
    
    std::cout << "uciok" << std::endl;
}

void UCIProtocol::handleIsReady() {
    std::cout << "readyok" << std::endl;
}

void UCIProtocol::handleUCINewGame() {
    // Clear hash tables and reset engine state
    engine.clearTT();
    game.newGame();
    
    if (debugMode) {
        std::cout << "info string New game started" << std::endl;
    }
}

void UCIProtocol::handlePosition(const std::string& command) {
    std::vector<std::string> tokens = split(command, ' ');
    
    if (tokens.size() < 2) return;
    
    if (tokens[1] == "startpos") {
        game.newGame();
        
        // Look for moves
        auto movesIt = std::find(tokens.begin(), tokens.end(), "moves");
        if (movesIt != tokens.end()) {
            // Apply moves
            for (auto it = movesIt + 1; it != tokens.end(); ++it) {
                if (!game.makeMove(*it)) {
                    if (debugMode) {
                        std::cout << "info string Invalid move: " << *it << std::endl;
                    }
                    return;
                }
            }
        }
    } else if (tokens[1] == "fen" && tokens.size() >= 8) {
        // Reconstruct FEN string
        std::string fen = tokens[2] + " " + tokens[3] + " " + tokens[4] + " " + 
                         tokens[5] + " " + tokens[6] + " " + tokens[7];
        
        game.newGameFromFEN(fen);
        
        // Look for moves after FEN
        auto movesIt = std::find(tokens.begin(), tokens.end(), "moves");
        if (movesIt != tokens.end()) {
            for (auto it = movesIt + 1; it != tokens.end(); ++it) {
                if (!game.makeMove(*it)) {
                    if (debugMode) {
                        std::cout << "info string Invalid move: " << *it << std::endl;
                    }
                    return;
                }
            }
        }
    }
}

void UCIProtocol::handleGo(const std::string& command) {
    std::vector<std::string> tokens = split(command, ' ');
    
    searchActive = true;
    
    // Parse go command parameters
    int depth = std::stoi(getOption("Depth"));
    int moveTime = 0;
    int wtime = 0, btime = 0, winc = 0, binc = 0;
    bool infinite = false;
    
    for (size_t i = 1; i < tokens.size(); i++) {
        if (tokens[i] == "depth" && i + 1 < tokens.size()) {
            depth = std::stoi(tokens[i + 1]);
            i++;
        } else if (tokens[i] == "movetime" && i + 1 < tokens.size()) {
            moveTime = std::stoi(tokens[i + 1]);
            i++;
        } else if (tokens[i] == "wtime" && i + 1 < tokens.size()) {
            wtime = std::stoi(tokens[i + 1]);
            i++;
        } else if (tokens[i] == "btime" && i + 1 < tokens.size()) {
            btime = std::stoi(tokens[i + 1]);
            i++;
        } else if (tokens[i] == "winc" && i + 1 < tokens.size()) {
            winc = std::stoi(tokens[i + 1]);
            i++;
        } else if (tokens[i] == "binc" && i + 1 < tokens.size()) {
            binc = std::stoi(tokens[i + 1]);
            i++;
        } else if (tokens[i] == "infinite") {
            infinite = true;
        }
    }
    
    // Set engine parameters
    engine.setDepth(depth);
    
    // Calculate time allocation if needed
    if (moveTime > 0) {
        engine.setTimeForMove(moveTime);
    } else if (wtime > 0 || btime > 0) {
        // Simple time management
        Color sideToMove = game.getBoard().getSideToMove();
        int timeLeft = (sideToMove == Color::WHITE) ? wtime : btime;
        int increment = (sideToMove == Color::WHITE) ? winc : binc;
        
        // Allocate roughly 1/30th of remaining time plus increment
        int allocatedTime = (timeLeft / 30) + increment;
        allocatedTime = std::max(100, std::min(allocatedTime, timeLeft / 2));
        
        engine.setTimeForMove(allocatedTime);
    }
    
    // Start search in a separate thread (for real implementation)
    // For now, we'll do it synchronously
    auto startTime = std::chrono::high_resolution_clock::now();
    
    Move bestMove = engine.getBestMove();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Send info about the search
    sendInfo(depth, 0, engine.getNodesSearched(), duration.count(), engine.getPVString());
    
    // Send best move
    sendBestMove(bestMove);
    
    searchActive = false;
}

void UCIProtocol::handleStop() {
    if (searchActive) {
        // In a real implementation, this would signal the search thread to stop
        searchActive = false;
        
        if (debugMode) {
            std::cout << "info string Search stopped" << std::endl;
        }
    }
}

void UCIProtocol::handleSetOption(const std::string& command) {
    std::vector<std::string> tokens = split(command, ' ');
    
    // Format: setoption name <name> value <value>
    if (tokens.size() >= 5 && tokens[1] == "name" && tokens[3] == "value") {
        std::string optionName = tokens[2];
        std::string optionValue = tokens[4];
        
        // Handle multi-word option names
        size_t valuePos = command.find(" value ");
        if (valuePos != std::string::npos) {
            size_t nameStart = command.find(" name ") + 6;
            optionName = command.substr(nameStart, valuePos - nameStart);
            optionValue = command.substr(valuePos + 7);
        }
        
        updateOption(optionName, optionValue);
    }
}

void UCIProtocol::handleQuit() {
    if (debugMode) {
        std::cout << "info string Goodbye!" << std::endl;
    }
    exit(0);
}

void UCIProtocol::sendOption(const UCIOption& option) {
    std::cout << "option name " << option.name << " type ";
    
    switch (option.type) {
        case UCIOptionType::CHECK:
            std::cout << "check default " << option.defaultValue;
            break;
        case UCIOptionType::SPIN:
            std::cout << "spin default " << option.defaultValue 
                      << " min " << option.minValue 
                      << " max " << option.maxValue;
            break;
        case UCIOptionType::COMBO:
            std::cout << "combo default " << option.defaultValue;
            for (const auto& val : option.comboValues) {
                std::cout << " var " << val;
            }
            break;
        case UCIOptionType::BUTTON:
            std::cout << "button";
            break;
        case UCIOptionType::STRING:
            std::cout << "string default " << option.defaultValue;
            break;
    }
    
    std::cout << std::endl;
}

void UCIProtocol::updateOption(const std::string& name, const std::string& value) {
    if (options.find(name) != options.end()) {
        options[name].currentValue = value;
        
        // Apply the option to the engine
        if (name == "Hash") {
            int hashSize = std::stoi(value);
            engine.setTTSize(hashSize);
        } else if (name == "Depth") {
            int depth = std::stoi(value);
            engine.setDepth(depth);
        } else if (name == "TimeManagement") {
            bool timeManaged = (value == "true");
            engine.setTimeManagement(timeManaged);
        } else if (name == "Debug") {
            debugMode = (value == "true");
        } else if (name == "Clear Hash") {
            engine.clearTT();
        }
        // Note: Evaluation weights would need engine modifications to be applied
        
        if (debugMode) {
            std::cout << "info string Set " << name << " = " << value << std::endl;
        }
    } else if (debugMode) {
        std::cout << "info string Unknown option: " << name << std::endl;
    }
}

std::string UCIProtocol::getOption(const std::string& name) const {
    auto it = options.find(name);
    return (it != options.end()) ? it->second.currentValue : "";
}

bool UCIProtocol::hasOption(const std::string& name) const {
    return options.find(name) != options.end();
}

std::vector<std::string> UCIProtocol::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    
    while (std::getline(iss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

void UCIProtocol::sendBestMove(const Move& move) {
    std::cout << "bestmove " << move.toString() << std::endl;
}

void UCIProtocol::sendInfo(int depth, int score, long nodes, int time, const std::string& pv) {
    std::cout << "info depth " << depth 
              << " score cp " << score 
              << " nodes " << nodes 
              << " time " << time;
    
    if (time > 0) {
        std::cout << " nps " << (nodes * 1000 / time);
    }
    
    if (!pv.empty()) {
        std::cout << " pv " << pv;
    }
    
    std::cout << std::endl;
}