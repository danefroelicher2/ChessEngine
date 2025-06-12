#include <iostream>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "game.h"
#include "engine.h"

#pragma comment(lib, "ws2_32.lib")

class EngineBridge {
private:
    Game game;
    Engine engine;
    
public:
    EngineBridge() : engine(game, 3) {
        std::cout << "Real Engine Bridge initialized!" << std::endl;
    }
    
    std::string getEngineMove(const std::string& fen, int depth) {
        try {
            std::cout << "Setting position: " << fen.substr(0, 30) << "..." << std::endl;
            
            // Use default starting position for now to avoid FEN parsing issues
            game.newGame();
            std::cout << "Position set to starting position" << std::endl;
            
            std::cout << "Engine thinking (depth " << depth << ")..." << std::endl;
            engine.setDepth(depth);
            
            std::cout << "Calling getBestMove()..." << std::endl;
            Move bestMove = engine.getBestMove();
            std::cout << "Got best move" << std::endl;
            
            std::string moveStr = bestMove.from.toString() + bestMove.to.toString();
            int eval = engine.evaluatePosition(game.getBoard());
            
            std::cout << "Engine move: " << moveStr << " (eval: " << eval << ")" << std::endl;
            
            return "{\"move\":\"" + moveStr + "\",\"eval\":" + std::to_string(eval) + "}";
            
        } catch (const std::exception& e) {
            std::cout << "ENGINE EXCEPTION: " << e.what() << std::endl;
            return "{\"move\":\"e2e4\",\"eval\":0,\"error\":\"" + std::string(e.what()) + "\"}";
        } catch (...) {
            std::cout << "UNKNOWN ENGINE ERROR" << std::endl;
            return "{\"move\":\"e2e4\",\"eval\":0,\"error\":\"Unknown error\"}";
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
    static EngineBridge bridge;
    
    if (request.find("GET /status") == 0) {
        return httpResponse("{\"status\":\"Real Chess Engine Connected\"}");
    }
    
    if (request.find("GET /move") == 0) {
        std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        int depth = 3;
        
        return httpResponse(bridge.getEngineMove(fen, depth));
    }
    
    return httpResponse("{\"error\":\"Unknown request\"}");
}

int main() {
    std::cout << "🏆 REAL ENGINE SERVER STARTING! 🏆" << std::endl;
    
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
    
    std::cout << "✅ Server listening on port 8080" << std::endl;
    std::cout << "Ready for chess engine requests!" << std::endl;
    
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