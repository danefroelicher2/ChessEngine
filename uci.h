#ifndef UCI_H
#define UCI_H

#include "common.h"
#include "game.h"
#include "engine.h"
#include <string>
#include <vector>
#include <map>

// UCI Option Types
enum class UCIOptionType {
    CHECK,      // boolean
    SPIN,       // integer with min/max
    COMBO,      // dropdown selection
    BUTTON,     // action button
    STRING      // text input
};

// UCI Option Structure
struct UCIOption {
    std::string name;
    UCIOptionType type;
    std::string defaultValue;
    std::string currentValue;
    std::string minValue;
    std::string maxValue;
    std::vector<std::string> comboValues;
    
    UCIOption() = default;
    UCIOption(const std::string& n, UCIOptionType t, const std::string& def) 
        : name(n), type(t), defaultValue(def), currentValue(def) {}
    UCIOption(const std::string& n, UCIOptionType t, const std::string& def,
              const std::string& min, const std::string& max)
        : name(n), type(t), defaultValue(def), currentValue(def), minValue(min), maxValue(max) {}
};

class UCIProtocol {
private:
    Game& game;
    Engine& engine;
    std::map<std::string, UCIOption> options;
    bool debugMode;
    bool searchActive;
    
    // UCI Command Handlers
    void handleUCI();
    void handleIsReady();
    void handleUCINewGame();
    void handlePosition(const std::string& command);
    void handleGo(const std::string& command);
    void handleStop();
    void handleSetOption(const std::string& command);
    void handleQuit();
    
    // Helper Methods
    void initializeOptions();
    void sendOption(const UCIOption& option);
    void parsePosition(const std::string& positionStr);
    void parseGoCommand(const std::string& goStr);
    std::vector<std::string> split(const std::string& str, char delimiter);
    void sendBestMove(const Move& move);
    void sendInfo(int depth, int score, long nodes, int time, const std::string& pv);
    
public:
    UCIProtocol(Game& g, Engine& e);
    
    // Main UCI Loop
    void run();
    
    // Process single UCI command
    void processCommand(const std::string& command);
    
    // Option Management
    void updateOption(const std::string& name, const std::string& value);
    std::string getOption(const std::string& name) const;
    bool hasOption(const std::string& name) const;
    
    // State Management
    bool isSearchActive() const { return searchActive; }
    void setSearchActive(bool active) { searchActive = active; }
};

#endif // UCI_H