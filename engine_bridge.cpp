#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "game.h"
#include "engine.h"

#pragma comment(lib, "ws2_32.lib")

class SafeEngineBridge {
private:
    Game game;
    Engine engine;
    
public:
    SafeEngineBridge() : engine(game, 1) {  // Start with depth 1 for safety
        std::cout << "Safe Engine Bridge initialized with depth 1!" << std::endl;
    }
    
    std::string getEngineMove(const std::string& fen, int depth) {
        try {
            std::cout << "=== ENGINE MOVE REQUEST ===" << std::endl;
            std::cout << "Requested depth: " << depth << std::endl;
            
            // Use minimal depth
            if (depth > 1) depth = 1;
            std::cout << "Using safe depth: " << depth << std::endl;
            
            // Use starting position for now
            std::cout << "Setting up starting position..." << std::endl;
            game.newGame();
            std::cout << "✓ Game initialized" << std::endl;
            
            // SKIP EVALUATION - it's causing the crash
            std::cout << "Skipping evaluation test..." << std::endl;
            
            std::cout << "Setting engine depth..." << std::endl;
            engine.setDepth(depth);
            std::cout << "✓ Depth set to " << depth << std::endl;
            
            std::cout << "Calling getBestMove()..." << std::endl;
            Move bestMove = engine.getBestMove();
            std::cout << "✓ Got best move!" << std::endl;
            
            // Validate the move
            if (!bestMove.from.isValid() || !bestMove.to.isValid()) {
                std::cout << "WARNING: Invalid move returned!" << std::endl;
                return "{\"move\":\"e2e4\",\"eval\":0,\"error\":\"Invalid move\"}";
            }
            
            std::string moveStr = bestMove.from.toString() + bestMove.to.toString();
            std::cout << "✓ Move string: " << moveStr << std::endl;
            
            std::cout << "=== SUCCESS ===" << std::endl;
            
            return "{\"move\":\"" + moveStr + "\",\"eval\":0}";
            
        } catch (const std::exception& e) {
            std::cout << "❌ ENGINE EXCEPTION: " << e.what() << std::endl;
            return "{\"move\":\"e2e4\",\"eval\":0,\"error\":\"Exception: " + std::string(e.what()) + "\"}";
        } catch (...) {
            std::cout << "❌ UNKNOWN ENGINE ERROR" << std::endl;
            return "{\"move\":\"e2e4\",\"eval\":0,\"error\":\"Unknown crash\"}";
        }
    }
};

std::string httpResponse(const std::string& content) {
    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Access-Control-Allow-Methods: GET\r\n";
    response << "Content-Length: " << content.length() << "\r\n";
    response << "\r\n";
    response << content;
    return response.str();
}

std::string handleRequest(const std::string& request) {
    static SafeEngineBridge bridge;
    
    if (request.find("GET /status") == 0) {
        return httpResponse("{\"status\":\"Safe Engine Bridge Ready\"}");
    }
    
    if (request.find("GET /move") == 0) {
        std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        int depth = 1;  // Safe depth
        
        return httpResponse(bridge.getEngineMove(fen, depth));
    }
    
    return httpResponse("{\"error\":\"Unknown request\"}");
}

int main() {
    std::cout << "🛡️ SAFE ENGINE BRIDGE STARTING! 🛡️" << std::endl;
    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "WSAStartup failed" << std::endl;
        return 1;
    }
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Socket creation failed" << std::endl;
        WSACleanup();
        return 1;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Bind failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cout << "Listen failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    std::cout << "✅ Safe server listening on port 8080" << std::endl;
    std::cout << "Ready for safe chess engine testing!" << std::endl;
    
    while (true) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) continue;
        
        char buffer[4096] = {0};
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);
        
        if (bytesReceived > 0) {
            std::string request(buffer);
            std::string response = handleRequest(request);
            send(clientSocket, response.c_str(), (int)response.length(), 0);
        }
        
        closesocket(clientSocket);
    }
    
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}